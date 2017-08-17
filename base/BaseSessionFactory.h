#ifndef BASESESSIONFACTORY_H
#define BASESESSIONFACTORY_H
#include "Timer.h"
#include "BaseSession.h"
#include <map>
#include<memory>

class BaseSessionFactory
{
public:
    BaseSessionFactory()
    {
        m_timer  = NULL;
    }

    void SetTimerPtr(Timer* timer)
    {
        m_timer = timer ;
    }

    void SetTcpMsgSender(std::function<int(int, int,unsigned int,unsigned short,void*,int)>  cb)
    {
        m_tcpManagercb = cb;
    }

    virtual int OnMsgReceive(int type,int socketid,unsigned int ip,unsigned short port, void* data, int dataSize)
    {
        return -1;
    }

    virtual BaseSession* CreateSession(int sessionType)
    {
        return NULL;
    }

    virtual int DeteteSession(BaseSession* ins)
    {
        return -1;
    }

    virtual int PreCreateSession()
    {
        return -1;
    };

    function<int(int, int, unsigned int,unsigned short,void*,int)  > m_tcpManagercb;

    map<int,shared_ptr<BaseSession> > m_sessionMap;

    Timer* m_timer;
};
#endif
