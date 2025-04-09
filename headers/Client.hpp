#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sys/types.h>

/* Aims to store the client data received by the e-pollserver */
class Client
{
    private:
        int fd;
        std::string hostname;
        std::string nickname;
        std::string username;
        std::string input_buffer;
        bool registered;
        bool isOperator;
    public:
        Client(int fd, std::string hostname);
        ~Client();
        int getFd() const;
        std::string getHostname() const;
        std::string getNickname() const;
        std::string getUsername() const;
        std::string getInputBuffer() const;
        bool isRegistered() const;
        bool isOp() const;

        void setNickname(std::string nickname);
        void setUsername(std::string username);
        void setInputBuffer(std::string input_buffer);
        void setRegistered(bool registered);
        void setOp(bool isOperator);

        void sendMessage(std::string message);
}