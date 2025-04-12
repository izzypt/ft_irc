#include <iostream>
#include <sstream>
#include "headers/Config.hpp"
#include "headers/Log.hpp"
#include "headers/EpollSocketServer.hpp"

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
    Controller controller(config, log);

    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 0;
    }

    config.setListenPort(convertToInt(argv[1]));

    config.setPassword(argv[2]);

    server.setController(&controller);

    controller.setEpollSocketServer(&server);

    server.startServer();

    return 0;
}