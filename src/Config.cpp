#include "../headers/Config.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

Config::Config() : listenPort(0), maxConnections(0)
{
    readConfigFile("irc.conf");
}

Config::Config(const std::string& pathToFile) : listenPort(0), maxConnections(0)
{
    readConfigFile(pathToFile);
}

Config::Config(const Config& other) : logFolder(other.logFolder), Password(other.Password), listenPort(other.listenPort), maxConnections(other.maxConnections) {}

Config& Config::operator=(const Config& other)
{
    if (this != &other) {
        logFolder = other.logFolder;
        listenPort = other.listenPort;
        Password = other.Password;
        maxConnections = other.maxConnections;
    }
    return *this;
}

Config::~Config() {}

void Config::readConfigFile(const std::string& pathToFile)
{
    std::ifstream file(pathToFile.c_str());
    if (!file.is_open())
	{
        std::cout << "Unable to open config file: " << pathToFile << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line))
	{
        std::istringstream iss(line);
        std::string key;
        if (std::getline(iss, key, '='))
		{
            std::string value;
            if (std::getline(iss, value))
			{
                if (key == "LogFolder")
				{
                    logFolder = value;
                }
				else if (key == "ListenPort")
				{
                    std::istringstream(value) >> listenPort;
                }
				else if (key == "MaxConnections")
				{
                    std::istringstream(value) >> maxConnections;
                }
            }
        }
    }

    file.close();
}

std::string Config::getLogFolder() const
{
    return logFolder;
}

std::string Config::getPassword() const
{
    return Password;
}

int Config::getListenPort() const
{
    return listenPort;
}

unsigned long int Config::getMaxConnections() const
{
    return maxConnections;
}

void Config::setLogFolder(const std::string& logFolder)
{
    this->logFolder = logFolder;
}

void Config::setPassword(const std::string& Password)
{
    this->Password = Password;
}

void Config::setListenPort(int listenPort)
{
    this->listenPort = listenPort;
}

void Config::setMaxConnections(unsigned long int maxConnections)
{
    this->maxConnections = maxConnections;
}
