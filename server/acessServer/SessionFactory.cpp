#include "SessionFactory.h"
#include "base/Logging.h"




int SessionFactory::OnMsgReceive(int type,int socketid,unsigned int ip,unsigned short port, void* data, int dataSize)
{
    if(type == TCPMSG_CONNECTED)
    {
        if(m_sessionMap.find(socketid)  ==m_sessionMap.end())
        {
            //如果是被动的连接，等待消息，再决定Session分类。

            return 0;
        }
        m_sessionMap[socketid]->OnSessionCreate();

    }
    else if(type ==TCPMSG_DISCONNECTED)
    {
        if(m_sessionMap.find(socketid)  == m_sessionMap.end()){
            //socket会自动销毁
            return 0;
        }
        m_sessionMap[socketid]->OnSessionDestory();
        m_sessionMap.erase(socketid);
        LOG_INFO << "断开Session,id ="<<socketid;
    }
    else if(type == TCPMSG_RECEIVED)
    {
        auto it = m_sessionMap.find(socketid) ;
        if(it == m_sessionMap.end())
        {
            //m_tcpManagercb(TCPMSG_DISCONNECT,-1,ip,port,NULL,0);


            LOG_ERROR<<"在没有session 的情况下收到消息";

        }
        //处理消息
        LOG_INFO <<"收到msg ,Session,id ="<<socketid;
    }



    return 0;
}

BaseSession* SessionFactory::CreateSession(int sessionType)
{
    switch(sessionType){
        case SESSIONTYPE_CLIENT:{
            break;
        }
        default:break;
    }
    return NULL;
}

int SessionFactory::DeteteSession(BaseSession* ins)
{
    return 0;
}

int SessionFactory::PreCreateSession()
{
    return 0;
}








