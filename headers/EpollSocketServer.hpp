#ifndef EPOLL_SOCKET_SERVER_HPP
#define EPOLL_SOCKET_SERVER_HPP

#include <iostream>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <string>
#include "Config.hpp"
#include "Log.hpp"
#include <map>
#include "Client.hpp"

#define BUFFER_SIZE 500

class EpollSocketServer
{
	public:
    	EpollSocketServer(Config &new_config, Log &new_log);
    	~EpollSocketServer();

    	EpollSocketServer(const EpollSocketServer& other);
    	EpollSocketServer& operator=(const EpollSocketServer& other);

    	void startServer();
    	void stopServer();

	private:
    	int serverFd;
    	int epollFd;
    	struct sockaddr_in serverAddr;
    	unsigned long int connectionsNumber;
    	struct epoll_event *events;
		Config &config;
		Log &log;
		std::map<int, Client *> clients;

    	int openSocket();
    	void listenForConnections();
    	void setNonBlocking(int socketFd);
    	int processRequest(int socketFd);
    	int appendStringInBuffer(char *buffer, std::string& fullRequest);
    	void logErrorAndExit(const char *msg);
		void StoreClient(int clientFd, struct sockaddr_in clientAddr);
};

#endif
