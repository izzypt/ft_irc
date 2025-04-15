#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <set>

class Client
{
    public:
        Client(int new_fd, std::string new_nickname);
        ~Client();
        int getFd() const;
        std::string getHostname() const;
        std::string getNickname() const;
        std::string getUsername() const;
        std::string getRealname() const;
        bool isRegistered() const;
        bool isOp() const;

        void setFd(int new_fd);
        void setHostname(std::string new_hostname);
        void setNickname(std::string new_nickname);
        void setUsername(std::string new_username);
        void setRealname(std::string new_realname);
        void setRegistered(bool new_registered);
        void setOp(bool new_isOperator);
        void addChannel(std::string name);
        void removeChannel(std::string name);
        std::set<std::string> getClientChannels();
        void clearData();
    private:
        int fd;
        std::string hostname;
        std::string nickname;
        std::string username;
        std::string realname;
        bool registered;
        bool isOperator;
        std::set<std::string> channels;

        Client(const Client&);
        Client& operator=(const Client&);
};

#endif