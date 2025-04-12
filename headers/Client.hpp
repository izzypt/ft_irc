#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>

class Client
{
    private:
        int fd;
        std::string hostname;
        std::string nickname;
        std::string username;
        bool registered;
        bool isOperator;

        Client(const Client&);
        Client& operator=(const Client&);
    public:
        Client(int new_fd, std::string new_nickname);
        ~Client();
        int getFd() const;
        std::string getHostname() const;
        std::string getNickname() const;
        std::string getUsername() const;
        bool isRegistered() const;
        bool isOp() const;

        void setNickname(std::string new_nickname);
        void setUsername(std::string new_username);
        void setRegistered(bool new_registered);
        void setOp(bool new_isOperator);
};

#endif