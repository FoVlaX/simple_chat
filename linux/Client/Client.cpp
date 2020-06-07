#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <string.h>
#include <thread>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

void sender(int clientSocket) {
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
	
	}
	~Client() {
		
		close(clientSocket);
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
			
		}
		cout << "\nServer lost connection!\n";
		return false;
	}
private:
	int clientSocket;
	sockaddr_in c_addr;
	char buffer[1024];
};

int main() {
	Client client;
	
	client.run();
	
	return 0;
}
