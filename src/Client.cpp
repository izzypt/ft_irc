#include "../headers/Client.hpp"

Client::Client(int new_fd, std::string new_nickname) : fd(new_fd) , nickname(new_nickname) {}

Client::~Client() {}

int Client::getFd() const
{
    return fd;
}

std::string Client::getHostname() const
{
    return hostname;
}

std::string Client::getNickname() const
{
    return nickname;
}

std::string Client::getUsername() const
{
    return username;
}

std::string Client::getRealname() const
{
    return realname;
}

bool Client::isRegistered() const
{
    return registered;
}

bool Client::isOp() const
{
    return isOperator;
}

void Client::setFd(int new_fd)
{
    fd = new_fd;
}

void Client::setHostname(std::string new_hostname)
{
    hostname = new_hostname;
}

void Client::setNickname(std::string new_nickname)
{
    nickname = new_nickname;
}

void Client::setUsername(std::string new_username)
{
    username = new_username;
}

void Client::setRealname(std::string new_realname)
{
    realname = new_realname;
}

void Client::setRegistered(bool new_registered)
{
    registered = new_registered;
}

void Client::setOp(bool new_isOperator)
{
    isOperator = new_isOperator;
}