#pragma once
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class Server;
struct ClientSession;

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

struct Room
{
    std::string name;
    std::vector<ClientSession *> clientList;
};

struct BroadcastMessage : Task
{
    std::string message;
    ClientSession *sender;

    BroadcastMessage(Server *server, std::string &message, ClientSession *sender)
        : Task(server), message(message), sender(sender) {};

    void execute() override;
};
