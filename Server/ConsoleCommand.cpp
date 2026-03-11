#include "Server.h"

bool Server::isCommand(const std::string &message)
{
    return !message.empty() && message[0] == '/';
}

Command Server::parseCommand(const std::string &message)
{
    if (message.rfind("/help", 0) == 0)
    {
        return Command::HELP;
    }

    if (message.rfind("/join", 0) == 0)
    {
        return Command::JOIN;
    }

    if (message.rfind("/quit", 0) == 0)
    {
        return Command::QUIT;
    }
    return Command::NONE;
}
