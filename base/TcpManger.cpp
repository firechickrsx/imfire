#include "TcpManager.h"
#include "ScopedGuard.h"
#include "Logging.h"

TcpManager::TcpManager()
{
    m_bRunning = false;
    m_epoll = -1;
}


TcpManager::~TcpManager()
{
}


int TcpManager::SetTcpMsgReceiver(std::function<void(int, int,unsigned int,unsigned short,void*,int)>  cb)
{

    m_msgcb  =cb;
    return 0;
}


int TcpManager::SendTcpMsg(int msgType,int socketid, unsigned int ip,unsigned short port,void* data,int dataSize)
{
    unique_lock<mutex> lock(m_socketMapMutex);
    shared_ptr<TcpSocket> target;

    auto it =  m_socketMap.begin();
    if(socketid != -1)
    {
        it = m_socketMap.find(socketid);
    }
    else
    {
        for(; it != m_socketMap.end(); it++)
        {
            if(it->second->m_destIp  ==ip && it ->second->m_destPort == port)
            {
                target = it->second;
                break;
            }
        }
    }



    if(msgType == TCPMSG_CONNECT)
    {
        if( it != m_socketMap.end() )
        {
            LOG_ERROR<<"TcpManager 申请的连接已存在";
            return -1;
        }

        shared_ptr<TcpSocket> newSocket(new TcpSocket);
        if( target->Connect(ip,port) ==  -1 )
        {
            return -1;
        }

        m_socketMap[newSocket->m_socket] = newSocket;
        epoll_event ev;
        ev.data.fd  =newSocket->m_socket;
        ev.events=EPOLLIN;

        if(m_epoll != -1 && epoll_ctl(m_epoll,EPOLL_CTL_ADD,newSocket->m_socket,&ev) <0)
        {
            LOG_ERROR<<"TcpManager 无法加入epoll";
            return -1 ;
        }
        return newSocket->m_socket;
    }
    else if(msgType == TCPMSG_DISCONNECT)
    {
        if(it == m_socketMap.end())
        {
            return -1;
        }
        CloseSocket(it->second->m_socket);
        return it->second->m_socket;
    }
    else if(msgType ==TCPMSG_SEND)
    {
        if(it == m_socketMap.end())
        {
            return -1;
        }
        int send_count =  it->second ->SendData(data,dataSize);
        if(send_count == 0 ) //发送缓冲区不够用了
        {
            //注册一个发送监听
            epoll_event ev;
            ev.data.fd  =it->second->m_socket;
            ev.events=EPOLLIN |EPOLLOUT;
            if(epoll_ctl(m_epoll,EPOLL_CTL_MOD,it->second->m_socket,&ev) == -1)
            {
                LOG_INFO<<"epoll_ctl error in  EPOLL_CTL_MOD";
                return -1;
            }
        }
        return it->second->m_socket;
    }


    return -1;
}

int TcpManager::RunProcess(int handleSocketCount)
{
    LOG_INFO  <<"TcpManager:: RunProcess start! thread id = "<<  this_thread::get_id();
    int ret = 0;

    epoll_event* event_buf =NULL;
    event_buf  = new epoll_event[handleSocketCount];
    if(event_buf == NULL)
    {
        return -1;
    }



    epoll_event listen_ev;
    listen_ev.data.fd  =m_listenSocket.m_socket;
    listen_ev.events=EPOLLIN;

    if(epoll_ctl(m_epoll,EPOLL_CTL_ADD,m_listenSocket.m_socket,&listen_ev) <0)
    {
        LOG_INFO<<"epoll_error";
        return ret = -1;
    }

    int event_count = 0;
    m_bRunning = true;
    while(m_bRunning)
    {
        event_count  =epoll_wait(m_epoll,event_buf,handleSocketCount, 500);
        if(event_count >0)
        {
            for(int i = 0 ; i<event_count ; i++)
            {
                if( event_buf[i].data.fd  == m_listenSocket.m_socket)
                {
                    HandleAccept();
                }
                else if(event_buf[i].events&EPOLLIN)
                {
                    HandleRead(event_buf[i].data.fd);
                }
                else if(event_buf[i].events&EPOLLOUT)
                {
                    HandleWrite(event_buf[i].data.fd);
                }
                else
                {
                    LOG_INFO<< "epoll wait ";
                    CloseSocket(event_buf[i].data.fd);
                    //错误或者对方关闭
                }
            }

        }
        else if(event_count == 0) //超时
        {

            //LOG_EVERY_N(INFO, 10) << "epoll timteout";
            //超时
        }
        else  //error
        {
            if(errno == EINTR){
                continue;
            }else
            {
                 LOG_INFO<< "epoll erorr errno="<<errno;
            }

        }
    }//while

    LOG_INFO  <<"TcpManager:: RunProcess end! thread id = "<<  this_thread::get_id();
    return 0;
}

int TcpManager::StartProcess(unsigned int local_ip, unsigned short local_port,int handleSocketCount)
{
    if(m_bRunning ==true)
    {
        return -1;
    }

    if (m_listenSocket.Listen(local_ip, local_port) != 0 )
    {
        return  -1;
    }

    m_epoll = epoll_create(handleSocketCount);//Linux 2.6.8 或者之后，该传入参数被忽略
    if(m_epoll == -1)
    {
        return  -1;
    }

    m_processThread = std::thread(bind(&TcpManager::RunProcess,this,handleSocketCount));

    return 0;
}

void TcpManager::HandleAccept()
{
    unique_lock<mutex> lock(m_socketMapMutex);
    int new_fd = -1;
    unsigned int remoteIp = 0;
    unsigned short remotePort = 0;
    while(m_listenSocket.Accept(new_fd, remoteIp, remotePort) == 0 )
    {
        shared_ptr<TcpSocket> shSocket(new TcpSocket());
        shSocket->ResetClassToConnectedSocket(new_fd);
        shSocket->m_destIp = remoteIp;
        shSocket->m_destPort = remotePort;
        m_socketMap[new_fd]  = shSocket;
        m_msgcb( TCPMSG_CONNECTED, new_fd,  remoteIp,remotePort,NULL,0 );
    }
}
void TcpManager::HandleRead(int read_fd)
{
    unique_lock<mutex> lock(m_socketMapMutex);
    if(m_socketMap.find(read_fd)  == m_socketMap.end())
    {
        return;
    }
    shared_ptr<TcpSocket> target  = m_socketMap[read_fd];

    char buf[1024];
    memset(buf,0,1024);

    int receiveSize = 1;//初始条件
    while(receiveSize >0)
    {
        receiveSize= target->ReceiveData(buf,1024);
        if(receiveSize == 0)
        {
            CloseSocket(read_fd);
        }
        else if(receiveSize >0)
        {
            m_msgcb( TCPMSG_RECEIVED,  read_fd,target->m_destIp, target->m_destPort,buf,receiveSize );
        }
        else if(receiveSize == -1 &&errno ==EAGAIN)
        {
            return;
        }
        else
        {
            CloseSocket(read_fd);
        }

    }
}

void TcpManager::HandleWrite(int write_fd)
{
    unique_lock<mutex> lock(m_socketMapMutex);
    if(m_socketMap.find(write_fd)  == m_socketMap.end())
    {
        return;
    }
    shared_ptr<TcpSocket> target  = m_socketMap[write_fd];

    if(target->m_connectSucEvent ==true)
    {
        epoll_event ev;
        ev.data.fd  =m_listenSocket.m_socket;
        ev.events=EPOLLIN;
        int epoll_rs = epoll_ctl(m_epoll,EPOLL_CTL_MOD,write_fd,&ev);
        if(epoll_rs == -1)
        {
            LOG_ERROR<<"epoll error CTL_MOD";
            CloseSocket(write_fd);
        }
        m_msgcb( TCPMSG_CONNECTED,  write_fd,target->m_destIp, target->m_destPort, NULL,  0 );
        target->m_connectSucEvent = false;
    }

    //发送因为缓冲区满而没有发送的数据。返回的是还有多少个数据没发出去
    int next_time_send_buf_count = target->SendDataInBufferQueue();
    if(next_time_send_buf_count >0)
    {
        //do nothing
    }
    else if(next_time_send_buf_count == 0)
    {
        //缓冲区的数据发完了
        epoll_event ev;
        ev.data.fd  =m_listenSocket.m_socket;
        ev.events=EPOLLIN;
        int epoll_rs = epoll_ctl(m_epoll,EPOLL_CTL_MOD,write_fd,&ev);
        if(epoll_rs ==-1){
            LOG_ERROR<<"epoll error CTL_MOD";
        }
    }
    else
    {
        CloseSocket(write_fd);
    }
}

int TcpManager::StopProcess()
{
    m_bRunning =false;
    if(m_processThread.joinable())
    {
        m_processThread.join();
    }

    return 0;
}

void TcpManager::CloseSocket(int fd)
{
    // 注意，执行这个函数的前提是已经对m_socketMap保护。

    if(m_socketMap.find(fd)  == m_socketMap.end())
    {
        return;
    }
    shared_ptr<TcpSocket> target  = m_socketMap[fd];
    struct epoll_event ev;
    ev.data.fd=fd;
    ev.events=EPOLLIN|EPOLLET;
    if(epoll_ctl(m_epoll,EPOLL_CTL_DEL,fd,&ev) == -1)
    {
        LOG_INFO<<"TcpManager::CloseSocket epoll error";
    };
    SendTcpMsg(TCPMSG_DISCONNECTED,fd,target->m_destIp,target->m_destPort,NULL, 0);
    m_socketMap.erase(fd);
}






