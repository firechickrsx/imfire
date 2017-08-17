#include "TcpSocket.h"
#include "Logging.h"
#include <fcntl.h>



TcpSocket::TcpSocket()
{
    m_socket = -1;
    m_destIp = 0;
    m_destPort = 0;
    m_connectSucEvent  = true;
}

TcpSocket::~TcpSocket()
{
    Disconnect();
}


int TcpSocket::Connect(unsigned int ip, unsigned short port)
{
    if(m_socket != -1) //已经连接了
    {
        LOG_INFO<<"TcpSocket::Connect()== -1 Repeat Connect";
        return -1;
    }

    m_socket=socket(AF_INET,SOCK_STREAM,0);

    if(m_socket ==  -1)
    {
        LOG_INFO<<"TcpSocket::Connect()== -1 socket() fail";
        return -1;
    }

    if (fcntl(m_socket, F_SETFL, fcntl(m_socket, F_GETFD, 0)|O_NONBLOCK) == -1)
    {
        LOG_INFO<<"TcpSocket::Connect()== -1 fcntl() fail";
        return -1;
    }


    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_addr.s_addr=ip;
    addr.sin_port=port;
    addr.sin_family =AF_INET;

    int ret = connect(m_socket,(struct sockaddr*)&addr,sizeof(addr));

    if( ret == -1 && errno ==EINPROGRESS)
    {
        return 0;
    }
    else if(ret == 0 )
    {
        return 0;
    }
    else
    {
        LOG_INFO<<"TcpSocket::Connect()== -1 connect fail";
        return -1;
    }
}

int TcpSocket::Listen(unsigned int localIP,unsigned short local_port)
{
    if(m_socket != -1)
    {
        return -1;
    }

    m_socket=socket(AF_INET,SOCK_STREAM,0);
    if(m_socket ==  -1)
    {
        return -1;
    }

    if (fcntl(m_socket, F_SETFL, fcntl(m_socket, F_GETFD, 0)|O_NONBLOCK) == -1)
    {
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_addr.s_addr = localIP;
    addr.sin_port = local_port;
    addr.sin_family =AF_INET;


    int ret = bind(m_socket,(struct sockaddr*)&addr,sizeof(addr));
    if(ret == -1)
    {
        return -1;
    }

    ret = listen(m_socket,1000);
    if(ret == -1)
    {
        return -1;
    }

    return ret;
}

int TcpSocket::Accept(int& newSocket ,unsigned int& remoteIp,unsigned short& remotePort)
{
    if(m_socket == -1)
    {
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    socklen_t len =sizeof(addr);
    int new_Socket = accept(m_socket,(struct sockaddr*)&addr,&len);
    if(new_Socket  == -1 )
    {
        return -1;
    }

    newSocket =new_Socket;
    remoteIp = addr.sin_addr.s_addr;
    remotePort = addr.sin_port;
    return 0;
}


int TcpSocket::Disconnect()
{

    if(m_socket != -1)
    {
        close(m_socket);
    }
    return 0;
}


void TcpSocket::ResetClassToConnectedSocket(int newSocket)
{
    Disconnect();
    if(newSocket == -1)
    {
        return ;//error
    }

    m_socket  =newSocket;
}



int TcpSocket::SendData(void* data,  int dataSize)
{
    if(m_socket == -1)
    {
        return -1;
    }

    int send_count = send(m_socket,data,dataSize,0);
    if(send_count == -1 &&errno == EAGAIN)
    {
        shared_ptr<struct TCPBUFSTRUCT>  buf (new struct TCPBUFSTRUCT(dataSize));
        m_sendBufQueue.push(buf);
        return 0;
    }
    else if(send_count>0)
    {
        if(dataSize != send_count)
        {
            LOG_INFO<<"SendData need check";
        }
        return send_count;
    }
    else
    {
        return -1;
    }

}

int TcpSocket::SendDataInBufferQueue()
{
    if(m_socket == -1)
    {
        return -1;
    }

    while(m_sendBufQueue.size() !=0)
    {
        shared_ptr<struct TCPBUFSTRUCT>  buf =m_sendBufQueue.front();
        int send_count = send(m_socket,buf->m_data,buf->m_dataSize,0);
        if(send_count == -1 &&errno == EAGAIN)//发送缓冲区满
        {
            break;
        }
        else if( send_count == -1 )//错误
        {
            return -1;
        }
        m_sendBufQueue.pop();
    }
    return m_sendBufQueue.size();
}



int TcpSocket::ReceiveData(void* buf, int bufSize)
{
    if(m_socket == -1)
    {
        return -1;
    }

    return recv(m_socket,buf,bufSize,0);
}

int TcpSocket::Reset()
{
    if(m_socket)
    {
        close(m_socket);
    }
    m_socket = -1;
    m_destIp = 0;
    m_destPort = 0;

    return 0;
}

