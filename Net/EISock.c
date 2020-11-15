#include "EIPlatform.h"
#include "EISock.h"
#include "EILog.h"

bool bSocketCreate(struct SSockClient *pstClient, const char *ip, short nPort)
{
    int nRet;
    int iSocket;
// 获取ip和port地址
#if defined(_WIN32)
    pstClient->iServerIp.S_un.S_addr = inet_addr(ip);
#else
    pstClient->iServerIp.s_addr = inet_addr(ip);
#endif
    pstClient->nServerPort = htons(nPort);

    // 创建socket对象
    if (IPPROTO_TCP == pstClient->iProtocol)
    {
        pstClient->iSocket = socket(AF_INET, SOCK_STREAM, 0);
    }
    else if (IPPROTO_UDP == pstClient->iProtocol)
    {
        pstClient->iSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }

    iSocket = pstClient->iSocket;
#ifdef _WIN32
    errno = WSAGetLastError();
#endif
    if (INVALID_SOCKET == pstClient->iSocket)
    {
        return false;
    }

    //设置套接字接收和发送缓冲区大小
    // if(pstClient->iRecvBufferSize > 0)
    // 	setsockopt(iSocket, SOL_SOCKET, SO_RCVBUF, (char*)&pstClient->iRecvBufferSize, sizeof(int));
    // if(pstClient->iSendBufferSize > 0)
    // 	setsockopt(iSocket, SOL_SOCKET, SO_SNDBUF, (char*)&pstClient->iSendBufferSize, sizeof(int));

    return true;
}

bool bStartConnect(struct SSockClient *pstClient)
{
    int nRet;
    int iSocket = pstClient->iSocket;
    struct sockaddr_in stRemoteAddr;
    memset(&stRemoteAddr, 0, sizeof(stRemoteAddr));

    stRemoteAddr.sin_family = AF_INET;
    stRemoteAddr.sin_addr = pstClient->iServerIp;
    stRemoteAddr.sin_port = pstClient->nServerPort;

    //连接服务端
    nRet = connect(iSocket, (struct sockaddr *)&stRemoteAddr, sizeof(struct sockaddr));
    if (SOCKET_ERROR == nRet)
    {
        closesocket(iSocket);
        pstClient->iSocket = INVALID_SOCKET;
        return false;
    }

    //设置非阻塞模式
    int nBlock = 1;
#ifdef _WIN32
    nRet = ioctlsocket(iSocket, FIONBIO, (u_long FAR *)&nBlock);
    if (nRet == SOCKET_ERROR)
    {
        errno = WSAGetLastError();
        nRet = -1;
    }
#else
    nBlock = fcntl(iSocket, F_GETFL, 0);
    if (nBlock != -1)
    {
        nBlock |= O_NONBLOCK;
        nRet = fcntl(iSocket, F_SETFL, nBlock);
    }
#endif
    if (-1 == nRet)
    {
        closesocket(iSocket);
        pstClient->iSocket = INVALID_SOCKET;
        return false;
    }
    return true;
}

void vCloseConnect(struct SSockClient *pstClient)
{
    int iSocket = pstClient->iSocket;
    if (pstClient->iSocket != INVALID_SOCKET)
    {
        closesocket(pstClient->iSocket);
        pstClient->iSocket = INVALID_SOCKET;
    }
}

int iSockSend(struct SSockClient *pstClient, void *pvData, int iLen)
{
    const char *psData = pvData;
    int iSize = 0, iCurSize = 0;
    int iLeftSize = iLen;
    while (iLeftSize > 0)
    {
        iSize = send(pstClient->iSocket, psData + iCurSize, iLeftSize, 0);
        if (SOCKET_ERROR == iSize)
        {
#ifdef _WIN32
            errno = WSAGetLastError();
#endif
            LOG(EERROR, "iSize:%d, errcode:%d, errdetail:%s",
                iSize, errno, strerror(errno));
            return iSize;
        }
        iCurSize += iSize;
        iLeftSize -= iSize;
    }
    return iCurSize;
}

bool bSockEventLoop(struct SSockClient *pstClient, fnSockCallback cb, void *pvUserData, int *piLoop, int iTimeOut)
{
    int fds, nRet;
    sock_t iSocket = pstClient->iSocket;
    int nNeeRecvLen = pstClient->iRecvBufferSize, nRecvLen = 0, ntmpLen = 0;
    struct timeval tv;
    char szbuf[1024 * 4 + 1];
    int iTotal;
    fd_set fd_recv;
    FD_ZERO(&fd_recv);
    struct sockaddr_in addr;
    int iAddrSize = sizeof(addr);

    while (*piLoop)
    {
        tv.tv_sec = iTimeOut / 1000;
        tv.tv_usec = iTimeOut % 1000;
        fds = 0;
        FD_ZERO(&fd_recv);
        FD_SET(iSocket, &fd_recv);
        fds = iSocket;

        iTotal = select(fds + 1, &fd_recv, 0, 0, &tv);
        if (SOCKET_ERROR == iTotal) // error
        {
#ifdef _WIN32
            errno = WSAGetLastError();
#endif
            LOG(EERROR, "Sock::TcpRecv socket error:%s", strerror(errno));
        }
        else if (0 == iTotal) // timeout
        { 
            LOG(ETRACE, "select timeout, ret:%d", iTotal);
            continue;
        }

        // 接收数据
        memset(szbuf, 0, sizeof(szbuf));
        nRecvLen = recv(iSocket, szbuf, nNeeRecvLen, 0);
        // LOG(EDEBUG, "nRecvLen:%d", nRecvLen);
#ifdef _WIN32
        if (SOCKET_ERROR == nRecvLen)
        {
            errno = WSAGetLastError();
            if (EWOULDBLOCK == errno)
            {
#else
        if (-1 == nRecvLen)
        {
            if (EAGAIN == errno)
            {
                errno = EWOULDBLOCK;
#endif
            }
            else
            {
                LOG(EERROR, "Sock::TcpRecv socket error:%s", strerror(errno));
            }
        }
        else if (nRecvLen == 0) //套接字关闭或者没有读取到内容
        {
            LOG(EDEBUG, "读取结束");
            break;
        }
        else
        {
            // 回调返回非true结束连接
            if (cb(pstClient, pvUserData, szbuf, nRecvLen))
                break;
        }
    }

    if (iSocket)
    {
        closesocket(iSocket);
        pstClient->iSocket = INVALID_SOCKET;
    }
    return true;
}

bool bSockInit(SSockClient_t *pstClient)
{
#ifdef _WIN32
    WSADATA wsa = {0};
    int ret = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (ret != 0)
    {
        return false;
    }
#endif

    pstClient->bCreate = bSocketCreate;
    pstClient->bConnect = bStartConnect;
    pstClient->vClose = vCloseConnect;
    pstClient->iSend = iSockSend;
    pstClient->bEventLoop = bSockEventLoop;
    pstClient->iSocket = INVALID_SOCKET;
    pstClient->iRecvBufferSize = 4096;
    pstClient->iSendBufferSize = 4096;
    pstClient->iProtocol = IPPROTO_TCP;
    pstClient->iServerIp.s_addr = INADDR_ANY;
    pstClient->nServerPort = 0;

    return true;
}
bool bSockUninit(SSockClient_t *pstClient)
{
#ifdef _WIN32
    if (WSACleanup() == SOCKET_ERROR)
    {
        return true;
    }
#endif
    return false;
}
