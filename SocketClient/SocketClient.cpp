#include <iostream> 
#include <WinSock2.h>
#include <WS2tcpip.h> 
#include <inaddr.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#pragma comment(lib, "Ws2_32.lib")
using namespace std;

int main(void) {

	SetConsoleTitle("Socket Client");
	
	const char SERVER_IP[] = "127.0.0.1";
	const short SERVER_PORT_NUM = 1234;
	const short BUFF_SIZE = 1024;

	int erStat;

	in_addr ip_to_num;
	inet_pton(AF_INET, SERVER_IP, &ip_to_num);
	
	WSADATA wsData;
	erStat = WSAStartup(MAKEWORD(2, 2), &wsData);

	if (erStat != 0) {
		cout << "Error WinSock version initializaion #" << WSAGetLastError();
		return 1;
	} else cout<< "WinSock is initialized" << endl;
	
	SOCKET ClientSock = socket(AF_INET, SOCK_STREAM, 0);


	if (ClientSock == INVALID_SOCKET) {
		cout << "Error initialization socket #" << WSAGetLastError() << endl; 
		closesocket (ClientSock);
		WSACleanup();
		return 0;
		
	} else cout << "Client socket is initialized" << endl;

	sockaddr_in servInfo;
	
	ZeroMemory(&servInfo, sizeof(servInfo));
	
	servInfo.sin_family = AF_INET;
	servInfo.sin_addr = ip_to_num;
	servInfo.sin_port = htons(SERVER_PORT_NUM);
	
	erStat = connect(ClientSock, (sockaddr*)&servInfo, sizeof(servInfo));
	
	if (erStat != 0) {
		cout << "Connection to Server is failed. Error # " << WSAGetLastError() << endl; closesocket(ClientSock);
		WSACleanup();
		return 1;
	} else cout << "Connection is established. Ready to send a message to Server" << endl;
	
	vector<char> servBuff(BUFF_SIZE), clientBuff(BUFF_SIZE);
	short packet_size = 0;

	while (true) {
		cout << "Client message to Server: ";
		fgets(clientBuff.data(), clientBuff.size(), stdin);
		if (clientBuff[0] == 'e' && clientBuff[1] == 'x' && clientBuff[2] == 'i' && clientBuff[3] == 't') {
			shutdown(ClientSock, SD_BOTH);
			closesocket(ClientSock);
			WSACleanup();
			return 0;
		}

		packet_size = send(ClientSock, clientBuff.data(), clientBuff.size(), 0);

		if (packet_size == SOCKET_ERROR) {
			cout << "Can't send message to Server. Error # " << WSAGetLastError() << endl;
			closesocket(ClientSock);
			WSACleanup();
			return 1;
		}

		packet_size = recv(ClientSock, servBuff.data(), servBuff.size(), 0);

		if (packet_size == SOCKET_ERROR) {
			cout << "Can't receive message from Server. Error # " << WSAGetLastError() << endl;
			closesocket(ClientSock);
			WSACleanup();
			return 1;
		} else cout << endl << "Server message: " << servBuff.data() << endl;
	}
				
		closesocket(ClientSock);
		WSACleanup();
		return 0;
}