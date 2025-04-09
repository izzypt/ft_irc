#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include "Client.hpp"
#include "EpollSocketServer.hpp"

class Controller
{
public:
    Controller();
    ~Controller();

    void run();
    void init();
    void loop();
    void handleClient(int client_fd);
    void handleServer();
    void handleError(const std::string &error_message);
    void handleSIGINT(int signal);
private:
    int server_fd;
    std::vector<int> client_fds;
    std::map<int, Client *> clients;
    std::string server_name;
    std::string server_version;
    std::string server_motd;
    std::string server_info;
    bool running;
}