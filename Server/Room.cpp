#include "Server.h"
#include <algorithm>
#include <memory>
#include <string>
#include <winsock2.h>

// Room operation
void Server::createRoom(const std::string roomName, std::shared_ptr<ClientSession> client)
{
    auto room = rooms.find(roomName);

    if (room != rooms.end())
    {
        std::string message = "There is a room with the same name already";
        taskManager.enqueue(std::make_unique<NotifyUser>(this, message, client));
        return;
    }

    rooms.emplace(roomName, Room{});

    rooms[roomName].clientList.push_back(client);
    clients.push_back(client);
    client->currentRoom = roomName;

    std::cout << client->username << "created room: " << roomName;
}

void Server::addUser(const std::string roomName, std::shared_ptr<ClientSession> client)
{
    auto pRoom = rooms.find(roomName);

    if (pRoom == rooms.end())
    {
        std::string message = "No room of this name";
        taskManager.enqueue(std::make_unique<NotifyUser>(this, message, client));
        return;
    }

    if (client->currentRoom == roomName)
    {
        std::string message = "You are in a room already: " + client->currentRoom;
        taskManager.enqueue(std::make_unique<NotifyUser>(this, message, client));
        return;
    }

    auto &clients = pRoom->second.clientList;
    clients.push_back(client);
    client->currentRoom = roomName;
}

void Server::removeUser(const std::string roomName, std::shared_ptr<ClientSession> client)
{
    if (client->currentRoom == "")
    {
        std::string message = "You are not in a room";
        taskManager.enqueue(std::make_unique<NotifyUser>(this, message, client));
        return;
    }
    auto pRoom = rooms.find(roomName);
    if (pRoom == rooms.end())
    {
        std::string message = "No room of this name";
        taskManager.enqueue(std::make_unique<NotifyUser>(this, message, client));
        return;
    }
    auto &clients = pRoom->second.clientList;
    clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
    client->currentRoom = "";

    if (rooms.empty())
    {
        rooms.erase(pRoom);
    }
}

void Server::loadMessage(std::shared_ptr<ClientSession> client)
{
    const auto &messageList = rooms[client->currentRoom].messageHistory;

    for (auto message : messageList)
    {
        message = message + '\n';
        send(client->ClientSocket, message.c_str(), message.size(), 0);
    }
}
