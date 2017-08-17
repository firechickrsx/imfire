#ifndef CLIENTSESSION_H
#define CLIENTSESSION_H
#include "base/BaseSession.h"

class ClientSession :public BaseSession{
public:
    ClientSession(BaseSessionFactory* factory);
    ~ClientSession();

};
#endif
