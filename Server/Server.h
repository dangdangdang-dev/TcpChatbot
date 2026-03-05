#pragma once
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <winsock2.h>

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

struct ClientSession
{
    SOCKET ClientSocket;
    std::string username;
};

class WSA
{
  public:
    WSA();
    ~WSA();

  private:
    WSADATA wsaData{};
};

class Server
{
  public:
    const std::string port;

    explicit Server(const std::string &port);
    ~Server();

    void init();
    void start();

    void broadcastMessage(const std::string &message, ClientSession *sender);

  private:
    std::mutex clientsMutex;
    std::vector<ClientSession *> clients;
    SOCKET listenSocket{};

    void awaitClientConnection();
    void handleClient(ClientSession *client);
    void removeUser(ClientSession *client);
};
