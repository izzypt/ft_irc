#include <iostream>
#include <sstream>
#include "Config.hpp"
#include "Log.hpp"
#include "EpollSocketServer.hpp"

int convertToInt(std::string nbr_str)
{
    int nbr;
    std::istringstream(nbr_str) >> nbr;
    return nbr;
}

int main(int argc, char **argv)
{
    Config config;
    Log log(config.getLogFolder());
    EpollSocketServer server(config, log);

    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 0;
    }

    config.setListenPort(convertToInt(argv[1]));

    config.setPassword(argv[2]);

    server.startServer();

    return 0;
}