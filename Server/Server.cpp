// main
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <iostream>
#include <mutex>
#include <stdio.h>
#include <string>
#include <thread>
#include <vector>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

std::vector<SOCKET> _clients;
std::mutex _clientsMutex;

void broadcastMessage(const std::string &message, SOCKET sender)
{
    std::lock_guard<std::mutex> lock(_clientsMutex);

    for (auto client : _clients)
    {
        if (client != sender)
        {
            send(client, message.c_str(), message.size(), 0);
        }
    }
}

void handleClient(SOCKET ClientSocket)
{
    char recvbuf[DEFAULT_BUFLEN];
    int iResult, iSendResult;
    int recvbuflen = DEFAULT_BUFLEN;

    std::cout << "Client Connected" << std::endl;

    while (true)
    {
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);

        if (iResult <= 0)
            break;

        std::string message(recvbuf, iResult);
        std::cout << "Message: " << message << "\n";

        broadcastMessage(message, ClientSocket);
    };
    // cleanup
    closesocket(ClientSocket);

    std::lock_guard<std::mutex> lock(_clientsMutex);
    _clients.erase(std::remove(_clients.begin(), _clients.end(), ClientSocket), _clients.end());
}

int main()
{
    WSADATA wsaData;
    struct addrinfo *result = NULL, *ptr = NULL, hints{};
    int iResult;

    // init winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed : %d\n", iResult);
        return 1;
    }

    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // addrinfo create -> iResult
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        printf("Get addrinfo fail with error %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // listen socket create
    SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
        printf("Error at socket(): %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // disable ipv6 only
    int no = 0;
    setsockopt(ListenSocket, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&no, sizeof(no));

    // bind listensocket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("Listen failed with error : %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "awaiting connection on port : " << DEFAULT_PORT << std::endl;

    // accept connection loop
    while (true)
    {
        SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET)
        {
            printf("accept failed: %d\n", WSAGetLastError());
            continue;
        }
        std::lock_guard<std::mutex> lock(_clientsMutex);
        _clients.push_back(ClientSocket);

        std::thread clientThread(handleClient, ClientSocket);
        clientThread.detach();
    }

    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}
