#ifndef LOGGING_H_H_H_A
#define LOGGING_H_H_H_A

#include <stdio.h>
#include <sstream>
#include "FileSystemCtrl.h"

#define LOG_ERROR if(LSL_ERROR <= Logging::GetLogLevel())LogStream().m_stream
#define LOG_INFO if(LSL_INFO <= Logging::GetLogLevel())LogStream().m_stream
#define LOG_DEBUG if(LSL_DEBUG <= Logging::GetLogLevel())LogStream().m_stream
#define LOG_ANY if(LSL_ANY <= Logging::GetLogLevel())LogStream().m_stream

enum LogStreamLevel{ 
	LSL_ERROR=0,	
	LSL_INFO = 1,
	LSL_DEBUG =2,
	LSL_ANY =3,
};


class Logging
{
public:
    static bool InitLogging(const char* appName,const  char* log_path);
    static void ShutdownLogging();
    static void SetLogLevel(LogStreamLevel lv);
    static LogStreamLevel GetLogLevel();
private:	   
    static LogStreamLevel m_logLevel;

};

class LogStream
{
public:
	LogStream();
	~LogStream();
public:
std::stringstream m_stream;	
};


















#endif //LOGGING_H_H_H_A
