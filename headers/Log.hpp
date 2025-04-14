#ifndef LOG_HPP
#define LOG_HPP

#include <string>
#include <fstream>
#include <ctime>

class Log
{
	public:
    	Log(const std::string& new_logFolder);
    	Log(const Log& other);
    	Log& operator=(const Log& other);
		~Log();

    	void entry(const std::string& type, const std::string& message);
		
	private:
    	std::string logFolder;
    
    	void setFile(const std::string& pathToLogFolder, const std::string& type);
    	void formatTime(char type, std::string& timeStr);
    	int countFileLines(const std::string& path);
};

#endif
