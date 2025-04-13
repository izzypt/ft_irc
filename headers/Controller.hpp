#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <iostream>
#include <string>
#include <sstream>
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
        std::map<std::string, Client *> clients_by_nick;
        std::map<std::string, int (Controller::*)(int, std::istringstream&)> parseHandlersMap;

        int parseHandler(int fd, const std::string& CMD, std::istringstream &msgStream);
        int authHandler(int fd, std::istringstream &msgStream);
        void initParseHandlers();

        Controller(const Controller &other);
        Controller& operator=(const Controller &other);
};

#endif