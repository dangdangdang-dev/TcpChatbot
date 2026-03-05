#include "Client.h"
#include <iostream>
#include <thread>
#include <ws2tcpip.h>

#define DEFAULT_BUFLEN 512

// 0 sucess, non 0 if fail
int iResult;

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

void refreshLine()
{
    std::cout << "> ";
}

Client::Client(const std::string &port) : port(port)
{
}

Client::~Client()
{
}

void Client::init(char *serverName)
{
    WSADATA wsaData;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
    }

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo(serverName, port.c_str(), &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
    }

    username = getUsername();
}

void Client::startConnect()
{
    std::cout << "connection attempt" << std::endl;
    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {

        // Create a SOCKET for connecting to server
        ClientSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ClientSocket == INVALID_SOCKET)
        {
            printf("socket failed with error: %d\n", WSAGetLastError());
            WSACleanup();
        }

        // Connect to server.
        iResult = connect(ClientSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            printf("socket cant connect with family: %d\n", ptr->ai_family);
            closesocket(ClientSocket);
            ClientSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ClientSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
    }

    handleConnection();
}

void Client::handleConnection()
{

    send(ClientSocket, username.c_str(), username.size(), 0);

    std::thread recvThread(&Client::receivedMessage, this);
    recvThread.detach();

    std::string input;

    while (true)
    {
        refreshLine();
        std::getline(std::cin, input);
        send(ClientSocket, input.c_str(), input.size(), 0);
    }
}

void Client::disconnectServer()
{
    std::cout << "disconnected from server" << "\n";
    shutdown(ClientSocket, SD_BOTH);
    closesocket(ClientSocket);
}

void Client::receivedMessage()
{
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN, bytesReveived;

    while (true)
    {
        bytesReveived = recv(ClientSocket, recvbuf, recvbuflen, 0);

        if (bytesReveived <= 0)
        {
            disconnectServer();
            break;
        }

        std::string message(recvbuf, bytesReveived);
        std::cout << "\n" << message << "\n" << "> ";
    }
}
