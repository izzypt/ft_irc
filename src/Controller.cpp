#include "../headers/Controller.hpp"

Controller::Controller(Config &new_config, Log &new_log) : config(new_config) , log(new_log) , epollServer(NULL)
{
    initParseHandlers();
}

Controller::~Controller() {}

void Controller::setEpollSocketServer(EpollSocketServer *new_epollServer)
{
    epollServer = new_epollServer;
}

void Controller::processMessage(int client_fd, std::string message)
{
    std::istringstream msgStream(message);
    std::string CMD;

    // site to follow: https://chi.cs.uchicago.edu/chirc/irc_examples.html

    //std::cout << message << std::endl;
    while (msgStream >> CMD)
    {
        if (parseHandler(client_fd, CMD, msgStream) == -1)
            break;
    }
    /*while (std::getline(stream, line))
    {
        std::cout << "Line: " << line << std::endl;
        // Now split the line into words
        std::istringstream lineStream(line);
        std::string value;
        std::string CMD;
        lineStream >> CMD;
        if (parseHandler(client_fd, CMD, message) == -1)
            break;
    } */

   /*std::vector<int> sendTo;
    std::vector<int> invalids;

    sendTo.push_back(client_fd);
    log.entry("info", message);
    invalids = epollServer->sendMessage(sendTo, message); */
}

void Controller::connectionClosed(int client_fd)
{
    std::map<int, Client *>::iterator it = clients.find(client_fd);
    if (it != clients.end())
        it->second->setFd(-1);
}

int Controller::parseHandler(int fd, const std::string& CMD, std::istringstream &msgStream)
{
    std::map<std::string, int (Controller::*)(int, std::istringstream&)>::iterator it = parseHandlersMap.find(CMD);
    if (it != parseHandlersMap.end())
        return (this->*(it->second))(fd, msgStream);
    return 1;
}

int Controller::authHandler(int fd, std::istringstream &msgStream)
{
    std::string response;
    std::string password;
    std::string nickname;
    std::string username;
    std::string realname;
    std::string CMD;
    std::string trash;
    std::vector<int> sendTo;
    std::vector<int> invalids;

    sendTo.push_back(fd);

    msgStream >> password;
    std::cout << "password " << password << std::endl;
    if (password != config.getPassword())
    {
        response = ":localhost 464 * :Password incorrect\r\n";
        invalids = epollServer->sendMessage(sendTo, response);
        return 0;
    }
    msgStream >> CMD;
    msgStream >> nickname;
    msgStream >> CMD;
    msgStream >> username;
    std::getline(msgStream, trash, ':');
    msgStream >> realname;
    std::map<std::string, Client *>::iterator it = clients_by_nick.find(nickname);
    if (it != clients_by_nick.end())
    {
        it->second->setFd(fd);
        if (username != it->second->getUsername())
            it->second->setUsername(username);
        if (realname != it->second->getRealname())
            it->second->setRealname(realname);
        it->second->setHostname(epollServer->getHostname(fd));
        response = ":localhost 001 " + nickname + " :Welcome to the Internet Relay Network "+ nickname + "!" + username + "@" + it->second->getHostname() + "\r\n";
        invalids = epollServer->sendMessage(sendTo, response);
        return 0;
    }
    Client *newClient = new Client(fd, nickname);
    newClient->setUsername(username);
    newClient->setRealname(realname);
    newClient->setHostname(epollServer->getHostname(fd));
    clients.insert(std::pair<int, Client *>(fd, newClient));
    clients_by_nick.insert(std::pair<std::string, Client *>(nickname, newClient));
    response = ":localhost 001 " + nickname + " :Welcome to the Internet Relay Network "+ nickname + "!" + username + "@" + newClient->getHostname() + "\r\n";
    invalids = epollServer->sendMessage(sendTo, response);
    return 0;
    // no protocol IRC a autenteicao ocorre apos o cliente fazer o envio de 3 dados
    /*
        1 - (Opcional) PASS
        2 - NICK
        3 - USER

        Apos o servidor registar o cliente e se tudo estiver OK, devemos enviar uma mensagem de volta para o cliente no seguinte formato:

        :<server> 001 <nick> :Welcome to the Internet Relay Network <nick>!<user>@<host>

        Se a password estiver incorrecta temos de enviar:

        :<server> 464 * :Password incorrect

        464 'e o numerico para ERR_PASSWDMISMATCH
    */
}

void Controller::initParseHandlers()
{
    parseHandlersMap["PASS"] = &Controller::authHandler;
}