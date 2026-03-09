#pragma once
#include "TaskManager.h"
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
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

class Server
{
  public:
    const std::string port;

    explicit Server(const std::string &port);
    ~Server();

    void init();
    void start();

    void broadcastMessage(const std::string &message, ClientSession *sender);
    void handleClient(ClientSession *client);
    void awaitClientConnection();
    void removeUser(ClientSession *client);
    std::vector<ClientSession *> clients;
    std::mutex clientsMutex;

    TaskManager taskManager;

  private:
    SOCKET listenSocket{};
};
