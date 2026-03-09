#include "TaskManager.h"
#include "Server.h"
#include <iostream>
#include <memory>
#include <mutex>

TaskManager::TaskManager(size_t threadCount) : stop(false)
{
    for (size_t i = 0; i < threadCount; i++)
    {
        workers.emplace_back(&TaskManager::workerLoop, this);
    }
}

void TaskManager::workerLoop()
{
    std::unique_ptr<Task> task;

    while (true)
    {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] { return stop || !taskQueue.empty(); });

            if (stop && taskQueue.empty())
                return;

            task = std::move(taskQueue.front());
            taskQueue.pop();
        }
        task->execute();
    }
}

void TaskManager::enqueue(std::unique_ptr<Task> task)
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push(std::move(task));
    }
    condition.notify_one();
};

void BroadcastMessage::execute()
{
    std::lock_guard<std::mutex> lock(server->clientsMutex);
    std::cout << message << "\n";

    for (auto client : server->clients)
    {
        if (client->ClientSocket != sender->ClientSocket)
        {
            send(client->ClientSocket, message.c_str(), message.size(), 0);
        }
    }
}

TaskManager::~TaskManager()
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stop = true;
    }

    condition.notify_all();

    for (size_t i = 0; i < workers.size(); i++)
    {
        if (workers[i].joinable())
            workers[i].join();
    }
}
