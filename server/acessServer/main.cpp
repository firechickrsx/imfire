#include "Server.h"
#include "base/Logging.h"


int main(int argc, char* argv[]){

   Logging::InitLogging("Server" , "/home/firechick/桌面/imfire/log/Server");

    LOG_INFO<< "LoginServer Start !";

    Server Server;
    Server.Run();

    Logging::ShutdownLogging();
    LOG_INFO<< "LoginServer Stop and exit !";
    return 0;
}
