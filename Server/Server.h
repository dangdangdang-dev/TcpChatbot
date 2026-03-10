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

    void awaitClientConnection();
    void recvLoop();
    void setUsername(std::shared_ptr<ClientSession>);
    std::vector<std::shared_ptr<ClientSession>> clients;
    std::mutex clientsMutex;

    TaskManager taskManager;

  private:
    SOCKET listenSocket{};
};
