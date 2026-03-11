#include "Server.h"
#include <algorithm>
#include <memory>
#include <string>
#include <winsock2.h>

// Room operation
void Server::createRoom(const std::string &roomName, std::shared_ptr<ClientSession> client)
{
    auto it = rooms.find(roomName);

    if (it != rooms.end())
        return;

    rooms.emplace(roomName, Room{});
    taskManager.enqueue(std::make_unique<JoinRoom>(this, client, roomName));
}

void Server::addUser(const std::string &roomName, std::shared_ptr<ClientSession> client)
{
    auto pRoom = rooms.find(roomName);
    if (pRoom == rooms.end())
    {
        std::string message = "No room of this name";
        send(client->ClientSocket, message.c_str(), message.size(), 0);
        return;
    }
    pRoom->second.clientList.push_back(client);
}

void Server::removeUser(const std::string &roomName, std::shared_ptr<ClientSession> client)
{
    auto pRoom = rooms.find(roomName);
    if (pRoom == rooms.end())
    {
        std::string message = "No room of this name";
        send(client->ClientSocket, message.c_str(), message.size(), 0);
        return;
    }
    auto clients = pRoom->second.clientList;
    clients.erase(std::remove(clients.begin(), clients.end(), client));
}
