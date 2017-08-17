#ifndef Server_H
#define Server_H

#include "base/TcpManager.h"
#include "base/MsgLooper.h"
#include "SessionFactory.h"

class Server
{
    public:
        Server();
        virtual ~Server();

        int Run();

    protected:
    private:
       std::shared_ptr<TcpManager> m_tcpManager;
       std::shared_ptr<MsgLooper> m_msgLooper;
       std::shared_ptr<SessionFactory> m_sessionFactory;

};

#endif // Server_H
