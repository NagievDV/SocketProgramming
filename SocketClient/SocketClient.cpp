#include <iostream> 
#include <WinSock2.h>
#include <WS2tcpip.h> 
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
        cout << "Error WinSock version initialization #" << WSAGetLastError();
        return 1;
    }
    else cout << "WinSock is initialized" << endl;

    SOCKET ClientSock = socket(AF_INET, SOCK_STREAM, 0);

    if (ClientSock == INVALID_SOCKET) {
        cout << "Error initialization socket #" << WSAGetLastError() << endl;
        closesocket(ClientSock);
        WSACleanup();
        return 0;

    }
    else cout << "Client socket is initialized" << endl;

    sockaddr_in servInfo;

    ZeroMemory(&servInfo, sizeof(servInfo));

    servInfo.sin_family = AF_INET;
    servInfo.sin_addr = ip_to_num;
    servInfo.sin_port = htons(SERVER_PORT_NUM);

    erStat = connect(ClientSock, (sockaddr*)&servInfo, sizeof(servInfo));

    if (erStat != 0) {
        cout << "Connection to Server failed. Error # " << WSAGetLastError() << endl;
        closesocket(ClientSock);
        WSACleanup();
        return 1;
    }
    else cout << "Connection established. Ready to send a message to Server" << endl;

    char fileBuffer[BUFF_SIZE] = { 0 };

    while (true) {
        cout << "Path to file: ";
        char filePath[BUFF_SIZE] = { 0 };
        cin.getline(filePath, BUFF_SIZE);

        if (strcmp(filePath, "exit") == 0) {
            shutdown(ClientSock, SD_BOTH);
            closesocket(ClientSock);
            WSACleanup();
            return 0;
        }

        ifstream file(filePath, ios::binary);
        if (!file) {
            cout << "Cannot open file: " << filePath << endl;
            continue;
        }

        file.read(fileBuffer, BUFF_SIZE);
        int fileSize = file.gcount();
        file.close();

        cout << "Symbol: ";
        char replacement;
        cin >> replacement;
        cin.ignore();

        int packet_size = send(ClientSock, fileBuffer, fileSize, 0);
        if (packet_size == SOCKET_ERROR) {
            cout << "Can't send message to Server. Error # " << WSAGetLastError() << endl;
            closesocket(ClientSock);
            WSACleanup();
            return 1;
        }

        packet_size = send(ClientSock, &replacement, sizeof(replacement), 0);
        if (packet_size == SOCKET_ERROR) {
            cout << "Can't send replacement character to Server. Error # " << WSAGetLastError() << endl;
            closesocket(ClientSock);
            WSACleanup();
            return 1;
        }

        vector<char> servBuff(BUFF_SIZE);
        int totalReceived = 0;

        cout << "Receiving data from server..." << endl;

        while (true) {
            packet_size = recv(ClientSock, servBuff.data() + totalReceived, BUFF_SIZE - totalReceived, 0);

            if (packet_size == SOCKET_ERROR) {
                cout << "Can't receive message from Server. Error # " << WSAGetLastError() << endl;
                closesocket(ClientSock);
                WSACleanup();
                return 1;
            }
            if (packet_size == 0) {
                cout << "Server closed the connection." << endl;
                break;
            }

            totalReceived += packet_size;

            if (packet_size < BUFF_SIZE - totalReceived) {
                break;
            }
        }

        cout << endl << "Server message: " << string(servBuff.data(), totalReceived) << endl;
    }

    closesocket(ClientSock);
    WSACleanup();
    return 0;
}
