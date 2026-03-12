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
        send(client->ClientSocket, message.c_str(), message.size(), 0);
        return;
    }

    rooms.emplace(roomName, Room{});
    std::cout << client->username << "created room: " << roomName;
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

    if (client->currentRoom != "")
    {
        std::string message = "You are in a room already: " + client->currentRoom;
        send(client->ClientSocket, message.c_str(), message.size(), 0);
        return;
    }

    auto &clients = pRoom->second.clientList;
    clients.push_back(client);
    client->currentRoom = roomName;
}

void Server::removeUser(const std::string &roomName, std::shared_ptr<ClientSession> client)
{
    if (client->currentRoom == "")
    {
        std::string message = "You are not in a room";
        send(client->ClientSocket, message.c_str(), message.size(), 0);
        return;
    }
    auto pRoom = rooms.find(roomName);
    if (pRoom == rooms.end())
    {
        std::string message = "No room of this name";
        send(client->ClientSocket, message.c_str(), message.size(), 0);
        return;
    }
    auto &clients = pRoom->second.clientList;
    clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
    client->currentRoom = "";
}
