#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <memory>
#include <queue>
#include <memory.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

using namespace std;

struct TCPBUFSTRUCT{
public:
    TCPBUFSTRUCT(int createBufSize){
        if(createBufSize  ==0){
            m_data =NULL;
        }else{
            m_data = new char[createBufSize];
        }
        m_dataSize = 0 ;
    };
    ~TCPBUFSTRUCT(){
        if(m_data){
            delete[] m_data;
        }
    }
    char* m_data;
    int m_dataSize;
};



enum BASETCPSOCKETSTATE{
    TCP_DISCONNECT,
    TCP_CONNECTING,
    TCP_CONNECTED,
    TCP_LISTENNING
};

class TcpSocket
{
public:
    TcpSocket();
    ~TcpSocket();

    virtual int Connect(unsigned int ip , unsigned short port) ;
    virtual int Listen(unsigned int ip,unsigned short port) ;
    virtual int Accept(int& newSocket ,unsigned int& remoteIp,unsigned short& remotePort);
    virtual  void ResetClassToConnectedSocket(int newSocket);
    virtual int Disconnect();
    virtual int SendData(void* data,  int dataSize) ;
    virtual int SendDataInBufferQueue();
    virtual int ReceiveData(void* buf, int bufSize) ;
    virtual int Reset();

    unsigned int m_destIp;
    unsigned short m_destPort;

    queue< shared_ptr<struct TCPBUFSTRUCT> >  m_sendBufQueue;

    bool m_connectSucEvent;
    int  m_socket;
};

#endif // TCPSOCKET_H
