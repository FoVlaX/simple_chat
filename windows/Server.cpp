#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <map>
#include <fstream>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
using namespace std;


enum ClientStates { login, inChat };
class Server;

class Client {
public:

	Client(Server* server, SOCKET clientSocket);

	~Client();

	void run();

	void setName(string name);
	void setClientSocket(SOCKET clientSocket);
	string getName();
	SOCKET getClientSocket();
private:
	string name;
	SOCKET clientSocket;
	Server* server;
	ClientStates clientState;

};


void clientThread(Server* server, SOCKET clientSocket) {
	Client client(server, clientSocket);
	client.run();
}



class Server {
public:
	Server(unsigned short port);
	~Server();
		
	void startServer();

	void sendMessageAll(string msg, string name);

	bool sendPrivateMsg(string msg, string toName, string myName);

	void addClient(string name, Client* client);

	void removeClient(string name);

	void loadData();

	void saveData();

	bool addUser(string login, string password);

	int checkData(string login, string password);

	void sendOnline(string name);

	void sendAllUsers(string name);

private:

	void handle();
	map<string, Client*> clients; //клиенты онлайн

	map<string, string> data; //словарь логин - пароль

	WSAData wData;
	SOCKET listenSocket; //сокет сервера
	SOCKADDR_IN addr; //адрес сервера
	unsigned short port; //порт

};






	Server::Server(unsigned short port) {
		this->port = port;
		if (WSAStartup(MAKEWORD(2, 2), &wData) == 0) //создаем словарь
		{
			printf("WSA Startup succes\n");
		}
		
		int addrl = sizeof(addr);
		addr.sin_addr.S_un.S_addr = INADDR_ANY;
		addr.sin_port = htons(port);
		addr.sin_family = AF_INET;
		listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //создаем сокет сервера 
		if (listenSocket == SOCKET_ERROR) {
			printf("Socket not created\n");
		}
		
		loadData(); //«агружаем данные о логинах и парол¤х
	}
	Server::~Server() {
		WSACleanup();
		closesocket(listenSocket);
	}

	void Server::startServer() {

		if (bind(listenSocket, (struct sockaddr*) & addr, sizeof(addr)) != SOCKET_ERROR) { //св¤зываем сокет с адресом
			printf("Socket success binded\n");
		}
		else {
			printf("ERROR! Socket not binded\n");
			return;
		}

		if (listen(listenSocket, SOMAXCONN) != SOCKET_ERROR) { //начинаем "слушать" порт
			printf("Start listenin at port %u\n", ntohs(addr.sin_port));
		}
		else {
			printf("ERROR! Can't listenin port\n");
			return;
		}
		
		handle(); //ждем поключени¤ клиентов

	}

	void Server::sendMessageAll(string msg, string name) { //отправить всем сообщение
		map<string, Client*>::iterator it = clients.begin();
		for ( ;it != clients.end(); it++) {
			if ((*it).first != name) {
				string message = name + ": " + msg;
				int size = message.length()+1;
				char* charMsg = new char[size];
				strcpy(charMsg, message.c_str());
				charMsg[size] = '\0';
				send((*it).second->getClientSocket(), charMsg, size, 0);
			}
		}
	}
	
	bool Server::sendPrivateMsg(string msg, string toName, string myName) { //отправиtь приватное сообщение
		
		if (toName != myName) {
			if (clients.find(toName) != clients.end()) {
				Client* client = clients[toName];
				string message = "(private)" + myName + ": " + msg;
				int size = message.length() + 1;
				char* charMsg = new char[size];
				strcpy(charMsg, message.c_str());
				send(client->getClientSocket(), charMsg, size, 0);
				return true;
			}
		}
		return false;
		
	}

	void Server::addClient(string name, Client *client) {//добавить клиента который сейчас онлайн
		clients[name] = client;
	}

	void Server::removeClient(string name) { //удалить клиента при его отключении
		if (clients.find(name) != clients.end()) {
			clients.erase(name);
			sendMessageAll(name + " disconnected", "Server");
		}
	}

	void Server::loadData() { //загрузить данные о пар¤л¤х и логинах из файла
		ifstream in("data.dat");
		int count;
		in >> count;
		for (int i = 0; i < count; i++) {
			string login;
			string password;
			in >> login >> password;
			data[login] = password;
		}
		in.close();
	}

	void Server::saveData() { //сохранить данные
		ofstream out("data.dat");
		int count = data.size();
		out << count << endl;
		map<string, string>::iterator it = data.begin();
		for (; it != data.end(); it++) {
			out << (*it).first << " " << (*it).second << endl;
		}
		out.close();
	}

	bool Server::addUser(string login, string password) { //добавить нового юзера
		if (data.find(login) == data.end()) {
			data[login] = password;
			saveData();
			return true;
		}
		else {
			return false;
		}
	}

	int Server::checkData(string login, string password) { //проверить совпадени¤ логина и парол¤ по базе

		if (data.find(login) == data.end()) {
			return -1;
		}
		else {
			if (data[login] == password) {
				if (clients.find(login) == clients.end()) { //провер¤ем  есть ли юзер уже на сервере
					return 0;
				}
				else {
					return -2;
				}
			}
			else {
				return -1;
			}
		}
	}

	void Server::sendOnline(string name)
	{
		string msg = "Now online:\n";
		map<string, Client*>::iterator it = clients.begin();
		while (it != clients.end()) {
			
			msg += (*it).first + "\n";
			it++;
		}
		sendPrivateMsg(msg, name, "Server");
	}

	void Server::sendAllUsers(string name)
	{
		string msg = "All users on Server:\n";
		map<string, string>::iterator it = data.begin();
		while (it != data.end()) {
			msg += (*it).first + "\n";
			it++;
		}
		sendPrivateMsg(msg, name, "Server");
	}


	void Server::handle() {
		while (true)
		{
			SOCKET clientSocket; //сокет кллиента
			SOCKADDR_IN addr_c; //адрес клиента
			int addrlen = sizeof(addr_c);
			if ((clientSocket = accept(listenSocket, (struct sockaddr*) &addr_c, &addrlen)) != 0) { //ждем когда клиент подключитьс¤, сокеты в блокирующем режиме
				thread thread(clientThread,this,clientSocket);
				thread.detach();
			}
			Sleep(50);
		}
	}





	Client::Client(Server* server, SOCKET clientSocket) {
		this->server = server;
		this->clientSocket = clientSocket;
		this->name = "";
		clientState = login;
	}

	Client::~Client() {
		closesocket(clientSocket);
	}

	void Client::run() {
		char buffer[1024];
		int k = 0;
		send(clientSocket, (char*)"Write [/register login password] to register or [/login login password] to login, login - it's your name\n", 1024, 0);
		while ((k = recv(clientSocket, (char*)buffer, 1024, 0)) > 0) {
			switch (clientState) {
			case login: {
				string msg(buffer);

				size_t pos = msg.find("/register ");
				if (pos != string::npos && pos == 0) {

					size_t first = msg.find(" ");
					size_t second = msg.find(" ", first + 1);
					string login;
					string password;
					if (second != string::npos && second != msg.length() - 1 && second - first > 1) { //провер¤ем соответствие формату

						login = msg.substr(first + 1, second - first - 1);
						password = msg.substr(second + 1, msg.length() - second - 1);


						if (server->addUser(login, password)) {
							setName(login);
							server->addClient(login, this);
							clientState = inChat;
							server->sendMessageAll("User " + login + " join to server \n", "Server");
							send(clientSocket, (char*)"Accept!\n Use /help for more information\n", 1024, 0);
							continue;
						}
						else {
							send(clientSocket, (char*)"User already registered\n", 1024, 0);
							continue;
						}
					}

				}

				pos = msg.find("/login ");
				if (pos != string::npos && pos == 0) {
					size_t first = msg.find(" ");
					size_t second = msg.find(" ", first + 1);
					string login;
					string password;
					if (second != string::npos && second != msg.length() - 1 && second - first > 1) {//провер¤ем соответствие формату

						login = msg.substr(first + 1, second - first - 1);
						password = msg.substr(second + 1, msg.length() - second-1);

						int check;
						if ((check = server->checkData(login, password))==0) {
							setName(login);
							server->addClient(login, this);
							clientState = inChat;
							server->sendMessageAll("User " + login + " join to server\n", "Server");
							send(clientSocket, (char*)"Accept! Use /help for more information \n", 1024, 0);
							continue;
						}
						else {
							if (check == -1) {
								send(clientSocket, (char*)"Wrong login or password\n", 1024, 0);
							}
							else {
								send(clientSocket, (char*)"User already on server\n", 1024, 0);
							}
							continue;
						}
					}
				}
				send(clientSocket, (char*)"Unknow command!\n", 1024, 0);
			}break;

			case inChat: {
				string msg(buffer);
				size_t pos = msg.find("/msg ");
				if (pos != string::npos && pos == 0) {
					size_t first = msg.find(" ");
					size_t second = msg.find(" ", first + 1);
					string toName;
					string message;

					if (second != string::npos && second != msg.length() - 1 && second - first > 1) {//провер¤ем соответствие формату

						toName = msg.substr(first + 1, second - first - 1);
						message = msg.substr(second + 1, msg.length() - second - 1);
						server->sendPrivateMsg(message, toName, name);
						continue;
					}
				}

				if (msg == "/help") {
					send(clientSocket, (char*)"Commands:\n /help - help\n /msg username message - private message\n/online - list online users\n/users - list all registered users\n", 1024, 0);
					continue;
				}

				if (msg == "/online") {
					server->sendOnline(name);
					continue;
				}

				if (msg == "/users") {
					server->sendAllUsers(name);
					continue;
				}
				server->sendMessageAll(msg, name);
			}break;
			}
		}
		server->removeClient(name);
	}

	void Client::setName(string name) {
		this->name = name;
	}

	void Client::setClientSocket(SOCKET clientSocket) {
		this->clientSocket = clientSocket;
	}

	string Client::getName() {
		return name;
	}

	SOCKET Client::getClientSocket() {
		return clientSocket;
	}


int main() {

	Server server(27015);
	server.startServer();
	return 0;
}