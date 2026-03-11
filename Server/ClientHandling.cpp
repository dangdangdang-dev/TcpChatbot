#include "Server.h"
#include "TaskManager.h"
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <winsock2.h>

void Server::recvLoop()
{
    char recvbuf[DEFAULT_BUFLEN];
    int iResult, iSendResult;
    int recvbuflen = DEFAULT_BUFLEN;

    while (true)
    {
        std::vector<std::shared_ptr<ClientSession>> snapshot;
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            snapshot = clients;
        }

        for (auto &client : snapshot)
        {
            int result = recv(client->ClientSocket, recvbuf, recvbuflen, 0);

            if (result > 0)
            {
                std::string message(recvbuf, result);
                processMessage(client, message);
                continue;
            }

            if (result == 0)
            {
                taskManager.enqueue(std::make_unique<RemoveUser>(this, client));
                continue;
            }

            // result == SOCKET_ERROR
            int err = WSAGetLastError();

            if (err == WSAEWOULDBLOCK)
                continue;

            taskManager.enqueue(std::make_unique<RemoveUser>(this, client));
        }
    }
}

void Server::awaitClientConnection()
{
    std::cout << "awaiting connection on port : " << port << std::endl;

    while (true)
    {
        SOCKET ClientSocket = accept(listenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                printf("accept failed: %d\n", WSAGetLastError());
            }
            continue;
        }

        auto client = std::make_shared<ClientSession>();

        client->ClientSocket = ClientSocket;
        setUsername(client);

        std::cout << client->username << "Client connected with socket" << client->ClientSocket
                  << std::endl;

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back(client);
        }
    }
}

void Server::setUsername(std::shared_ptr<ClientSession> client)
{
    std::string username;

    char recvbuf[DEFAULT_BUFLEN];
    int iResult, iSendResult;
    int recvbuflen = DEFAULT_BUFLEN;
    std::string pending;

    // get username

    while (true)
    {
        iResult = recv(client->ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult <= 0)
            break;
        std::string username(recvbuf, iResult);
        client->username = username;
        break;
    }
}

void Server::processMessage(std::shared_ptr<ClientSession> client, std::string &message)
{
    if (isCommand(message))
    {
        const Command &cmd = parseCommand(message);
        const std::string &argument = getCommandArgument(message);
        if (argument == "")
            return;
        handleCommand(client, cmd, argument);
        return;
    }
    message = client->username + ": " + message;
    taskManager.enqueue(std::make_unique<BroadcastMessage>(this, message, client));
}

std::string Server::getCommandArgument(const std::string &message)
{
    size_t pos = message.find(' ');
    if (pos == std::string::npos)
        return "";

    return message.substr(pos + 1);
}

void Server::handleCommand(std::shared_ptr<ClientSession> client, const Command &cmd,
                           const std::string &argument)
{
    switch (cmd)
    {
    case Command::HELP:
    {
        std::string msg = "HELP JOIN QUIT";
        send(client->ClientSocket, msg.c_str(), msg.size(), 0);
        break;
    }
    case Command::JOIN:
    {
        taskManager.enqueue(std::make_unique<JoinRoom>(this, client, argument));
    }
    case Command::QUIT:
    {
        taskManager.enqueue(std::make_unique<QuitRoom>(this, client, argument));
    }
    case Command::CREATE:
    {
        taskManager.enqueue(std::make_unique<CreateRoom>(this, client, argument));
    }
    default:
    case Command::NONE:
        std::cout << "what the fuck" << std::endl;
    }
}
