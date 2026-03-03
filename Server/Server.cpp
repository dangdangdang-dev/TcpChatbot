// main
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <iostream>
#include <mutex>
#include <stdio.h>
#include <thread>
#include <vector>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

std::vector<SOCKET> _clients;
std::mutex _clientsMutex;

void handleClient(SOCKET ClientSocket)
{
    char recvbuf[DEFAULT_BUFLEN];
    int iResult, iSendResult;
    int recvbuflen = DEFAULT_BUFLEN;

    std::cout << "Client Connected" << std::endl;

    do
    {
        std::cout << "intaking input" << std::endl;
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
        {
            printf("Bytes received: %d\n", iResult);

            // Echo the buffer back to the sender
            iSendResult = send(ClientSocket, recvbuf, iResult, 0);
            if (iSendResult == SOCKET_ERROR)
            {
                printf("send failed: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                break;
            }
            printf("Bytes sent: %d\n", iSendResult);
        }
        else if (iResult == 0)
        {
            printf("Connection closing...\n");
            break;
        }
        else
        {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            break;
        }
    } while (iResult > 0);

    // shut down connection
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
    }

    // cleanup
    closesocket(ClientSocket);
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

    // client socket, currently single thread, only 1 connection can be accepted

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
