#ifndef _EISOCK_H_
#define _EISOCK_H_

#include <EIPlatform.h>

struct SSockClient;
typedef bool (*fnSockCallback)(struct SSockClient *pstClient, void *pvUserData, void *pvData, int iLen);
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

bool bSockInit(SSockClient_t *pstClient);
bool bSockUninit(SSockClient_t *pstClient);

#endif
