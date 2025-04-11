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
#include <vector>
#include "Config.hpp"
#include "Log.hpp"
#include "Controller.hpp"

#define BUFFER_SIZE 500

class Controller;

class EpollSocketServer
{
	public:
    	EpollSocketServer(Config &new_config, Log &new_log);
    	EpollSocketServer(const EpollSocketServer& other);
    	EpollSocketServer& operator=(const EpollSocketServer& other);
		~EpollSocketServer();

		void setController(Controller *new_controller);
    	void startServer();
    	void stopServer();
		std::vector<int> sendMessage(std::vector<int> clientsFileDescriptors, std::string message);
	private:
    	int serverFd;
    	int epollFd;
    	struct sockaddr_in serverAddr;
    	unsigned long int connectionsNumber;
    	struct epoll_event *events;
		Config &config;
		Log &log;
		Controller *controller;

    	int openSocket();
    	void listenForConnections();
    	void setNonBlocking(int socketFd);
    	void processRequest(int socketFd);
    	int appendStringInBuffer(char *buffer, std::string& fullRequest);
    	void logErrorAndExit(const char *msg);
		void StoreClient(int clientFd, struct sockaddr_in clientAddr);
};

#endif
