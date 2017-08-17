#ifndef SESSION_H
#define SESSION_H
#include "BaseDefine.h"

#include <stdio.h>

#include <memory>
#include <mutex>
using namespace std;


class BaseSessionFactory;


struct SESSIONMSGSTRUCT
{
public:
    SESSIONMSGSTRUCT(int bufSize)
    {
        m_ip = 0;
        m_port =0;
        m_msgType = 0;
        m_socketid = -1;
        if(bufSize >0)
        {
            m_data  = new char[bufSize];
        }
        else
        {
            m_data =NULL;
        }
        m_dataSize  =0;
    }
    ~SESSIONMSGSTRUCT()
    {
        if(m_data)
        {
            delete[] m_data;
        }
    }
    int m_msgType;
    int m_socketid;
    char* m_data;
    int m_dataSize;
    unsigned int m_ip;
    unsigned short m_port;
};



class BaseSession
{
public:
    BaseSession(BaseSessionFactory* factory){
        m_factory =factory;
        m_sessionType  =SESSIONTYPE_NULL;
        m_destIp = 0;
        m_destPort =0;

    }
    ~BaseSession(){}


    virtual void OnSessionCreate() = 0 ;
    virtual void OnSessionDestory() = 0;
    virtual void OnMsgReceive() = 0;
    virtual void OnTimer() = 0;

public:
    eSessionType m_sessionType;
    unsigned int m_destIp;
    unsigned short m_destPort;

    BaseSessionFactory* m_factory;

};

#endif // SESSION_H
