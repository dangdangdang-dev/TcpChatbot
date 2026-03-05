#include "Server.h"
#include <WS2tcpip.h>
#include <iostream>
#include <minwindef.h>
#include <mutex>
#include <ostream>
#include <string>
#include <winsock2.h>

int _result;

// WSA init
WSA::WSA()
{
    _result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (_result != 0)
        throw std::runtime_error("WSAStartup failed");
}

WSA::~WSA()
{
}

// Server init
Server::Server(const std::string &port) : port(port)
{
    WSA wsaData;
    init();
}

Server::~Server()
{
    WSACleanup();
}

void Server::start()
{
    awaitClientConnection();
}

void Server::init()
{
    struct addrinfo *result = NULL, *ptr = NULL, hints{};
    int iResult;

    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // addrinfo create -> iResult
    iResult = getaddrinfo(NULL, port.c_str(), &hints, &result);
    if (iResult != 0)
    {
        printf("Get addrinfo fail with error %d\n", iResult);
        WSACleanup();
    }

    // listen socket create
    listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listenSocket == INVALID_SOCKET)
    {
        printf("Error at socket(): %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
    }

    // disable ipv6 only
    int no = 0;
    setsockopt(listenSocket, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&no, sizeof(no));

    // bind listensocket
    iResult = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(listenSocket);
        WSACleanup();
    }

    freeaddrinfo(result);

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("Listen failed with error : %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
    }
}

void Server::broadcastMessage(const std::string &message, ClientSession *sender)
{
    std::lock_guard<std::mutex> lock(clientsMutex);

    for (auto client : clients)
    {
        if (client->ClientSocket != sender->ClientSocket)
        {
            send(client->ClientSocket, message.c_str(), message.size(), 0);
        }
    }
}

// handle client per thread
void Server::handleClient(ClientSession *client)
{
    char recvbuf[DEFAULT_BUFLEN];
    int iResult, iSendResult;
    int recvbuflen = DEFAULT_BUFLEN;
    std::string pending;

    // get username

    while (true)
    {
        iResult = recv(client->ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult <= 0)
            break;

        std::string username(recvbuf, iResult);
        client->username = username;

        if (client->username != "")
            break;

        // pending.append(recvbuf, iResult);
        //
        // size_t pos;
        //
        // while ((pos = pending.find('\n')) != std::string::npos)
        // {
        //     std::cout << "processing name" << std::endl;
        //     std::string username = pending.substr(0, pos);
        //     pending.erase(0, pos + 1);
        //     client->username = username;
        //     break;
        // }
    }

    while (true)
    {
        iResult = recv(client->ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult <= 0)
            break;

        std::string message(recvbuf, iResult);
        std::cout << client->username << ": " << message << "\n";

        // pending.append(recvbuf, iResult);

        // size_t pos;

        // while ((pos = pending.find('\n')) != std::string::npos)
        // {
        //     std::string message = pending.substr(0, pos);
        //     pending.erase(0, pos + 1);
        //     broadcastMessage(message, client);
        // }
    }
    // cleanup
    removeUser(client);
}

void Server::awaitClientConnection()
{
    std::cout << "awaiting connection on port : " << port << std::endl;

    std::string username;
    while (true)
    {
        SOCKET ClientSocket = accept(listenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET)
        {
            printf("accept failed: %d\n", WSAGetLastError());
            continue;
        }

        ClientSession *client = new ClientSession();
        client->ClientSocket = ClientSocket;

        std::cout << "Client connected with socket" << client->ClientSocket << std::endl;

        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.push_back(client);

        std::thread clientThread(&Server::handleClient, this, client);
        clientThread.detach();
    }
}

void Server::removeUser(ClientSession *client)
{
    std::cout << "Removing user: " << client->username << "\n";
    shutdown(client->ClientSocket, SD_BOTH);
    closesocket(client->ClientSocket);
    std::lock_guard<std::mutex> lock(clientsMutex);
    clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
    // raw pointer what the fuck
    delete client;
}
