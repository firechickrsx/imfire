#ifndef BASESOCKET_H
#define BASESOCKET_H


enum BASETCPSOCKETSTATE{
    TCP_DISCONNECT,
    TCP_CONNECTING,
    TCP_CONNECTED,
    TCP_LISTENNING
};



class BaseTcpSocket
{
    public:
        BaseTcpSocket(){
            m_destlIp =0;
            m_destPort =0;
            m_socket = -1;
            m_state  = TCP_DISCONNECT;
        }
       ~BaseTcpSocket(){

       };


        //Connect 和 ListenAndWaitConnect 只能选择其中一个。

        //主动发起连接
        virtual int Connect(unsigned int ip , unsigned short port) =0;

        //j监听
        virtual int Listen(unsigned int local_ip, unsigned short local_port) =0;

        //j接受一个新连接
        virtual int Accept(int& newSocket )  =0;

        //j将本类的socket重置为一个已经有的，并且已经连接的socket
        virtual  void ResetClassToConnectedSocket(int newSocket) =0;


        //发送数据
        virtual int SendData(void* data,  int dataSize) =0;

        //收到数据通知
        virtual int ReceiveData(void* data, int dateSize) =0;

    protected:

    unsigned int m_destlIp;
    unsigned short m_destPort;

    int  m_socket;
    int m_state;
};

#endif // BASESOCKET_H
