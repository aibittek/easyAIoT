#ifndef _EIHTTPCLIENT_H
#define _EIHTTPCLIENT_H

#include "EIString.h"
#include "EISock.h"
#include "map.h"
#include "ringbuffer.h"

// HTTP协议的请求数据结构体
typedef struct {
    char szMethod[8];               // 方法："POST","GET"
    EIString *pstPath;              // URL路径
    EIString *pstHost;              // 服务器域名
    int16_t nPort;                  // 服务器端口号
    EIString *pstExtraHeader;       // 额外的头信息
    uint32_t content_length;        // POST方法有效
    EIString *pstBody;              // POST方法的数据部分
    char sShakeKey[32];             // 32位随机数
}SEIHttpRequest_t;
// HTTP协议的返回数据结构体
typedef struct {
    int32_t iStatus;                // 返回状态
    map_void_t mapResponse;         // 头信息列表
    bool bMapInit;                  // 是否初始化
    EIString *pstBody;              // POST方法的数据部分
}SEIHttpResponse_t;

// Websocket接口
typedef enum EEIWS_MESSAGE
{
    EEIWS_ON_HAND_SHAKE,
    EEIWS_ON_MESSAGE,
    EEIWS_ON_CLOSE,
    EEIWS_ON_ERROR,
}EEIWS_MESSAGE;
typedef enum EEIWS_PARSE_STATUS
{
    EEIWS_PARSE_OPCODE = 0,
    EEIWS_PARSE_MASK_PLAYLOADSIZE,
    EEIWS_PARSE_PAYLOAD_SIZE,
    EEIWS_PARSE_PAYLOAD_SIZE16,
    EEIWS_PARSE_PAYLOAD_SIZE64,
    EEIWS_PARSE_MASKDATA,
    EEIWS_PARSE_PAYLOAD
}EEIWS_PARSE_STATUS;
typedef enum EEIWS_OPCODE {
    EEIWS_OPCODE_MINDATA = 0x0,      // 0x0：标识一个中间数据包
    EEIWS_OPCODE_TXTDATA = 0x1,      // 0x1：标识一个txt类型数据包
    EEIWS_OPCODE_BINDATA = 0x2,      // 0x2：标识一个bin类型数据包
    EEIWS_OPCODE_DISCONN = 0x8,      // 0x8：标识一个断开连接类型数据包
    EEIWS_OPCODE_PING = 0x9,         // 0x9：表示一个ping类型数据包
    EEIWS_OPCODE_PONG = 0xA,         // 0xA：表示一个pong类型数据包
    EEIWS_OPCODE_ERR = -1,
    EEIWS_OPCODE_NULL = 0
}EEIWS_OPCODE;
typedef struct SEIWSFrame
{
    EEIWS_PARSE_STATUS eStatus;
    int iType;
    EEIWS_OPCODE eOpcode;
    int iMask;
    int iCount;
    unsigned long dwPlayloadSize;
    unsigned long dwCurSize;
    unsigned char sMask[4];
    EIString *pstPlayload;
}SEIWSFrame_t;
typedef bool (*fnWebsocketCallback)(SSockClient_t *pstSock, void *pvCBUserData, EEIWS_MESSAGE eType, void *pvMessage, int iLen);
typedef struct SEIHttpParser{
    int8_t sRecvBuffer[4096*2];     // 接收数据缓存
    int32_t iCurLen;                // 当前处理得数据长度
    int32_t iTotalLen;              // recvbuffer有效数据长度
    int32_t iContentLength;         // ContentLength长度
    int32_t iLength;                // 当前临时获取得数据长度

    bool bIsTrunkLength;            // 是否是在trunked数据获取长度状态
    bool bNotFirst;                 // 是否第一次解析
    bool bTrunked;                  // 是否为trunk数据
    bool bHeadEnd;                  // head部分是否解析完成
    bool bBodyEnd;                  // body部分是否解析完成
    bool bWebsocket;                // 是否是websocket协议
    fnWebsocketCallback fwsCallback;// websocket解析回调函数
    void *pvCBUserData;             // 回调用户数据
    ring_buffer_t stRingBuffer;     // websocket 解码数据ring buffer
    SEIWSFrame_t stWSFrame;         // websocket frame
}SEIHttpParser_t;
typedef struct SEIHttpInfo{
    SEIHttpRequest_t stRequest;     // http请求消息
    SEIHttpResponse_t stResponse;   // http返回消息
    SSockClient_t stSockClient;     // socket网络连接
    SEIHttpParser_t stHttpParser;   // http解析器
}SEIHttpInfo_t;
int http_header(map_void_t *pmap, char *pszString, bool bNorFirst);
bool bHttpUrlParse(const char *pszUrl, EIString** pstHost, EIString** pstPath, uint16_t* pnPort);
bool bHttpOpen(SEIHttpInfo_t *pstHttpInfo, const char *pszUrl, const char *pszExtraHeader, void *pvBody, int32_t iSize);
bool bHttpClose(SEIHttpInfo_t *pstHttpInfo);

/**
 * @brief 连接http服务端
 * 
 * @param pstHttpInfo httpclient的结构体，包含了请求、响应、网络连接、http解析四个功能
 * @param pszUrl 访问的http url
 * @param pszExtraHeader 额外的头文件
 * @param pvBody 如果是post消息，需要提供这一项，将会填充到http请求的body中
 * @param iSize body的长度
 * 
 * @return 调用成功返回true，否则返回false
 * 
 * @note 注意返回结果在pstHttpInfo的stResponse成员中，并且调用完成后
 *       要通过bHttpClose来释放pstHttpInfo中的资源
 */
bool bConnectHttpServer(SEIHttpInfo_t *pstHttpInfo, const char *pszUrl, const char *pszExtraHeader, void* pvBody, int iSize);
#endif