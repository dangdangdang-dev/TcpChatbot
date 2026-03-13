#pragma once
#include "TaskManager.h"
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>
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
    std::string currentRoom;
};

struct Room
{
    std::vector<std::shared_ptr<ClientSession>> clientList;
    std::vector<std::string> messageHistory;
};

enum class Command
{
    JOIN,
    QUIT,
    HELP,
    NONE,
    CREATE
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
    void processMessage(std::shared_ptr<ClientSession>, std::string &messsage);
    void setUsername(std::shared_ptr<ClientSession>);

    bool isCommand(const std::string &message);
    Command parseCommand(const std::string &messsage);
    void handleCommand(std::shared_ptr<ClientSession> client, const Command &cmd,
                       const std::string argument);
    std::string getCommandArgument(const std::string &message);

    std::mutex clientsMutex;
    std::vector<std::shared_ptr<ClientSession>> clients;

    std::mutex roomMutex;
    std::unordered_map<std::string, Room> rooms;
    void createRoom(const std::string roomName, std::shared_ptr<ClientSession> client);
    void addUser(const std::string roomName, std::shared_ptr<ClientSession> client);
    void removeUser(const std::string roomName, std::shared_ptr<ClientSession> client);
    void loadMessage(std::shared_ptr<ClientSession> client);

    TaskManager taskManager;

  private:
    SOCKET listenSocket{};
};
