#include "Server.h"
#include <algorithm>
#include <string>
#include <winsock2.h>

void Server::createRoom(const std::string &roomName, std::shared_ptr<ClientSession> client)
{
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
