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
    std::istringstream stream(message);
    std::string line;

    while (std::getline(stream, line))
    {
        std::cout << "Line: " << line << std::endl;
        // Now split the line into words
        std::istringstream lineStream(line);
        std::string value;
        std::string CMD;
        lineStream >> CMD;
        if (parseHandler(client_fd, CMD, message) == -1)
            break;
    }

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

int Controller::parseHandler(int fd, const std::string& CMD, std::string value)
{
    std::map<std::string, int (Controller::*)(int, std::string)>::iterator it = parseHandlersMap.find(CMD);
    if (it != parseHandlersMap.end())
        return (this->*(it->second))(fd, value);
    return 1;
}

int Controller::authHandler(int fd, std::string password)
{
    std::istringstream stream(password);
    std::string line;
    std::string response;
    std::vector<int> sendTo;
    std::vector<int> invalids;

    sendTo.push_back(fd);

    std::getline(stream, line);
    std::istringstream lineStream(line);
    std::string value;
    std::string CMD;
    lineStream >> CMD;
    lineStream >> value;

    std::cout << "valor" << value << std::endl;
    if (value != config.getPassword() && CMD == "PASS")
    {
        response = ":localhost 464 * :Password incorrect\r\n";
        invalids = epollServer->sendMessage(sendTo, response);
        return -1;
    }
    std::getline(stream, line);
    std::string nickname;
    lineStream >> CMD;
    lineStream >> nickname;
    std::getline(stream, line);
    std::string username;
    lineStream >> CMD;
    lineStream >> username;
    Client *newClient = new Client(fd, nickname);
    newClient->setUsername(username);
    newClient->setHostname(epollServer->getHostname(fd));
    response = ":<server> 001 <nick> :Welcome to the Internet Relay Network "+ nickname + "!" + username + "@" + newClient->getHostname() + "\r\n";
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