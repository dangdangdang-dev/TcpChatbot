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

    if (!client)
        return;

    std::cout << message << "\n";

    auto pRoom = server->rooms.find(client->currentRoom);

    if (pRoom == server->rooms.end())
        return;

    auto &roomClient = pRoom->second.clientList;

    for (auto &otherClient : roomClient)
    {
        if (otherClient->ClientSocket != client->ClientSocket)
        {
            std::cout << client->username << " send a message in " << client->currentRoom
                      << std::endl;
            send(otherClient->ClientSocket, message.c_str(), message.size(), 0);
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
    server->removeUser(client->currentRoom, client);
}
