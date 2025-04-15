#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>

class Client;

class Channel
{
    public:
        Channel(const std::string& name);
        ~Channel();

        const std::string& getName() const;
        const std::string& getTopic() const;
        void setTopic(const std::string& newTopic);

        void addClient(int fd);
        void removeClient(int fd);
        std::set<int> getChannelClients();
        void clearData();

    private:
        std::string name;
        std::string topic;
        std::set<int> clients;

        Channel(const Channel& other);
        Channel& operator=(const Channel& other);
};

#endif
