#include "EpollSocketServer.hpp"

EpollSocketServer::EpollSocketServer(Config &new_config, Log &new_log) : serverFd(-1), epollFd(-1), connectionsNumber(0), events(NULL) , config(new_config) , log(new_log)
{
    events = new struct epoll_event[config.getMaxConnections() + 1];
}

EpollSocketServer::~EpollSocketServer()
{
    delete[] events;
}

EpollSocketServer::EpollSocketServer(const EpollSocketServer& other) : serverFd(other.serverFd), epollFd(other.epollFd), connectionsNumber(other.connectionsNumber), config(other.config) , log(other.log)
{
    // Allocate and copy the events array
    events = new struct epoll_event[config.getMaxConnections() + 1];
    memcpy(events, other.events, sizeof(struct epoll_event) * (config.getMaxConnections() + 1));
}

EpollSocketServer& EpollSocketServer::operator=(const EpollSocketServer& other)
{
    if (this != &other) // Self-assignment check
    {
        serverFd = other.serverFd;
        epollFd = other.epollFd;
        connectionsNumber = other.connectionsNumber;
		config = other.config;
		log = other.log;

        // Allocate and copy the events array
        delete[] events;
        events = new struct epoll_event[config.getMaxConnections() + 1];
        memcpy(events, other.events, sizeof(struct epoll_event) * (config.getMaxConnections() + 1));
    }

    return *this;
}

void EpollSocketServer::startServer()
{
    serverFd = openSocket();  // Open the server socket
    if (serverFd == -1)
    {
        logErrorAndExit("Failed to open server socket");
    }

    listenForConnections();  // Listen for incoming connections
}

void EpollSocketServer::stopServer()
{
    if (serverFd != -1)
    {
        close(serverFd);  // Close the server socket
        serverFd = -1;
    }
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

    epollFd = epoll_create1(0);
    if (epollFd == -1)
    {
        log.entry("error", "Unable to create epoll fd");
        return;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = serverFd;

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &ev) == -1)
    {
        log.entry("error", "Unable to add listen socket fd to epoll");
        return;
    }

    log.entry("info", "Service ready to accept connections");

    while (true)
    {
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

                log.entry("info", "New connection opened");

                setNonBlocking(clientFd);
                ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP;
                ev.data.fd = clientFd;

                if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &ev) == -1)
                {
                    log.entry("error", "Unable to add client socket fd to epoll");
                    return;
                }

                connectionsNumber++;
            }
            else if (events[n].events & (EPOLLRDHUP | EPOLLHUP))
            {
                epoll_ctl(epollFd, EPOLL_CTL_DEL, events[n].data.fd, NULL);
                close(events[n].data.fd);
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

int EpollSocketServer::processRequest(int socketFd)
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

    if (fullRequest.empty())
    {
        write(socketFd, "Unable to get request", 21);
        return -1;
    }

    n = write(socketFd, fullRequest.c_str(), fullRequest.size());

    if (n <= 0)
        return -1;

    return 0;
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
