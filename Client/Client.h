#pragma once

#include <string>
class Client
{
  public:
    std::string username;

  private:
    std::string getUsername()
    {
        return username;
    }
};
