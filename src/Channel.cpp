#include "Channel.hpp"
#include "Client.hpp"

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
    std::set<int>::iterator it = clients.find(fd);
    
}

void Channel::removeClient(int fd)
{
    for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (*it == client)
        {
            clients.erase(it);
            break;
        }
    }
}
