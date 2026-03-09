#include "Server.h"
#include "TaskManager.h"

// broadcast message to all client except sender
void Server::broadcastMessage(const std::string &message, ClientSession *sender)
{
    std::lock_guard<std::mutex> lock(clientsMutex);
    std::cout << message << "\n";

    for (auto client : clients)
    {
        if (client->ClientSocket != sender->ClientSocket)
        {
            send(client->ClientSocket, message.c_str(), message.size(), 0);
        }
    }
}

// handle client per thread
void Server::handleClient(ClientSession *client)
{
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
        std::string announcement = username + " has join the room";
        taskManager.enqueue(std::make_unique<BroadcastMessage>(this, announcement, client));
        break;
    }

    // process message loop
    while (true)
    {
        iResult = recv(client->ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult <= 0)
            break;
        std::string message(recvbuf, iResult);
        message = client->username + ": " + message;
        taskManager.enqueue(std::make_unique<BroadcastMessage>(this, message, client));
    }
    // cleanup
    removeUser(client);
}

void Server::awaitClientConnection()
{
    std::cout << "awaiting connection on port : " << port << std::endl;

    std::string username;
    while (true)
    {
        SOCKET ClientSocket = accept(listenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET)
        {
            printf("accept failed: %d\n", WSAGetLastError());
            continue;
        }

        ClientSession *client = new ClientSession();
        client->ClientSocket = ClientSocket;

        std::cout << username << "Client connected with socket" << ClientSocket << std::endl;

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back(client);
        }

        std::thread clientThread(&Server::handleClient, this, client);
        clientThread.detach();
    }
}

void Server::removeUser(ClientSession *client)
{
    std::cout << "Removing user: " << client->username << "\n";
    shutdown(client->ClientSocket, SD_BOTH);
    closesocket(client->ClientSocket);
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
    }
    // raw pointer what the fuck
    delete client;
}
