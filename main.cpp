#include <iostream>
#include "Config.hpp"
#include "Log.hpp"
#include "EpollSocketServer.hpp"

int main()
{
    Config config;
    Log log(config.getLogFolder());
    EpollSocketServer server(config, log);

    server.startServer();
    return 0;
}