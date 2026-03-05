#include "Server.h"
#include <WS2tcpip.h>
#include <minwindef.h>
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
