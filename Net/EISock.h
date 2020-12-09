#ifndef _EISOCK_H_
#define _EISOCK_H_

#include <EIPlatform.h>

struct SSockClient;
/**
 * @brief 网络接收回调
 */
typedef bool (*fnSockCallback)(struct SSockClient *pstClient, void *pvUserData, void *pvData, int iLen);

/**
 * @brief 网络通信结构体，包含创建网络、连接网络、关闭网络，发送数据和网络通信事件循环
 */
typedef struct SSockClient {
    sock_t          iSocket;
    int             iRecvBufferSize;
    int             iSendBufferSize;
    int             iProtocol;
    struct in_addr  iServerIp;
    short           nServerPort;

    bool (*bCreate)(struct SSockClient *pstClient, const char *ip, short nPort);
    bool (*bConnect)(struct SSockClient *pstClient);
    void (*vClose)(struct SSockClient *pstClient);
    int (*iSend)(struct SSockClient *pstClient, void *pvData, int iLen);
    bool (*bEventLoop)(struct SSockClient *pstClient, fnSockCallback cb, void *pvUserData, int *piLoop, int iTimeOut);
}SSockClient_t;

/**
 * @brief 网络功能初始化
 * @param  pstClient        网络功能对象
 * @return true             网络初始化成功
 * @return false            网络初始化失败
 */
bool bSockInit(SSockClient_t *pstClient);

/**
 * @brief 网络功能反初始化
 * @param  pstClient        网络功能对象
 * @return true             网络反初始化成功
 * @return false            网络反初始化成功
 */
bool bSockUninit(SSockClient_t *pstClient);

#endif
