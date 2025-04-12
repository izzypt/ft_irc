#include "Client.hpp"

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

bool Client::isRegistered() const
{
    return isRegistered;
}

bool Client::isOp() const
{
    return isOp;
}

void Client::setNickname(std::string new_nickname);
void Client::setUsername(std::string new_username);
void Client::setRegistered(bool new_registered);
void Client::setOp(bool new_isOperator);