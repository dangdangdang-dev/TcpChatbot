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

enum class ClientState
{
    PENDING_CONNECTION,
};

struct ClientSession
{
    SOCKET ClientSocket;
    std::string username;
};

struct Room
{
    std::string name;
    std::vector<std::shared_ptr<ClientSession>> clientList;
};
;

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

    std::mutex clientsMutex;
    std::vector<std::shared_ptr<ClientSession>> clients;

    std::mutex roomMutex;
    std::vector<Room> rooms;

    TaskManager taskManager;

  private:
    SOCKET listenSocket{};
};
