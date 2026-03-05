// main
#include <string>
#include <thread>
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

void receivedMessage(SOCKET ConnectSocket)
{
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    while (true)
    {
        int bytesReveived = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (bytesReveived <= 0)
            break;

        std::cout << "\n" << std::string(recvbuf, bytesReveived) << "\n";
    }
}

std::string getUsername()
{
    std::string username{};
    while (username.length() < 4)
    {
        std::cout << "Input username, minimum 4 characters: ";
        std::getline(std::cin, username);
    }
    return username;
}

int __cdecl main(int argc, char **argv)
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL, *ptr = NULL, hints{};
    const char *sendbuf = "this is a test";
    int iResult;

    // Validate the parameters
    if (argc != 2)
    {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // get username before connection
    std::string username = getUsername();

    std::cout << "connection attempt" << std::endl;
    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            printf("socket failed with error: %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            printf("socket cant connect with family: %d\n", ptr->ai_family);
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    send(ConnectSocket, username.c_str(), username.size(), 0);

    std::thread recvThread(receivedMessage, ConnectSocket);
    // Send message loop
    std::string input;
    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, input);
        send(ConnectSocket, input.c_str(), input.size(), 0);
    }

    // // shutdown the connection since no more data will be sent
    // iResult = shutdown(ConnectSocket, SD_SEND);
    // if (iResult == SOCKET_ERROR)
    // {
    //     printf("shutdown failed with error: %d\n", WSAGetLastError());
    //     closesocket(ConnectSocket);
    //     WSACleanup();
    //     return 1;
    // }

    std::cout << "shutting down" << std::endl;
    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
