#include "../headers/Controller.hpp"

Controller::Controller(Config &new_config, Log &new_log) : config(new_config) , log(new_log) {}

Controller::Controller(const Controller &other) : config(other.config) , log(other.log) {}

Controller& Controller::operator=(const Controller &other)
{
    if (this != &other)
    {
        config = other.config;
        log = other.log;
    }
    return *this;
}

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