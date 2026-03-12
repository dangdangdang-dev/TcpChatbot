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
    std::shared_ptr<ClientSession> client;

  public:
    Task(Server *server, std::shared_ptr<ClientSession> client) : server(server), client(client) {};
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

    BroadcastMessage(Server *server, std::string &message, std::shared_ptr<ClientSession> client)
        : Task(server, client), message(message) {};

    void execute() override;
};

struct RemoveUser : Task
{
    RemoveUser(Server *server, std::shared_ptr<ClientSession> client) : Task(server, client) {};

    void execute() override;
};

// ROOM TASK STRUCT

struct RoomTask : Task
{
    RoomTask(Server *server, std::shared_ptr<ClientSession> client, const std::string roomName)
        : Task(server, client), roomName(roomName) {};
    const std::string roomName;
};

struct JoinRoom : RoomTask
{
    JoinRoom(Server *server, std::shared_ptr<ClientSession> client, const std::string &roomName)
        : RoomTask(server, client, roomName) {};

  private:
    void execute() override;
};

struct QuitRoom : Task
{
    QuitRoom(Server *server, std::shared_ptr<ClientSession> client) : Task(server, client) {};

  private:
    void execute() override;
};

struct CreateRoom : RoomTask
{
    CreateRoom(Server *server, std::shared_ptr<ClientSession> client, const std::string roomName)
        : RoomTask(server, client, roomName) {};

  private:
    void execute() override;
};
