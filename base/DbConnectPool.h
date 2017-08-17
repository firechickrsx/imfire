#ifndef DBCONNECTPOOL_H
#define DBCONNECTPOOL_H
#include "mysql/mysql.h"

#include <string>
#include <list>
#include <memory>
#include <mutex>
using namespace std;

class DbConnect{
public:
    int CreateConnect();
    int CloseConnect();

    uint32_t m_ip;
    uint16_t m_port;
    bool m_isConnected;
};
typedef shared_ptr<DbConnect> DbConnect_sp;


class DbConnectPool{
public:

    //server 用于标示服务器的名称
    //maxConnectCount 最大连接数
    //preConnectCount    在调用AddServer时预先连接好多少个连接
    //返回连接了多少个

    int AddServer( string  serverId ,unsigned  ip ,unsigned short port  , int maxConnectCount ,int preConnectCount );
    int RemoveServer(string serverId);

    DbConnect_sp GetFreeConnect();

    DbConnectPool();
    ~DbConnectPool();


private:


     mutex m_m_connectQueueMutex;
    list< DbConnect_sp> m_connectList;


};

#endif //DBCONNECTPOOL_H

