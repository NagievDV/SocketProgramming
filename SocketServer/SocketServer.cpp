#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#pragma comment (lib, "Ws2_32.lib")

using namespace std;

int main(void) {

    SetConsoleTitle("Socket Server");

    const char IP_SERV[] = "127.0.0.1";
    const int PORT_NUM = 1234;
    const short BUFF_SIZE = 1024;
    int erStat;

    in_addr ip_to_num;
    erStat = inet_pton(AF_INET, IP_SERV, &ip_to_num);

    if (erStat <= 0) {
        cout << "Error in IP translation to special numeric format" << endl;
        return 1;
    }

    WSADATA wsData;
    erStat = WSAStartup(MAKEWORD(2, 2), &wsData);
    if (erStat != 0) {
        cout << "Error WinSock version initialization #" << WSAGetLastError() << endl;
        return 1;
    }
    else cout << "WinSock is initialized" << endl;

    SOCKET ServSock = socket(AF_INET, SOCK_STREAM, 0);
    if (ServSock == INVALID_SOCKET) {
        cout << "Error initialization socket #" << WSAGetLastError() << endl;
        closesocket(ServSock);
        WSACleanup();
        return 1;
    }
    else cout << "Server socket is initialized" << endl;

    sockaddr_in servInfo;
    ZeroMemory(&servInfo, sizeof(servInfo));
    servInfo.sin_family = AF_INET;
    servInfo.sin_addr = ip_to_num;
    servInfo.sin_port = htons(PORT_NUM);
    erStat = bind(ServSock, (sockaddr*)&servInfo, sizeof(servInfo));
    if (erStat != 0) {
        cout << "Error Socket binding to server info. Error #" << WSAGetLastError() << endl;
        closesocket(ServSock);
        WSACleanup();
        return 1;
    }
    else cout << "Socket binding to server information completed" << endl;

    erStat = listen(ServSock, SOMAXCONN);
    if (erStat != 0) {
        cout << "Can't start to listen. Error #" << WSAGetLastError() << endl;
        closesocket(ServSock);
        WSACleanup();
        return 1;
    }
    else cout << "Listening" << endl;

    sockaddr_in clientInfo;
    ZeroMemory(&clientInfo, sizeof(clientInfo));
    int clientInfo_size = sizeof(clientInfo);
    SOCKET ClientConn = accept(ServSock, (sockaddr*)&clientInfo, &clientInfo_size);
    if (ClientConn == INVALID_SOCKET) {
        cout << "Client detected, but can't connect to a client. Error #" << WSAGetLastError() << endl;
        closesocket(ServSock);
        closesocket(ClientConn);
        WSACleanup();
        return 1;
    }
    else {
        cout << "Connection to a client established" << endl;
        char clientIP[22];
        inet_ntop(AF_INET, &clientInfo.sin_addr, clientIP, INET_ADDRSTRLEN);
        cout << "Client connected with IP address " << clientIP << endl;
    }

    vector<char> recvBuff(BUFF_SIZE);
    char replacement;

    int recvSize = recv(ClientConn, recvBuff.data(), recvBuff.size(), 0);
    if (recvSize == SOCKET_ERROR) {
        cout << "Error receiving file content. Error #" << WSAGetLastError() << endl;
        closesocket(ServSock);
        closesocket(ClientConn);
        WSACleanup();
        return 1;
    }

    cout << "Received file content from client." << endl;

    recvSize = recv(ClientConn, &replacement, sizeof(replacement), 0);
    if (recvSize == SOCKET_ERROR) {
        cout << "Error receiving replacement character. Error #" << WSAGetLastError() << endl;
        closesocket(ServSock);
        closesocket(ClientConn);
        WSACleanup();
        return 1;
    }
    cout << "Received replacement character: " << replacement << endl;

    for (int i = 0; i < recvBuff.size(); ++i) {
        if (ispunct(static_cast<unsigned char>(recvBuff[i]))) {
            recvBuff[i] = replacement;
        }
    }

    int totalSent = 0;
    while (totalSent < recvBuff.size()) {
        int sendSize = send(ClientConn, recvBuff.data() + totalSent, recvBuff.size() - totalSent, 0);

        if (sendSize == SOCKET_ERROR) {
            cout << "Error sending modified text to client. Error #" << WSAGetLastError() << endl;
            closesocket(ClientConn);
            closesocket(ServSock);
            WSACleanup();
            return 1;
        }
        totalSent += sendSize;
    }

    cout << "Modified text sent back to client." << endl;

    closesocket(ClientConn);
    closesocket(ServSock);
    WSACleanup();
    return 0;
}
