#include "Server.h"
#include "base/ScopedGuard.h"
#include "base/Logging.h"
#include <thread>
#include "base/Timer.h"

using namespace std::placeholders;

Server::Server()
{
    //ctor
}

Server::~Server()
{
    //dtor
}

void a(){
}

int Server::Run(){

    ON_SCOPE_EXIT([=]{
         m_tcpManager->StopProcess();
         m_msgLooper->StopProcess();
    });

    m_tcpManager.reset(new TcpManager);
    m_msgLooper.reset(new MsgLooper);
    m_sessionFactory.reset(new SessionFactory);


    m_tcpManager->SetTcpMsgReceiver(std::bind(&MsgLooper::PushMsgToLoop, m_msgLooper ,_1,_2,_3,_4,_5,_6));

    m_sessionFactory->SetTcpMsgSender(std::bind(&TcpManager::SendTcpMsg , m_tcpManager, _1 , _2, _3, _4, _5,_6));

    m_msgLooper->SetSessionFactory(m_sessionFactory);

    int ret = m_tcpManager->StartProcess(0,5699,5000);
    if(ret !=0){
        LOG_INFO<<"m_tcpManager.StartProcess fail";
        return -1;
    }


   // ret  =m_msgLooper->StartProcess();
  //  if(ret != 0){
  //      LOG_INFO<<"m_tcpManager.StartProcess fail";
  //      return -1;
  //  }

    while(1){
        sleep(1000);
    }

    return 0;
}
