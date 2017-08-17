#ifndef LOGIN_SESSION_FACTORY_H_H
#define LOGIN_SESSION_FACTORY_H_H

#include "base/BaseSessionFactory.h"

class SessionFactory:public BaseSessionFactory{
public:


    virtual int OnMsgReceive(int type,int socketid ,unsigned int ip,unsigned short port, void* data, int dataSize);

      virtual int PreCreateSession();

    virtual BaseSession* CreateSession(int sessionType);

    virtual int DeteteSession(BaseSession* ins);




};


#endif
