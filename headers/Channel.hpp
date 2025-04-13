#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <map>

class Client;

class Channel
{
    public:
        Channel(const std::string& name);
        ~Channel();

        const std::string& getName() const;
        const std::string& getTopic() const;
        void setTopic(const std::string& newTopic);

        void addClient(Client* client);
        void removeClient(Client* client);
        bool hasClient(Client* client) const;

        void broadcast(const std::string& message, Client* excludeClient = NULL);

    private:
        std::string name;
        std::string topic;
        std::vector<Client*> clients;

        Channel(const Channel& other);
        Channel& operator=(const Channel& other);
};

#endif
