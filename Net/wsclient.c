#include "ntp.h"
#include "EILog.h"
#include "cJSON_user_define.h"
#include "urlencode.h"
#include "base64.h"
#include "sha1.h"
#include "sha256.h"
#include "hmac_sha256.h"
#include "md5.h"
#include "wsclient.h"

#define APPID "5d2f27d2"
#define APP_SECRET "8110566cd9dd13066f9a1e38aeb12a48"
#define APP_KEY "a8331910d59d41deea317a3c76d47b60"
#define TTS_PARAM "{\"result_level\":\"plain\",\"auth_id\":\"27853aa9684eb19789b784a89ea5befd\",\"data_type\":\"audio\",\"sample_rate\":\"16000\",\"scene\":\"main_box\"}"
#define TTS_URL "https://ws-api.xfyun.cn/v2/tts"
#define TTS_TEXT "讯飞开放平台"

static int iGetGMTDate(char *szDate, int iLen)
{
    int iCount = 3;
    time_t t = 0;
    char szTime[5][10];
    // 获取系统当前NTP时间
    do {
        // t = dwGetNTPtime("cn.ntp.org.cn");
        t = dwGetNTPtime("ntp.aliyun.com");
        if (t > 0) break;
    } while(iCount--);
    // 以上获取了北京的NTP时间，北京位于东8区，比GMT(格林尼治)时间快8小时
    t -= 8 * 60 * 60;
    // Thu, 29 Sep 2011 15:04:39 GMT
    // Tue Mar 03 12:58:45 2020
    char *pszTime = ctime(&t);
    if (NULL == pszTime) return -1;
    sscanf(pszTime, "%s %s %s %s %s", szTime[0], szTime[1], szTime[2], szTime[3], szTime[4]);
    snprintf(szDate, iLen, "%s, %s %s %s %s GMT", szTime[0], szTime[2], szTime[1],
            szTime[4], szTime[3]);
    // strcpy(szDate, "Tue, 03 Mar 2020 11:44:51 GMT");
    return 0;
}
static int iHmacSha256(const char *szSrc, const char *szKey, char *szDst)
{
    unsigned char digest[32];
    hmac_sha256(szSrc, strlen(szSrc), szKey, strlen(szKey), szDst);
    // for(int i=0; i<32; i++)
    // {
    //     sprintf(szDst+i*2, "%2.2x",  digest[i]);
    // }
    return 0;
}
// 请求方式：ws(s)://tts-api.xfyun.cn/v2/tts?authorization=base64(authorization_origin)&date=Thu%2C%2001%20Aug%202019%2001%3A53%3A21%20GMT&host=tts-api.xfyun.cn"
// authorization_origin:
// api_key="$api_key",algorithm="hmac-sha256",headers="host date request-line",signature="$signature"
//
// signature_origin内容如下：
// host: tts-api.xfyun.cn
// date: Thu, 01 Aug 2019 01:53:21 GMT
// GET /v2/tts HTTP/1.1
// $signature_sha=hmac-sha256(signature_origin, $apiSecret)
// $signature=base64(signature_sha)
static int iGetAuthUrl(const char *c_szUrl, const char *c_szKey,
                const char *c_szSecret, const char *c_szDate,
                char *szAuth, int iLen)
{
    char szSignatureOrigin[1024];
    char szSignatureOriginSha[65] = {0};
    char szSignatureOriginBase64[1024];
    char szAuthOri[4096];
    sprintf(szSignatureOrigin, "host: ws-api.xfyun.cn\ndate: %s\nGET /v2/tts HTTP/1.1",
            c_szDate);
    printf("%s\n", szSignatureOrigin);
    iHmacSha256(szSignatureOrigin, c_szSecret, szSignatureOriginSha);
    // printf("%s\n", szSignatureOriginSha);
    iBase64Encode(szSignatureOriginSha, szSignatureOriginBase64, 32);
    printf("%s\n", szSignatureOriginBase64);
    sprintf(szAuthOri, "api_key=\"%s\", algorithm=\"hmac-sha256\", headers=\"host date request-line\", signature=\"%s\"",
            c_szKey, szSignatureOriginBase64);
    printf("%s\n", szAuthOri);
    iBase64Encode(szAuthOri, szAuth, strlen(szAuthOri));
    printf("%s\n", szAuth);
    return 0;
}
static void generate_randomkey(unsigned char *buf, unsigned int len)
{
    char szRandom[16] = {0};
    char psMD5[16] = {0};
    snprintf(szRandom, sizeof(szRandom), "%p", generate_randomkey);
    vMD5((uint8_t *)szRandom, sizeof(szRandom), psMD5);
    memcpy(buf, psMD5, len);
    for(int i = 0; i < len; i++) {
        if(0 == buf[i]) buf[i] = 128;
    }
}
static int wss_generatekey(unsigned char *shake_key)
{
        unsigned char random_key[WEBSOCKET_SHAKE_KEY_LEN] = {0};
        generate_randomkey(random_key, WEBSOCKET_SHAKE_KEY_LEN);
        return iBase64Encode((const unsigned char *)random_key, (char *)shake_key, WEBSOCKET_SHAKE_KEY_LEN);
}
/*******************************************************************************
 * 名称: _webSocket_buildRespondShakeKey
 * 功能: server端在接收client端的key后,构建回应用的key
 * 形参: *acceptKey：来自客户端的key字符串
 *         acceptKeyLen : 长度
 *          *respondKey :  在 acceptKey 之后加上 GUID, 再sha1哈希, 再转成base64得到 respondKey
 * 返回: respondKey的长度(肯定比acceptKey要长)
 * 说明: 无
 ******************************************************************************/
static int _webSocket_buildRespondShakeKey(const char *acceptKey, unsigned int acceptKeyLen, char *respondKey)
{
    char clientKey[128] = {0};  
    char sha1DataOutput[20] = {0};  
    char sha1DataHex[41] = {0};  
    char *sha1Data;  
    int i, n;  
    const char GUID[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    unsigned int GUIDLEN;
 
    if(acceptKey == NULL)  
        return 0;  
    GUIDLEN = sizeof(GUID);
    memcpy(clientKey, acceptKey, acceptKeyLen); 
    memcpy(&clientKey[acceptKeyLen], GUID, GUIDLEN);
    clientKey[acceptKeyLen + GUIDLEN] = '\0';
    //
    mbedtls_sha1(clientKey, strlen(clientKey), sha1DataOutput); 
    // for (int i=0; i<20; i++) {
    //     sprintf(sha1DataHex+i*2, "%2.2x", sha1DataOutput[i]);
    // }
    iBase64Encode((const unsigned char *)sha1DataOutput, (char *)respondKey, 20);
    //
    return strlen(respondKey);
}
/*******************************************************************************
 * 名称: webSocket_matchShakeKey
 * 功能: client端收到来自服务器回应的key后进行匹配,以验证握手成功
 * 形参: *myKey：client端请求握手时发给服务器的key
 *            myKeyLen : 长度
 *          *acceptKey : 服务器回应的key
 *           acceptKeyLen : 长度
 * 返回: 0 成功  -1 失败
 * 说明: 无
 ******************************************************************************/
static int _webSocket_matchShakeKey(unsigned char *myKey, unsigned int myKeyLen, unsigned char *acceptKey, unsigned int acceptKeyLen)
{
    int retLen;
    unsigned char tempKey[256] = {0};
    //
    retLen = _webSocket_buildRespondShakeKey(myKey, myKeyLen, tempKey);
    LOG(EDEBUG, "_webSocket_matchShakeKey :\r\n%d : %s\r\n%d : %s\r\n", acceptKeyLen, acceptKey, retLen, tempKey);
    //
    if(retLen != acceptKeyLen)
    {
        LOG(EDEBUG, "_webSocket_matchShakeKey : len err\r\n%s\r\n%s\r\n%s\r\n", myKey, tempKey, acceptKey);
        return -1;
    }
    else if(strcmp((const char *)tempKey, (const char *)acceptKey) != 0)
    {
        LOG(EDEBUG, "_webSocket_matchShakeKey : str err\r\n%s\r\n%s\r\n", tempKey, acceptKey);
        return -1;
    }
    return 0;
}
static void vWebsocketFrameClear(SEIWSFrame_t *pstWSFrame)
{
    pstWSFrame->dwCurSize = 0;
    pstWSFrame->dwPlayloadSize = 0;
    pstWSFrame->eOpcode = EEIWS_OPCODE_NULL;
    pstWSFrame->eStatus = EEIWS_PARSE_OPCODE;
    pstWSFrame->iCount = 0;
    pstWSFrame->iMask = 0;
    pstWSFrame->iType = 0;
    if (pstWSFrame->pstPlayload) {
        vEIStringDelete(pstWSFrame->pstPlayload);
        pstWSFrame->pstPlayload = NULL;
    }
}
static bool bParseWebsocketFrame(ring_buffer_t *pstRingBuffer, SEIWSFrame_t *pstWSFrame)
{
    bool bEnd = false;
    bool bLoop = true;
    unsigned char byData;
    do {
        switch(pstWSFrame->eStatus) {
        case EEIWS_PARSE_OPCODE:
        {
            if (ring_buffer_dequeue(pstRingBuffer, &byData)) {
                int type = byData & 0x0F;
                if((byData & 0x80) == 0x80)
                {
                    if(type == 0x01) 
                        pstWSFrame->eOpcode = EEIWS_OPCODE_TXTDATA;
                    else if(type == 0x02) 
                        pstWSFrame->eOpcode = EEIWS_OPCODE_BINDATA;
                    else if(type == 0x08) 
                        pstWSFrame->eOpcode = EEIWS_OPCODE_DISCONN;
                    else if(type == 0x09) 
                        pstWSFrame->eOpcode = EEIWS_OPCODE_PING;
                    else if(type == 0x0A) 
                        pstWSFrame->eOpcode = EEIWS_OPCODE_PONG;
                    else 
                        pstWSFrame->eOpcode = EEIWS_OPCODE_ERR;
                }
                else if(type == 0x00)
                    pstWSFrame->eOpcode = EEIWS_OPCODE_MINDATA;
                else {
                    pstWSFrame->eOpcode = EEIWS_OPCODE_ERR;
                    break;
                }  
            } else {
                // 需要读取更多的数据
                bLoop = false;
                break;
            }
            if (EEIWS_OPCODE_ERR != pstWSFrame->eOpcode) {
                pstWSFrame->eStatus = EEIWS_PARSE_MASK_PLAYLOADSIZE;
            }
        }
            break;
        case EEIWS_PARSE_MASK_PLAYLOADSIZE:
        {
            if (ring_buffer_dequeue(pstRingBuffer, &byData)) {
                if((byData & 0x80) == 0x80)
                {
                    pstWSFrame->iMask = 1;
                    pstWSFrame->iCount = 4;
                }
                else
                {
                    pstWSFrame->iMask = 0;
                    pstWSFrame->iCount = 0;
                }
                int iLen = byData & 0x7F;
                if (iLen < 126) {
                    if (pstWSFrame->iMask) {
                        pstWSFrame->eStatus = EEIWS_PARSE_MASKDATA;
                    } else {
                        pstWSFrame->eStatus = EEIWS_PARSE_PAYLOAD;
                    }
                    pstWSFrame->dwPlayloadSize = iLen;
                }
                else if (126 == iLen) {
                    pstWSFrame->eStatus = EEIWS_PARSE_PAYLOAD_SIZE16;
                }
                else { // 127 == iLen
                    pstWSFrame->eStatus = EEIWS_PARSE_PAYLOAD_SIZE64;
                }
            }
            else {
                bLoop = false;
                break;
            }
        }
            break;
        case EEIWS_PARSE_PAYLOAD_SIZE16:
        {
            unsigned char sSize[2];
            int iLen = ring_buffer_num_items(pstRingBuffer);
            if (iLen < sizeof(sSize)) {
                bLoop = false;
                break;
            }
            ring_buffer_dequeue_arr(pstRingBuffer, sSize, sizeof(sSize));
            iLen = sSize[0];
            iLen = (iLen << 8) + sSize[1];
            pstWSFrame->dwPlayloadSize = iLen;
            if (pstWSFrame->iMask) {
                pstWSFrame->eStatus = EEIWS_PARSE_MASKDATA;
            } else {
                pstWSFrame->eStatus = EEIWS_PARSE_PAYLOAD;
            }
        }
            break;
        case EEIWS_PARSE_PAYLOAD_SIZE64:
        {
            unsigned char sSize[8];
            unsigned long dwLen = ring_buffer_num_items(pstRingBuffer);
            if (dwLen < sizeof(sSize)) {
                bLoop = false;
                break;
            }
            ring_buffer_dequeue_arr(pstRingBuffer, sSize, sizeof(sSize));
            dwLen = sSize[0];
            dwLen = (dwLen << 8) + sSize[1];
            dwLen = (dwLen << 8) + sSize[2];
            dwLen = (dwLen << 8) + sSize[3];
            dwLen = (dwLen << 8) + sSize[4];
            dwLen = (dwLen << 8) + sSize[5];
            dwLen = (dwLen << 8) + sSize[6];
            dwLen = (dwLen << 8) + sSize[7];
            pstWSFrame->dwPlayloadSize = dwLen;
            if (pstWSFrame->iMask) {
                pstWSFrame->eStatus = EEIWS_PARSE_MASKDATA;
            } else {
                pstWSFrame->eStatus = EEIWS_PARSE_PAYLOAD;
            }
        }
            break;
        case EEIWS_PARSE_MASKDATA:
        {
            int iLen = ring_buffer_num_items(pstRingBuffer);
            if (iLen < pstWSFrame->iCount) {
                bLoop = false;
                break;
            }
            ring_buffer_dequeue_arr(pstRingBuffer, pstWSFrame->sMask, sizeof(pstWSFrame->sMask));
            pstWSFrame->eStatus = EEIWS_PARSE_PAYLOAD;
        }
            break;
        case EEIWS_PARSE_PAYLOAD:
        {
            int iLen;
            if (NULL == pstWSFrame->pstPlayload) {
                pstWSFrame->pstPlayload = stEvIStringNew(pstWSFrame->dwPlayloadSize);
            }
            while (pstWSFrame->dwCurSize < pstWSFrame->dwPlayloadSize) {
                iLen = ring_buffer_dequeue_arr(pstRingBuffer, 
                    pstWSFrame->pstPlayload->sBuffer+pstWSFrame->dwCurSize, 
                    pstWSFrame->dwPlayloadSize-pstWSFrame->dwCurSize);
                if (iLen <= 0) {
                    bLoop = false;
                    break;
                }
                pstWSFrame->dwCurSize += iLen;
            }
            if (pstWSFrame->dwCurSize >= pstWSFrame->dwPlayloadSize) {
                bLoop = false;
                bEnd = true;
            }
        }
            break;
        default:
            break;
        }
    } while(bLoop);
    return bEnd;
}
static bool bDefaultWSCallback(struct SSockClient *pstClient, void *pvUserData, void *pvData, int iLen)
{
    bool bRet = false;
    int iParseLen;
    char *pPrev, *pNext;
    SEIHttpInfo_t *pstHttpInfo = pvUserData;
    SEIHttpParser_t *pstHttpParser = &pstHttpInfo->stHttpParser;
    SEIHttpRequest_t *pstHttpRequest = &pstHttpInfo->stRequest;
    SEIHttpResponse_t *pstResponse = &pstHttpInfo->stResponse;
    SSockClient_t *pstSockClient = &pstHttpInfo->stSockClient;
    if (iLen <= 0) return true;
    if (!pstResponse->bMapInit) {
        map_init(&pstResponse->mapResponse);
        pstResponse->bMapInit = true;
    }
    // LOG(EDEBUG, "%s", pvData);
    memset(pstHttpParser->sRecvBuffer, 0, sizeof(pstHttpParser->sRecvBuffer));
    strncat(pstHttpParser->sRecvBuffer+pstHttpParser->iCurLen, 
        pvData, iLen);
    pstHttpParser->iTotalLen = pstHttpParser->iCurLen + iLen;
    pPrev = pstHttpParser->sRecvBuffer;
    while(1) {
        if (!pstHttpParser->bHeadEnd) { // 解析websocket头部
            if ((pNext = strstr(pPrev, "\r\n")) != NULL) {
                iParseLen = pNext - pPrev;
                *pNext = 0;
                if (iParseLen > 0) {
                    http_header(&pstResponse->mapResponse, pPrev, pstHttpParser->bNotFirst);
                    if (!pstHttpParser->bNotFirst) {
                        EIString **ppstValue = (EIString **)map_get(&pstResponse->mapResponse, "HTTP/1.1");
                        if (ppstValue && *ppstValue) {
                            pstResponse->iStatus = atoi((*ppstValue)->sBuffer);
                        }
                    }
                    pstHttpParser->bNotFirst = true;
                    pPrev = pNext + 2;    // skip CR+LF
                    pstHttpParser->iCurLen = (pPrev-(char *)pstHttpParser->sRecvBuffer);
                } else {
                    pstHttpParser->bHeadEnd = true;
                    pPrev = pNext + 2;    // skip CR+LF
                    pstHttpParser->iCurLen = (pPrev-(char *)pstHttpParser->sRecvBuffer);
                    EIString **ppstValue = (EIString **)map_get(&pstResponse->mapResponse, "Content-Length");
                    if (ppstValue && *ppstValue && (*ppstValue)->sBuffer) {
                        pstHttpParser->iContentLength = atoi((*ppstValue)->sBuffer);
                    }
                    ppstValue = (EIString **)map_get(&pstResponse->mapResponse, "Sec-WebSocket-Accept");
                    if (ppstValue && *ppstValue && (*ppstValue)->sBuffer) {
                        if(_webSocket_matchShakeKey(pstHttpRequest->sShakeKey, 
                            strlen(pstHttpRequest->sShakeKey), 
                            (*ppstValue)->sBuffer, (*ppstValue)->lSize) == 0) {
                            pstHttpParser->fwsCallback(pstSockClient, EEIWS_ON_HAND_SHAKE, NULL, 0);
                            ring_buffer_init(&pstHttpParser->stRingBuffer);
                            break;
                        } else {
                            const char *pszError = "not match shakekey";
                            pstHttpInfo->stHttpParser.fwsCallback(pstSockClient, EEIWS_ON_ERROR, (void *)pszError, strlen(pszError)+1);
                            return true;
                        }
                    }
                }
            } else {
                pstHttpParser->iCurLen -= (pPrev-(char *)pstHttpParser->sRecvBuffer);
                strncpy(pstHttpParser->sRecvBuffer, pPrev, pstHttpParser->iTotalLen);
                break;
            }
        }
        else if (!pstHttpParser->bBodyEnd) {
            if (!pstResponse->pstBody) {
                pstResponse->pstBody = stEvIStringNew(0);
            }
            if (pstHttpParser->iContentLength > 0) {
                
                if (pstHttpParser->iLength < pstHttpParser->iContentLength) {
                    int iLen = pstHttpParser->iTotalLen - pstHttpParser->iCurLen;
                    stEIStringRealloc(pstResponse->pstBody, 
                        pstHttpParser->sRecvBuffer+pstHttpParser->iCurLen, iLen);
                    pstHttpParser->iLength += iLen;
                    pstHttpParser->iCurLen = 0;
                } else {
                    pstHttpParser->bBodyEnd = true;
                    continue;
                }
            } else {
                // 解析websocket数据
                ring_buffer_queue_arr(&pstHttpParser->stRingBuffer, pvData, iLen);
                while (ring_buffer_num_items(&pstHttpParser->stRingBuffer) > 0) {
                    if (bParseWebsocketFrame(&pstHttpParser->stRingBuffer, &pstHttpParser->stWSFrame)) {
                        // LOG(EDEBUG, "%s", pstHttpParser->stWSFrame.pstPlayload->sBuffer);
                        bRet = pstHttpParser->fwsCallback(pstSockClient, EEIWS_ON_MESSAGE, 
                            pstHttpParser->stWSFrame.pstPlayload->sBuffer, pstHttpParser->stWSFrame.pstPlayload->lSize);
                        vWebsocketFrameClear(&pstHttpParser->stWSFrame);
                        if (bRet) return bRet;
                    }
                    else {
                        return bRet;
                    }
                }
            }
        }
        else { // 解析websocket通信消息
            LOG(EDEBUG, "websocket 通信消息");
            return true;
        }
    }
    
    return bRet;
}

bool bWSConnect(SEIHttpInfo_t *pstHttpInfo, fnWebsocketCallback cb)
{
    char request[4096];
    SEIHttpRequest_t *pstRequest = &pstHttpInfo->stRequest;
    SSockClient_t *pstSockClient = &pstHttpInfo->stSockClient;
    SEIHttpParser_t *pstHttpParser = &pstHttpInfo->stHttpParser;
    // 构造websocket Sec-WebSocket-Key字段
    wss_generatekey(pstRequest->sShakeKey);
    
    // 构造websocket握手协议
    const char base_header[] = "GET %s HTTP/1.1\r\n"
                                "Connection: Upgrade\r\n"
                                "Host: %s:%d\r\n"
                                "Sec-WebSocket-Key: %s\r\n"
                                "Sec-WebSocket-Version: 13\r\n"
                                "Upgrade: websocket\r\n\r\n";
    int iLen = snprintf(request, sizeof(request), base_header, 
        pstRequest->pstPath->sBuffer, pstRequest->pstHost->sBuffer, 
        pstRequest->nPort, pstRequest->sShakeKey);
    LOG(EDEBUG, "request:%s", request);

    // 连接websocket服务端
    int iLoop = 1;
    pstSockClient->bConnect(pstSockClient);
    pstSockClient->iSend(pstSockClient, request, iLen);
    pstHttpParser->bWebsocket = true;
    pstHttpParser->fwsCallback = cb;
    pstSockClient->bEventLoop(pstSockClient, bDefaultWSCallback, pstHttpInfo, &iLoop, 1000);
    return true;
}

bool bWebsocketConnect(const char *c_pszUrl, fnWebsocketCallback cb) {
    SEIHttpInfo_t stHttpInfo;
    bHttpOpen(&stHttpInfo, c_pszUrl, NULL, NULL, 0);
    bWSConnect(&stHttpInfo, cb);
    bHttpClose(&stHttpInfo);
}
/*******************************************************************************
 * 名称: webSocket_enPackage
 * 功能: websocket数据收发阶段的数据打包, 通常client发server的数据都要isMask(掩码)处理, 反之server到client却不用
 * 形参: *data：准备发出的数据
 *          dataLen : 长度
 *        *package : 打包后存储地址
 *        packageMaxLen : 存储地址可用长度
 *          isMask : 是否使用掩码     1要   0 不要
 *          type : 数据类型, 由打包后第一个字节决定, 这里默认是数据传输, 即0x81
 * 返回: 打包后的长度(会比原数据长2~16个字节不等)      <=0 打包失败 
 * 说明: 无
 ******************************************************************************/
int wss_encode_package(unsigned char *data, unsigned int dataLen, unsigned char *package, unsigned int packageMaxLen, bool isMask, EEIWS_OPCODE type)
{
    unsigned char maskKey[4] = {0};    // 掩码
    unsigned char temp1, temp2;
    int count;
    unsigned int i, len = 0;
 
    if(packageMaxLen < 2)
        return -1;
 
    if(type == EEIWS_OPCODE_MINDATA)
        *package++ = 0x00;
    else if(type == EEIWS_OPCODE_TXTDATA)
        *package++ = 0x81;
    else if(type == EEIWS_OPCODE_BINDATA)
        *package++ = 0x82;
    else if(type == EEIWS_OPCODE_DISCONN)
        *package++ = 0x88;
    else if(type == EEIWS_OPCODE_PING)
        *package++ = 0x89;
    else if(type == EEIWS_OPCODE_PONG)
        *package++ = 0x8A;
    else
        return -1;
    //
    if(isMask)
        *package = 0x80;
    len += 1;
    //
    if(dataLen < 126)
    {
        *package++ |= (dataLen&0x7F);
        len += 1;
    }
    else if(dataLen < 65536)
    {
        if(packageMaxLen < 4)
            return -1;
        *package++ |= 0x7E;
        *package++ = (char)((dataLen >> 8) & 0xFF);
        *package++ = (unsigned char)((dataLen >> 0) & 0xFF);
        len += 3;
    }
    else if(dataLen < 0xFFFFFFFF)
    {
        if(packageMaxLen < 10)
            return -1;
        *package++ |= 0x7F;
        *package++ = 0; //(char)((dataLen >> 56) & 0xFF);   // 数据长度变量是 unsigned int dataLen, 暂时没有那么多数据
        *package++ = 0; //(char)((dataLen >> 48) & 0xFF);
        *package++ = 0; //(char)((dataLen >> 40) & 0xFF);
        *package++ = 0; //(char)((dataLen >> 32) & 0xFF);
        *package++ = (char)((dataLen >> 24) & 0xFF);        // 到这里就够传4GB数据了
        *package++ = (char)((dataLen >> 16) & 0xFF);
        *package++ = (char)((dataLen >> 8) & 0xFF);
        *package++ = (char)((dataLen >> 0) & 0xFF);
        len += 9;
    }
    //
    if(isMask)    // 数据使用掩码时, 使用异或解码, maskKey[4]依次和数据异或运算, 逻辑如下
    {
        if(packageMaxLen < len + dataLen + 4)
            return -1;
        generate_randomkey(maskKey, sizeof(maskKey));    // 随机生成掩码
        *package++ = maskKey[0];
        *package++ = maskKey[1];
        *package++ = maskKey[2];
        *package++ = maskKey[3];
        len += 4;
        for(i = 0, count = 0; i < dataLen; i++)
        {
            temp1 = maskKey[count];
            temp2 = data[i];
            *package++ = (char)(((~temp1)&temp2) | (temp1&(~temp2)));  // 异或运算后得到数据
            count += 1;
            if(count >= sizeof(maskKey))    // maskKey[4]循环使用
                count = 0;
        }
        len += i;
        *package = '\0';
    }
    else    // 数据没使用掩码, 直接复制数据段
    {
        if(packageMaxLen < len + dataLen)
            return -1;
        memcpy(package, data, dataLen);
        package[dataLen] = '\0';
        len += dataLen;
    }
    //
    return len;
}
int iWSSend(SSockClient_t *pstSock, const void *pvData, int iLen, EEIWS_OPCODE eOpcode)
{
    unsigned char *psSendData = (unsigned char *)malloc(iLen+128);  
    memset(psSendData, 0, (iLen + 128));
    int iRetLen = wss_encode_package((unsigned char*)pvData, iLen, psSendData, iLen + 128, 1, eOpcode);
    iRetLen = pstSock->iSend(pstSock, psSendData, iRetLen);
    free(psSendData);
    return iRetLen;
}

static int iWriteData(const char *pszData, size_t dwLen)
{
    int iRet;
    int iCode;
    char *szDesc;
    char *pszAudioItem;
    char *pszAudio = NULL;
    int iStatus = 0;
    // if (fp) fseek(fp, 0, SEEK_SET);
    JSON_SERIALIZE_START(json_root, pszData, iRet);
    JSON_SERIALIZE_GET_INT(json_root, "code", iCode, iRet, JSON_CTRL_BREAK);
    JSON_SERIALIZE_GET_STRING(json_root, "desc", szDesc, iRet, JSON_CTRL_NULL);
    JSON_SERIALIZE_GET_OBJECT(json_root, "data", json_data, iRet, JSON_CTRL_BREAK);
    JSON_SERIALIZE_GET_INT(json_data, "status", iStatus, iRet, JSON_CTRL_BREAK);
    JSON_SERIALIZE_GET_STRING(json_data, "audio", pszAudioItem, iRet, JSON_CTRL_BREAK);
    if (pszAudioItem)
    {
        // decode64 audio，追加写入文件
        FILE *fp = NULL;
        int iAudioLen = strlen(pszAudioItem);
        pszAudio = (char *)malloc(iAudioLen);
        memset(pszAudio, 0, iAudioLen);
        iAudioLen = iBase64Decode(pszAudioItem, pszAudio);
        fp = fopen("./tts.pcm", "ab");
        int iWriteLen = fwrite(pszAudio, 1, iAudioLen, fp);
        if (fp) fclose(fp);
        if (pszAudio)
            free(pszAudio);
    }
    JSON_SERIALIZE_END(json_root, iRet);
    
    return iStatus;
}

static char *g_szText = NULL;
bool bWSCallback(SSockClient_t *pstSock, EEIWS_MESSAGE eType, void *pvMessage, int iLen)
{
    bool bEnd = false;
    switch(eType) {
    case EEIWS_ON_HAND_SHAKE:       // websocket服务握手成功
        LOG(EDEBUG, "收到了101消息，并且验证key成功");
        char szText[1024] = {0};
        char szData[4096] = {0};
        const char *pszOrigin = g_szText;
        iBase64Encode(pszOrigin, szText, strlen(pszOrigin));
        sprintf(szData, "{\"common\":{\"app_id\":\"%s\"},\"business\":{\"aue\":\"raw\",\"auf\":\"audio/L16;rate=16000\",\"vcn\":\"xiaoyan\",\"tte\":\"utf8\",\"ent\":\"aisound\"},\"data\":{\"status\":2,\"text\":\"%s\"}}", APPID, szText);
        iWSSend(pstSock, szData, strlen(szData), EEIWS_OPCODE_TXTDATA);
        break;
    case EEIWS_ON_MESSAGE:          // 收到websocket消息
        // LOG(EDEBUG, "收到了消息：%s", pvMessage);
        {
            int iStatus;
            #if defined(_WIN32)
            iStatus = iWriteData(pvMessage, iLen);
            if (2 == iStatus) {
                bEnd = true;
            }
            #else
            // apt-get install alsa-utils alsa-tools alsa-tools-gui alsamixergui -y
            // 使用aplay播放，解决不同设备的兼容性问题，
            // 如果设备支持16000采样，可以采取tinyplay提供的实时流播放效果会更加及时
            iStatus = iWriteData(pvMessage, iLen);
            if (2 == iStatus) {
                bEnd = true;
            }
            #endif
        }
        break;
    case EEIWS_ON_CLOSE:            // 断开websocket连接
        LOG(EDEBUG, "收到了关闭消息");
        break;
    case EEIWS_ON_ERROR:            // websocket错误信息
        LOG(EDEBUG, "收到了错误消息");
        break;
    default:
        break;
    }
    return bEnd;
}

int WebSocketTTS(const char *c_pszText)
{
    // 1. 构造websocket auth字段
    char szDate[64], szAuth[4096];
    char *szBaseUrl = "ws://tts-api.xfyun.cn/v2/tts?authorization=%s&date=%s&host=%s";
    if (iGetGMTDate(szDate, sizeof(szDate))) {
        LOG(EERROR, "get date error");
        return -1;
    }
    iGetAuthUrl(TTS_URL, APP_KEY, APP_SECRET, szDate, szAuth, sizeof(szAuth));
    LOG(EDEBUG, "szAuth:%s", szAuth);

    // 2. 构造完整的URL
    int iEncodeLen;
    char szFullUrl[4096];
    char szDateEncode[64];
    urlencode(szDate, szDateEncode);
    snprintf(szFullUrl, sizeof(szFullUrl), szBaseUrl, szAuth, szDateEncode, "ws-api.xfyun.cn");
    printf("szFullUrl:%s\n", szFullUrl);

    // 3. 连接websocket服务器
    g_szText = (char *)c_pszText;
    bWebsocketConnect(szFullUrl, bWSCallback);

    return 0;
}
