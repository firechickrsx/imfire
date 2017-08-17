#include "MsgLooper.h"
#include "BaseDefine.h"
#include "Logging.h"
#include "ServerSetting.h"

#include <string.h>
#include <unistd.h>

MsgLooper::MsgLooper()
{
    m_bStopProcess =false;
}

MsgLooper::~MsgLooper()
{

}




void MsgLooper::PushMsgToLoop(int type,int socketid,unsigned int ip,unsigned short port,void* data, int dataSize)
{
    //全部保存起来，让之后的多线程处理
    unique_lock<mutex> lock(m_msgQueueMutex);
    shared_ptr<SESSIONMSGSTRUCT> target (new SESSIONMSGSTRUCT(dataSize) );
    target->m_msgType = type;
    target->m_ip = ip;
    target->m_port = port;
    target->m_socketid = socketid;
    if(data >0)
    {
        memcpy(target->m_data,data,dataSize);
        target->m_dataSize = dataSize;
    }
    m_msgQueue.push(target);
}


void MsgLooper::OnMsgHandle()
{
    if( OnLoopStart()  != 0 )
    {
        LOG_ERROR<<"MsgLooper::OnLoopStart() error";
        return;
    }

    while(m_bStopProcess == false)
    {

        shared_ptr<SESSIONMSGSTRUCT> target;
        {
            unique_lock<mutex> lock(m_msgQueueMutex);
            if(m_msgQueue.size() != 0)
            {
                target = m_msgQueue.front();
                m_msgQueue.pop();
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
        }
        m_sessionFactory->OnMsgReceive(target->m_msgType, target->m_socketid, target->m_ip,target ->m_port, target->m_data, target->m_dataSize);
    }

    LOG_INFO  <<"MsgLooper:: OnMsgHandle End! thread id = "<<getpid();
}

int MsgLooper::RunProcess()
{
    LOG_INFO  <<"MsgLooper:: RunProcess start! thread id = "<<  this_thread::get_id();
    OnMsgHandle();
    LOG_INFO  <<"MsgLooper:: RunProcess end! thread id = "<<  this_thread::get_id();
    return 0;
}

int MsgLooper::StartProcess()
{
    m_processThread =  thread(bind(&MsgLooper::RunProcess,this));
    return 0;
}


int MsgLooper::StopProcess()
{

    m_bStopProcess = true;
    if(m_processThread.joinable())
    {
        m_processThread.join();
    }
    return 0;
}


int MsgLooper::OnLoopStart()
{
    return m_sessionFactory->PreCreateSession();
}

void MsgLooper::SetSessionFactory(shared_ptr<BaseSessionFactory> factory)
{
    m_sessionFactory = factory;
}

