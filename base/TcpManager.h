
#ifndef TCPMANAGER_
#define TCPMANAGER_

#include "BaseDefine.h"
#include "TcpSocket.h"
#include <sys/epoll.h>

#include <mutex>
#include <memory>
#include <map>
#include <functional>
#include <thread>
#include <condition_variable>
#include <chrono>

using namespace std;



class TcpManager{
public:
    TcpManager();
    ~TcpManager();

    //tcp消息的回调      参数：消息类型,socketid，ip，port，数据，数据大小
    int SetTcpMsgReceiver(std::function<void(int, int, unsigned int ,unsigned short , void* ,int)>  cb);

    //socketid和ip/port都可以用于指定目标。 如果创建新连接，socketid 必须是-1   。其他情况下，可以选择指定socketid或者ip+port
    int SendTcpMsg(int msgType,int ,unsigned int ip,unsigned short port,void* data = NULL ,int dataSize = 0 )  ;

    int StartProcess(unsigned int local_ip, unsigned short local_port,int handleSocketCount);
    int StopProcess();
private:
    int RunProcess(int handleSocketCount);

    void CloseSocket(int fd);

    void HandleAccept();
    void HandleRead(int read_fd);
    void HandleWrite(int write_fd);


    mutex m_socketMapMutex;
    map< int ,shared_ptr<TcpSocket> >  m_socketMap;

    function<void(int, int ,unsigned int ,unsigned short,void* ,int)  > m_msgcb;

    TcpSocket m_listenSocket;
    bool m_bRunning;

    thread m_processThread;


    int m_epoll;

};


#endif //TCPMANAGER_
