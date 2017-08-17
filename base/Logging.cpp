#include "Logging.h"
#include "glog/logging.h"

LogStreamLevel Logging::m_logLevel = LSL_ANY;
bool Logging::InitLogging(const char* appName,const  char* log_path)
 {

        FileSystemCtrl::CreateMutiDir(log_path);
        if(strlen(appName) + strlen(log_path) > 512 -1)
        {
            printf("InitLogging arg error! can not log!");
            return false;
        }
        char path_name[512];
        memset(path_name,0,512);
        strcat(path_name,log_path);
        strcat(path_name,"/");
        strcat(path_name,appName);

        google::InitGoogleLogging(appName);
        google::SetLogDestination(google::GLOG_INFO,path_name);
        FLAGS_alsologtostderr = true;
	return true;
 }

void Logging::ShutdownLogging()
{ 
   google::ShutdownGoogleLogging();
}

void Logging::SetLogLevel(LogStreamLevel lv){
	m_logLevel = lv;
}

LogStreamLevel Logging:: GetLogLevel(){
    return m_logLevel;
}

LogStream::LogStream(){

}

LogStream::~LogStream(){
	//log impl
    LOG(INFO)<< m_stream.str();
}


