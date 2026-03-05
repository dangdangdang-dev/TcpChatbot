// main
#include "Server.h"
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

int main()
{
    Server server("27015");
    server.start();

    return 0;
}
