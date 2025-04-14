#include "../headers/EpollSocketServer.hpp"

EpollSocketServer::EpollSocketServer(Config &new_config, Log &new_log) : serverFd(-1), epollFd(-1), connectionsNumber(0), config(new_config) , log(new_log) , controller(NULL)
{
    events = new struct epoll_event[config.getMaxConnections() + 1];
}

EpollSocketServer::~EpollSocketServer()
{
    closeAllConnections();
    if (serverFd != -1)
        close(serverFd);
    delete[] events;
    if (epollFd != -1)
        close(epollFd);
    clearContainers();
}

void EpollSocketServer::setController(Controller *new_controller)
{
    controller = new_controller;
}

void EpollSocketServer::startServer()
{
    serverFd = openSocket();
    if (serverFd == -1)
    {
        logErrorAndExit("Failed to open server socket.");
    }
    listenForConnections();
}

void EpollSocketServer::stopServer()
{
    closeAllConnections();
    if (serverFd != -1)
    {
        close(serverFd);
        serverFd = -1;
    }
    delete[] events;
    if (epollFd != -1)
    {
        close(epollFd);
        epollFd = -1;
    }
    clearContainers();
}

std::vector<int> EpollSocketServer::sendMessage(std::vector<int> clientsFileDescriptors, std::string message)
{
    std::vector<int> invalids;
    std::vector<int>::iterator it;
    std::set<int>::iterator itc;

    for (it = clientsFileDescriptors.begin(); it != clientsFileDescriptors.end(); ++it)
    {
        itc = connections.find(*it);
        if (itc == connections.end() || write(*it, message.c_str(), message.size()) < 0)
            invalids.push_back(*it); 
    }
    return invalids;
}

std::string EpollSocketServer::getHostname(int fd)
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    if (getsockname(fd, (struct sockaddr*)&addr, &addr_len) == -1)
    {
        std::string errorMessage = std::string("Error getting socket name: ") + strerror(errno);
        log.entry("error", errorMessage);
        return "";
    }
    std::string ip = inet_ntoa(addr.sin_addr);
    
    return getHostnameFromIp(ip);
}

std::string EpollSocketServer::getServerHostname()
{
    return getHostname(serverFd);
}

int EpollSocketServer::openSocket()
{
    if (config.getListenPort() < 1)
    {
        log.entry("error", "Port to listen socket not supplied. Check configuration file.");
        return -1;
    }

    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0)
    {
        log.entry("error", "Error opening listening socket");
        return -1;
    }

    int opt = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        log.entry("error", "Unable to set socket to allow the reuse of local addresses.");

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(config.getListenPort());

    if (bind(serverFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        log.entry("error", "Binding not possible");
        return -1;
    }
    return serverFd;
}

void EpollSocketServer::listenForConnections()
{
    setNonBlocking(serverFd);
    listen(serverFd, 5);

    /* Creates an epoll instance and returns a file descriptor (`epollFd`) that represents it. */
    epollFd = epoll_create1(0);
    if (epollFd == -1)
    {
        log.entry("error", "Unable to create epoll fd");
        return;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = serverFd;

    /* Used to add, modify, or remove file descriptors from the `epoll` instance */
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &ev) == -1)
    {
        log.entry("error", "Unable to add listen socket fd to epoll");
        return;
    }

    log.entry("info", "Service ready to accept connections");
    
    while (true)
    {
        /* Waits for events on the file descriptors registered with the `epoll` instance. */
        int nfds = epoll_wait(epollFd, events, config.getMaxConnections() + 1, -1);
        if (nfds == -1)
        {
            log.entry("error", "epoll wait failed");
            return;
        }

        for (int n = 0; n < nfds; n++)
        {
            if (events[n].data.fd == serverFd)
            {
                struct sockaddr_in clientAddr;
                socklen_t clientLen = sizeof(clientAddr);
                int clientFd = accept(serverFd, (struct sockaddr *)&clientAddr, &clientLen);
                if (clientFd == -1)
                {
                    log.entry("error", "Unable to accept connection");
                    return;
                }

                if (connectionsNumber + 1 > config.getMaxConnections())
                {
                    close(clientFd);
                    log.entry("warning", "Unable to accept more connections");
                    continue;
                }

                if (addConnection(clientFd))
                    continue;

                std::string logMessage = "Opened new TCP connection from " + getHostname(clientFd);
                log.entry("info", logMessage);
                connectionsNumber++;
            }
            else if (events[n].events & (EPOLLRDHUP | EPOLLHUP))
            {
                closeConnection(events[n].data.fd);
                connectionsNumber--;
                log.entry("info", "Connection closed");
            }
            else
            {
                processRequest(events[n].data.fd);
            }
        }
    }
    close(serverFd);
}

void EpollSocketServer::setNonBlocking(int socketFd)
{
    if (fcntl(socketFd, F_SETFD, fcntl(socketFd, F_GETFD, 0) | O_NONBLOCK) == -1)
    {
        logErrorAndExit("Unable to set non-blocking mode");
    }
}

void EpollSocketServer::processRequest(int socketFd)
{
    char buffer[BUFFER_SIZE];
    std::string fullRequest;

    int n = BUFFER_SIZE - 1;
    while (n == BUFFER_SIZE - 1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        n = read(socketFd, buffer, BUFFER_SIZE - 1);
        appendStringInBuffer(buffer, fullRequest);
    }

    controller->processMessage(socketFd, fullRequest);
}

int EpollSocketServer::appendStringInBuffer(char *buffer, std::string& fullRequest)
{
    if (!buffer || strcmp(buffer, "") == 0)
    {
        log.entry("error", "Empty read buffer");
        return 1;
    }

    fullRequest.append(buffer);
    return 0;
}

void EpollSocketServer::logErrorAndExit(const char *msg)
{
    log.entry("error", msg);
    exit(1);
}

int EpollSocketServer::addConnection(int fd)
{
    std::set<int>::iterator it = connections.find(fd);
    if (it != connections.end())
    {
        log.entry("error", "Something went wrong! There is already a connection with this file descriptor");
        return 1;
    }
    setNonBlocking(fd);
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP;
    ev.data.fd = fd;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        close(fd);
        log.entry("error", "Unable to add client socket fd to epoll");
        return 1;
    }
    connections.insert(fd);
    return 0;
}

void EpollSocketServer::closeConnection(int fd)
{
    std::set<int>::iterator it = connections.find(fd);
    if (it == connections.end())
    {
        log.entry("warning", "There is no connection with this file descriptor");
        return;
    }
    epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    controller->connectionClosed(fd);
    connections.erase(it);
}

void EpollSocketServer::closeAllConnections()
{
    std::set<int>::iterator it = connections.begin();
    while (it != connections.end())
    {
        close(*it);
        controller->connectionClosed(*it);
        connections.erase(it);
        it = connections.begin();
    }
}

std::string EpollSocketServer::getHostnameFromIp(const std::string& ip)
{
    struct hostent *he = gethostbyname(ip.c_str());
    if (he == NULL)
    {
        std::string errorMessage = std::string("Error resolving hostname: ") + hstrerror(h_errno);
        log.entry("warning", errorMessage);
        return ip;
    }
    return std::string(he->h_name);
}

void EpollSocketServer::clearContainers()
{
    connections.clear();
}