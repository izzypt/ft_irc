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
        std::set<std::string> clientChannels = it->second->getClientChannels();
        std::set<std::string>::iterator itc;
        for (itc = clientChannels.begin(); itc != clientChannels.end(); ++itc)
            channels[*itc]->removeClient(client_fd);
        existentNicks.erase(it->second->getNickname());
        it->second->clearData();
        delete it->second;
        clients.erase(it);
    }
}

std::set<int> Controller::broadcastTo(int client_fd)
{
    std::map<int, Client *>::iterator it = clients.find(client_fd);
    std::set<int> sendTo;

    if (it == clients.end())
    {
        return sendTo;
    }
    std::set<std::string> clientChannels = it->second->getClientChannels();
    std::set<std::string>::iterator itc;
    for (itc = clientChannels.begin(); itc != clientChannels.end(); ++itc)
    {
        std::set<int> users = channels[*itc]->getChannelClients();
        for (std::set<int>::iterator itu = users.begin(); itu != users.end(); ++itu)
            sendTo.insert(*itu);
    }
    sendTo.erase(client_fd);

    return sendTo;
}

void Controller::sendResponse(std::set<int> sendTo, std::string response)
{
    std::set<int> invalids;
    std::set<int>::iterator it;

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
        std::set<int> sendTo;

        sendTo.insert(fd);
        sendResponse(sendTo, ":localhost 464 * :Password incorrect");
        return 1;
    }
    return 0;
}

int Controller::nickHandler(int fd, std::string nickname)
{
    std::map<int, Client *>::iterator it = clients.find(fd);
    std::string response;
    std::pair<std::set<std::string>::iterator, bool> result = existentNicks.insert(nickname);

    if (it != clients.end())
    {
        if (result.second == true)
        {
            std::string oldNick = it->second->getNickname();
            it->second->setNickname(nickname);

            existentNicks.erase(oldNick);     
    
            std::set<int> sendTo = broadcastTo(fd);
            response = ":" + oldNick + "!" + it->second->getUsername() + "@" + it->second->getHostname() + " NICK :" + nickname;
            sendResponse(sendTo, response);
        }
        else
        {
            std::set<int> sendTo;

            sendTo.insert(fd);
            response = ":" + epollServer->getServerHostname() + " 433 * " + nickname + " :Nickname is already in use";
            sendResponse(sendTo, response);
        }
        return 0;
    }
    if (result.second == false)
    {
        std::set<int> sendTo;

        sendTo.insert(fd);
        response = ":" + epollServer->getServerHostname() + " 433 * " + nickname + " :Nickname is already in use";
        sendResponse(sendTo, response);
        return 1;
    }
    Client *newClient = new Client(fd, nickname);
    clients.insert(std::pair<int, Client *>(fd, newClient));
    return 0;
}

int Controller::userHandler(int fd, std::string userData)
{
    std::map<int, Client *>::iterator it = clients.find(fd);
    std::string response;
    std::set<int> sendTo;
    std::string username;
    std::string realname;
    std::istringstream userStream(userData);

    sendTo.insert(fd);

    if (it == clients.end())
        return 0;
    if (it->second->isRegistered())
    {
        response = ":" + epollServer->getServerHostname() + " 462 * " + it->second->getNickname() + " :You may not reregister";
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
    std::string response;
    std::set<int> sendTo;

    std::cout << quitMessage << std::endl;

    if (it == clients.end())
        return 0;
    sendTo = broadcastTo(fd);
    response = ":" +  it->second->getNickname() + "!" + it->second->getUsername() + "@" + it->second->getHostname() + " QUIT " + quitMessage;
    epollServer->closeConnection(fd);
    sendResponse(sendTo, response);
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
    std::map<int, Client *>::iterator it;

    for (it = clients.begin(); it != clients.end(); ++it)
    {
        it->second->clearData();
        delete it->second;
    }
}

void Controller::removeChannels()
{
    std::map<std::string, Channel *>::iterator it;

    for (it = channels.begin(); it != channels.end(); ++it)
    {
        it->second->clearData();
        delete it->second;
    }
}

void Controller::clearContainers()
{
    existentNicks.clear();
    clients.clear();
    channels.clear();
    parseHandlersMap.clear();
    fdBuffers.clear();
}