#include "../headers/Controller.hpp"

Controller::Controller(Config &new_config, Log &new_log) : config(new_config) , log(new_log) , epollServer(NULL)
{
    initParseHandlers();
}

Controller::~Controller()
{
    clearData();
}

void Controller::setEpollSocketServer(EpollSocketServer *new_epollServer)
{
    epollServer = new_epollServer;
}

void Controller::processMessage(int client_fd, std::string message)
{
    std::string CMD;
    std::string buffer;

    // site to follow: https://chi.cs.uchicago.edu/chirc/irc_examples.html
    std::cout << escapeVisible(message) << std::endl;
    message = loadBuffer(client_fd) + message;
    std::istringstream msgStream(message);
    while (std::getline(msgStream, buffer))
    {
        if (!buffer.empty() && buffer[buffer.size() - 1] == '\r')
        {
            buffer.erase(buffer.size() - 1);
            std::cout << buffer << std::endl;
            std::istringstream bufferStream(buffer);
            bufferStream >> CMD;
            bufferStream >> std::ws;
            std::string content((std::istreambuf_iterator<char>(bufferStream)), std::istreambuf_iterator<char>());
            if (parseHandler(client_fd, CMD, content) == -1)
                break;
        }
        else
        {
            saveBuffer(client_fd, buffer);
        }
    }
}

void Controller::connectionClosed(int client_fd)
{
    std::map<int, Client *>::iterator it = clients.find(client_fd);
    if (it != clients.end())
    {
        it->second->setFd(-1);
        clients.erase(it);
    }
    std::cout << "Connection closed by client" << std::endl;
}

void Controller::sendResponse(std::vector<int> sendTo, std::string response)
{
    std::vector<int> invalids;
    std::vector<int>::iterator it;

    response += "\r\n"; 

    invalids = epollServer->sendMessage(sendTo, response);

    for (it = invalids.begin(); it != invalids.end(); ++it)
        connectionClosed(*it);
}

void Controller::clearData()
{
    removeClients();
    clearContainers();
}

int Controller::parseHandler(int fd, const std::string& CMD, std::string content)
{
    std::map<std::string, int (Controller::*)(int, std::string)>::iterator it = parseHandlersMap.find(CMD);
    if (it != parseHandlersMap.end())
        return (this->*(it->second))(fd, content);
    return 1;
}

int Controller::authHandler(int fd, std::string password)
{
    std::cout << "password " << password << std::endl;
    if (password != config.getPassword())
    {
        std::vector<int> sendTo;

        sendTo.push_back(fd);
        sendResponse(sendTo, ":localhost 464 * :Password incorrect");
        return 1;
    }
    return 0;
}

int Controller::nickHandler(int fd, std::string nickname)
{
    std::map<std::string, Client *>::iterator it = clients_by_nick.find(nickname);
    std::string response;

    if (it != clients_by_nick.end())
    {
        if (it->second->getFd() == -1)
        {
            it->second->setFd(fd);
            return 0;
        }
        else if (it->second->getFd() != fd)
        {
            std::vector<int> sendTo;

            sendTo.push_back(fd);
            response = ":" + epollServer->getServerHostname() + " 433 * " + nickname + " :Nickname is already in use";
            sendResponse(sendTo, response);
            return 0;
        }
    }
    std::map<int, Client *>::iterator itf = clients.find(fd);
    if (itf != clients.end())
    {
        std::string oldNick = itf->second->getNickname();
        itf->second->setNickname(nickname);

        it = clients_by_nick.find(oldNick);
        if (it != clients_by_nick.end())
            clients_by_nick.erase(it);
        
        clients_by_nick.insert(std::pair<std::string, Client *>(nickname, itf->second));

        std::vector<int> sendTo;
        //TODO get file descriptors of all users of user channels
        sendTo.push_back(fd);
        response = ":" + oldNick + "!" + itf->second->getUsername() + "@" + itf->second->getHostname() + " NICK :" + nickname;
        sendResponse(sendTo, response);
        return 0;
    }
    else
    {
        Client *newClient = new Client(fd, nickname);
        clients.insert(std::pair<int, Client *>(fd, newClient));
        clients_by_nick.insert(std::pair<std::string, Client *>(nickname, newClient));
    }
    return 0;
}

int Controller::userHandler(int fd, std::string userData)
{
    std::map<int, Client *>::iterator it = clients.find(fd);
    std::string response;
    std::vector<int> sendTo;
    std::string username;
    std::string realname;
    std::istringstream userStream(userData);

    sendTo.push_back(fd);

    if (it == clients.end() || (it->second->isRegistered() && username != it->second->getUsername()))
    {
        if (it != clients.end())
        {
            response = ":" + epollServer->getServerHostname() + " 462 * " + it->second->getNickname() + " :You may not reregister";
        }
        else
        {
            response = ":" + epollServer->getServerHostname() + " 462 * :You may not reregister";
        }
        sendResponse(sendTo, response);
        return 0;
    }
    userStream >> username;
    std::getline(userStream, realname);
    std::getline(userStream, realname);

    it->second->setUsername(username);
    it->second->setRealname(realname);
    it->second->setHostname(epollServer->getHostname(fd));
    it->second->setRegistered(true);

    response = ":" + epollServer->getServerHostname() + " 001 " +  it->second->getNickname() + " :Welcome to the Internet Relay Network "+  it->second->getNickname() + "!" + username + "@" + it->second->getHostname();
    sendResponse(sendTo, response);
    return 0;
}

int Controller::quitHandler(int fd, std::string quitMessage)
{
    std::map<int, Client *>::iterator it = clients.find(fd);

    std::cout << quitMessage << std::endl;

    if (it != clients.end())
    {
        std::cout << "Closed by server" << std::endl;
        epollServer->closeConnection(fd);
    }
    return 0;
}

void Controller::initParseHandlers()
{
    parseHandlersMap["PASS"] = &Controller::authHandler;
    parseHandlersMap["NICK"] = &Controller::nickHandler;
    parseHandlersMap["USER"] = &Controller::userHandler;
    parseHandlersMap["QUIT"] = &Controller::quitHandler;
}

void Controller::saveBuffer(int fd, std::string buffer)
{
    fdBuffers[fd] += buffer;
}

std::string Controller::loadBuffer(int fd)
{
    std::map<int, std::string>::iterator it = fdBuffers.find(fd);
    if (it != fdBuffers.end())
    {
        std::string buffer = it->second;
        fdBuffers.erase(it);
        return buffer;
    }
    return "";
}

std::string Controller::escapeVisible(const std::string& input)
{
    std::string result;
    for (std::string::const_iterator it = input.begin(); it != input.end(); ++it) {
        switch (*it) {
            case '\r': result += "\\r"; break;
            case '\n': result += "\\n"; break;
            case '\t': result += "\\t"; break;
            case '\b': result += "\\b"; break;
            case '\\': result += "\\\\"; break;
            default: result += *it; break;
        }
    }
    return result;
}

void Controller::removeClients()
{
    std::map<std::string, Client *>::iterator it;

    for (it = clients_by_nick.begin(); it != clients_by_nick.end(); ++it)
    {
        delete it->second;
    }
}

void Controller::removeChannels()
{
    std::map<std::string, Channel *>::iterator it;

    for (it = channels.begin(); it != clients_by_nick.end(); ++it)
    {
        delete it->second;
    }
}

void Controller::clearContainers()
{
    clients_by_nick.clear();
    clients.clear();
    channels.clear();
    parseHandlersMap.clear();
    fdBuffers.clear();
}