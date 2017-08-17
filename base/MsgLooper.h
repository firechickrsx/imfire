#ifndef MsgLooper_H
#define MsgLooper_H

#include "BaseSessionFactory.h"
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <map>
#include <vector>
#include <thread>

using namespace std;


class MsgLooper:public enable_shared_from_this<MsgLooper>
{
    public:
        MsgLooper();
        virtual ~MsgLooper();

        void SetSessionFactory(shared_ptr<BaseSessionFactory>);

        void PushMsgToLoop(int type,int socketid ,unsigned int ip,unsigned short port, void* data, int dataSize);

        int OnLoopStart();

        void OnMsgHandle();

        int StartProcess();
        int StopProcess();

    private:
        int RunProcess();

        mutex m_msgQueueMutex;
        queue<shared_ptr<SESSIONMSGSTRUCT> > m_msgQueue;

        queue< shared_ptr<function<void()> > > m_eventQueue;

         //mutex m_sessionMapMutex;


        bool m_bStopProcess;
       thread m_processThread;

   shared_ptr<BaseSessionFactory> m_sessionFactory;

};

#endif // MsgLooper_H
