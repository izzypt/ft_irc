#include "../headers/Controller.hpp"

Controller::Controller(Config &new_config, Log &new_log) : config(new_config) , log(new_log) , epollServer(NULL) {}

Controller::~Controller() {}

void Controller::setEpollSocketServer(EpollSocketServer *new_epollServer)
{
    epollServer = new_epollServer;
}

void Controller::processMessage(int client_fd, std::string message)
{
    std::vector<int> sendTo;
    std::vector<int> invalids;

    sendTo.push_back(client_fd);
    log.entry("info", message);
    invalids = epollServer->sendMessage(sendTo, message);
}