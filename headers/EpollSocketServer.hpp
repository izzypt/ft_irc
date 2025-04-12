#ifndef EPOLL_SOCKET_SERVER_HPP
#define EPOLL_SOCKET_SERVER_HPP

#include <iostream>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <string>
#include <vector>
#include <set>
#include "Config.hpp"
#include "Log.hpp"
#include "Controller.hpp"

#define BUFFER_SIZE 500

class Controller;

class EpollSocketServer
{
	public:
    	EpollSocketServer(Config &new_config, Log &new_log);
		~EpollSocketServer();

		void setController(Controller *new_controller);
    	void startServer();
    	void stopServer();
		std::vector<int> sendMessage(std::vector<int> clientsFileDescriptors, std::string message);
		std::string getHostname(int fd);
	private:
    	int serverFd;
    	int epollFd;
    	struct sockaddr_in serverAddr;
    	unsigned long int connectionsNumber;
		struct epoll_event *events;
		std::set<int> connections;
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
		int addConnection(int fd);
		void closeConnection(int fd);
		void closeAllConnections();
		std::string getHostnameFromIp(const std::string& ip);

		EpollSocketServer();
		EpollSocketServer(const EpollSocketServer& other);
    	EpollSocketServer& operator=(const EpollSocketServer& other);
};

#endif
