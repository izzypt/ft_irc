#include "Channel.hpp"
#include "Client.hpp"

Channel::Channel(const std::string& name) : name(name), topic("") {}

Channel::~Channel() {}

const std::string& Channel::getName() const {
    return name;
}

const std::string& Channel::getTopic() const {
    return topic;
}

void Channel::setTopic(const std::string& newTopic) {
    topic = newTopic;
}

void Channel::addClient(Client* client) {
    if (!hasClient(client))
        clients.push_back(client);
}

void Channel::removeClient(Client* client) {
    for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (*it == client) {
            clients.erase(it);
            break;
        }
    }
}

bool Channel::hasClient(Client* client) const {
    for (std::vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
        if (*it == client)
            return true;
    }
    return false;
}
