#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

class Config
{
    private:
        std::string logFolder;
        int listenPort;
        unsigned long int maxConnections;

        void readConfigFile(const std::string& pathToFile);

    public:
        Config();
        Config(const std::string& pathToFile);
        Config(const Config& other);
        Config& operator=(const Config& other);
        ~Config();

        std::string getLogFolder() const;
        void setLogFolder(const std::string& logFolder);

        int getListenPort() const;
        void setListenPort(int listenPort);

        unsigned long int getMaxConnections() const;
        void setMaxConnections(unsigned long int maxConnections);
};

#endif
