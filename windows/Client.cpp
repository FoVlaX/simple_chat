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

void sender(SOCKET clientSocket) {
	cin.ignore();
	while (1) {
		char str[1024];
		
		cin.getline(str, 1024);
		send(clientSocket, (char*)str, 1024, 0);
	}
}

class Client {
public:
	Client() {
		if (WSAStartup(MAKEWORD(2, 2), &wData) == 0){ //создаем словарь
		
			printf("WSA Startup succes\n");
		}
	}
	~Client() {
		
		closesocket(clientSocket);
	}
	bool run() {

	
		if ((clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
			
				printf("\n Error : Could not create socket \n");
			
				return false;
			
		}


		cout << "Write ip address:" << endl;
		char ip[50];
		cin >> ip;
		cout << "Write port" << endl;
		unsigned int port;
		cin >> port;

		
		memset(&c_addr, '0', sizeof(c_addr));

		c_addr.sin_family = AF_INET;
		c_addr.sin_port = htons(port);

		if (inet_pton(AF_INET, ip, &c_addr.sin_addr) <= 0) {

			printf("\n inet_pton error occured\n");
			return false;
		}

		if (connect(clientSocket, (struct sockaddr*) & c_addr, sizeof(c_addr)) < 0){
			
			printf("\n Error : Connect Failed \n");
			
			return false;
		}
		int n;
		thread thread(sender, clientSocket);
		thread.detach();
		while ((n = recv(clientSocket, buffer, sizeof(buffer) - 1,0)) > 0) {
			buffer[n] = '\0';
			cout << buffer << endl;
			char bfr[1024];
		}
		cout << "\nServer lost connection!\n";
		return false;
	}
private:
	WSAData wData;
	SOCKET clientSocket;
	sockaddr_in c_addr;
	char buffer[1024];
};

int main() {
	Client client;
	
	client.run();
	
	return 0;
}