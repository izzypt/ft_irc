#include "../headers/Channel.hpp"

Channel::Channel(const std::string& name) : name(name), topic("") {}

Channel::~Channel() {}

const std::string& Channel::getName() const
{
    return name;
}

const std::string& Channel::getTopic() const
{
    return topic;
}

void Channel::setTopic(const std::string& newTopic)
{
    topic = newTopic;
}

void Channel::addClient(int fd)
{
    clients.insert(fd);
}

void Channel::removeClient(int fd)
{
    clients.erase(fd);
}

std::set<int> Channel::getChannelClients()
{
    return clients;
}

void Channel::clearData()
{
    clients.clear();
}