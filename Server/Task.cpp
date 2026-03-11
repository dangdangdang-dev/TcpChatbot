#include "Server.h"
#include "TaskManager.h"
#include <algorithm>
#include <iostream>
#include <mutex>

// SERVER TASK
void RemoveUser::execute()
{
    std::lock_guard<std::mutex> lock(server->clientsMutex);

    shutdown(client->ClientSocket, SD_BOTH);
    closesocket(client->ClientSocket);

    server->clients.erase(std::remove(server->clients.begin(), server->clients.end(), client),
                          server->clients.end());
    std::cout << client->username << " has disconnected\n";
}

// BROADCAST TASK
void BroadcastMessage::execute()
{
    std::lock_guard<std::mutex> lock(server->roomMutex);
    std::cout << message << "\n";

    auto room = server->rooms[client->currentRoom].clientList;

    for (auto &client : room)
    {
        if (client->ClientSocket != this->client->ClientSocket)
        {
            send(client->ClientSocket, message.c_str(), message.size(), 0);
        }
    }
}

// ROOM TASK
void CreateRoom::execute()
{
    std::lock_guard<std::mutex> lock(server->roomMutex);
    server->createRoom(roomName, client);
}

void JoinRoom::execute()
{
    std::lock_guard<std::mutex> lock(server->roomMutex);
    server->addUser(roomName, client);
}

void QuitRoom::execute()
{
    std::lock_guard<std::mutex> lock(server->roomMutex);
    server->removeUser(client);
}
