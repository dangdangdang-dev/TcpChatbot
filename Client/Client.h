#pragma once

#include <string>
#include <winsock2.h>
class Client
{
  public:
    std::string username;
    SOCKET ClientSocket;
    const std::string port;

    explicit Client(const std::string &port);
    ~Client();

    void init(char *serverName);

    void startConnect();

  private:
    struct addrinfo *result = NULL, *ptr = NULL, hints{};
    void handleConnection();
    void receivedMessage();
    void disconnectServer();
};
