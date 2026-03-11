#pragma once
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class Server;
struct ClientSession;
struct Room;

struct Task
{
  protected:
    Server *server;

  public:
    Task(Server *server) : server(server) {};
    virtual ~Task() = default;
    virtual void execute() = 0;
};

class TaskManager
{
  public:
    std::queue<std::unique_ptr<Task>> taskQueue;
    TaskManager(size_t threadCount);
    ~TaskManager();

    void workerLoop();
    void enqueue(std::unique_ptr<Task> task);

  private:
    std::vector<std::thread> workers;
    size_t threadCount;
    std::mutex queueMutex;
    std::condition_variable condition;

    bool stop;
};

struct BroadcastMessage : Task
{
    std::string message;
    std::shared_ptr<ClientSession> sender;

    BroadcastMessage(Server *server, std::string &message, std::shared_ptr<ClientSession> sender)
        : Task(server), message(message), sender(sender) {};

    void execute() override;
};

struct RemoveUser : Task
{
    std::shared_ptr<ClientSession> client;

    RemoveUser(Server *server, std::shared_ptr<ClientSession> client)
        : Task(server), client(client) {};

    void execute() override;
};

struct getRoom : Task
{
};

struct joinRoom : Task
{
    joinRoom(Server *server, std::shared_ptr<ClientSession> client,
             std::vector<std::shared_ptr<ClientSession>> room)
        : Task(server), client(client), room(room) {};

    std::shared_ptr<ClientSession> client;
    std::vector<std::shared_ptr<ClientSession>> room;

    void execute() override;
};
