/*
#include "DbConnectPool.h"
#include <stdio.h>

DbConnectPool::DbConnectPool()
{

}


DbConnectPool::~DbConnectPool()
{

}

int DbConnectPool::AddServer( string  serverId,unsigned  ip,unsigned short port, int maxConnectCount,int preConnectCount )
{
    int preConnectCountNum  =0;

    for(int i  = 0 ; i<maxConnectCount ; i++){
        DbConnect_sp newDbConnect(new DbConnect);


        m_connectList.push_back(newDbConnect);

        if(preConnectCountNum <= preConnectCount){

        }
         preConnectCountNum++;
    }
    return preConnectCountNum;
}

int DbConnectPool::RemoveServer(string serverId)
{


    return 0;
}
*/
