#include "Log.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sstream>

Log::Log(const std::string& logFolder) : logFolder(logFolder) {}

Log::~Log() {}

Log::Log(const Log& other) : logFolder(other.logFolder) {}

Log& Log::operator=(const Log& other)
{
    if (this != &other)
	{
        logFolder = other.logFolder;
    }
    return *this;
}

void Log::entry(const std::string& type, const std::string& message)
{
    std::ofstream file;
    std::string path = logFolder + "/" + type + ".log";

	file.open(path.c_str(), std::ios::in);
    std::string timeString;
    formatTime('p', timeString);

    file << timeString << " : " << message << std::endl;
	file.close();
}

void Log::setFile(const std::string& pathToLogFolder, const std::string& type)
{
    std::ofstream file;
    std::string path = pathToLogFolder + "/" + type + ".log";

    file.open(path.c_str(), std::ios::in);
    if (file.is_open())
	{
        int lineCount = countFileLines(path);
        file.close();
        if (lineCount > 999)
		{
            std::string oldPath = pathToLogFolder + "/" + type;
            std::string timeString;
            formatTime('a', timeString);
            oldPath += timeString + ".log";
            std::rename(path.c_str(), oldPath.c_str());
        }
    }
}

void Log::formatTime(char type, std::string& timeStr)
{
    time_t now = time(0);
    struct tm* nowInfo = localtime(&now);

    std::ostringstream os;
    os << nowInfo->tm_year + 1900
       << (nowInfo->tm_mon + 1 < 10 ? "0" : "") << nowInfo->tm_mon + 1
       << (nowInfo->tm_mday < 10 ? "0" : "") << nowInfo->tm_mday
       << (nowInfo->tm_hour < 10 ? "0" : "") << nowInfo->tm_hour
       << (nowInfo->tm_min < 10 ? "0" : "") << nowInfo->tm_min
       << (nowInfo->tm_sec < 10 ? "0" : "") << nowInfo->tm_sec;

    // Insert space between date and time if type is 'p'
    if (type == 'p') {
        timeStr = os.str();
        timeStr.insert(10, " ");  // Insert space after date (at position 10)
    } else if (type == 'a') {
        timeStr = os.str();
    }
}

int Log::countFileLines(const std::string& path)
{
    std::ifstream file(path.c_str());
    int lineCount = 0;
    std::string line;

    if (!file.is_open())
        return 0;

    while (std::getline(file, line))
        ++lineCount;

    file.close();
    return lineCount;
}
