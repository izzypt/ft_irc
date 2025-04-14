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
        void clearData();
    private:
        Config &config;
        Log &log;
        EpollSocketServer* epollServer;
        std::map<int, Client *> clients;
        std::map<std::string, Client *> clients_by_nick;
        std::map<std:::string, Channel *> channels;
        std::map<std::string, int (Controller::*)(int, std::string)> parseHandlersMap;
        std::map<int, std::string> fdBuffers;

        void sendResponse(std::vector<int> sendTo, std::string response);

        int parseHandler(int fd, const std::string& CMD, std::string content);
        int authHandler(int fd, std::string password);
        int nickHandler(int fd, std::string nickname);
        int userHandler(int fd, std::string userData);
        int quitHandler(int fd, std::string quitMessage);
        void initParseHandlers();
        
        void saveBuffer(int fd, std::string buffer);
        std::string loadBuffer(int fd);
        std::string escapeVisible(const std::string& input);

        void removeClients();
        void removeChannels();
        void clearContainers();

        Controller(const Controller &other);
        Controller& operator=(const Controller &other);
};

#endif