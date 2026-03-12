#include "Server.h"

bool Server::isCommand(const std::string &message)
{
    return !message.empty() && message[0] == '/';
}

Command Server::parseCommand(const std::string &message)
{
    // ROOM COMMAND
    if (message.rfind("/help", 0) == 0)
        return Command::HELP;
    if (message.rfind("/join", 0) == 0)
        return Command::JOIN;
    if (message.rfind("/quit", 0) == 0)
        return Command::QUIT;
    if (message.rfind("/create", 0) == 0)
        return Command::CREATE;
    return Command::NONE;
}

void Server::handleCommand(std::shared_ptr<ClientSession> client, const Command &cmd,
                           const std::string argument)
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
        break;
    }
    case Command::QUIT:
    {
        taskManager.enqueue(std::make_unique<QuitRoom>(this, client));
        break;
    }
    case Command::CREATE:
    {
        taskManager.enqueue(std::make_unique<CreateRoom>(this, client, argument));
        break;
    }
    default:
    case Command::NONE:
        std::cout << "Invalid command" << std::endl;
    }
}

std::string Server::getCommandArgument(const std::string &message)
{
    size_t pos = message.find(' ');
    if (pos == std::string::npos)
        return "";

    std::string arg = message.substr(pos + 1);

    while (!arg.empty() && (arg.back() == '\n' || arg.back() == '\r'))
        arg.pop_back();

    return arg;
}
