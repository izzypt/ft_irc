#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <iostream>
#include <string>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include "Config.hpp"
#include "Log.hpp"
#include "Client.hpp"
#include "EpollSocketServer.hpp"

class EpollSocketServer;

class Controller
{
    public:
        Controller(Config &new_config, Log &new_log);
        ~Controller();
        
        void setEpollSocketServer(EpollSocketServer *new_server);
        void processMessage(int client_fd, std::string message);
        void connectionClosed(int client_fd);
    private:
        Config &config;
        Log &log;
        EpollSocketServer* epollServer;
        std::map<int, Client *> clients;

        Controller(const Controller &other);
        Controller& operator=(const Controller &other);
};

#endif