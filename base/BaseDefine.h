#ifndef BASEDEFINE_H_H_2
#define BASEDEFINE_H_H_2

//与当前服务交互的对方对象类型
enum eSessionType{
    SESSIONTYPE_NULL,
    SESSIONTYPE_CLIENT,
    SESSIONTYPE_DB,
    SESSIONTYPE_LOGIN,
    SESSIONTYPE_MSG,
    SESSIONTYPE_ROUTE,
    SESSIONTYPE_MANAGER
};


enum MSGTYPE{
    TCPMSG_CONNECT = 0,//发起一个连接
    TCPMSG_DISCONNECT,//关闭一个连接
    TCPMSG_SEND,//发送一个消息
    TCPMSG_CONNECTED,//连接已经成功的消息
    TCPMSG_RECEIVED,//收到一个消息
    TCPMSG_DISCONNECTED,//收到一个关闭的消息
};


//json 格式的信令定义

//客户端

//登录


























#endif //BASEDEFINE_H_H_2
