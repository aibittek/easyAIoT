#include "EIPlatform.h"
#include "EIString.h"
#include "EISock.h"
#include "EILog.h"
#include <urlencode.h>
#include <cstring.h>
#include "EIHttpClient.h"

bool bHttpUrlParse(const char *pszUrl, EIString** pstHost, EIString** pstPath, uint16_t* pnPort)
{
    if (NULL == pszUrl || NULL == pstHost || NULL == pstPath || NULL == pnPort) return false;

    bool nRet = false;
    char *pszUrlDup = strdup(pszUrl);
    char *pszStart = pszUrlDup;
    char *pszHost = 0, *pszPath = 0, *pszPathDup = 0, *pszPort = 0;
    int16_t nPort = -1;

    do {
        if (0 == strncmp(pszUrlDup, "http://", 7)) {
            pszStart = pszUrlDup + 7;
        } else if (0 == strncmp(pszUrlDup, "ws://", 5)) {
            pszStart = pszUrlDup + 5;
        } else {
            break;
        }
        pszPath = strchr(pszStart, '/');
        if (pszPath) {
            pszPathDup = strdup(pszPath);
            pszPath[0] = '\0';
        } else {
            pszPathDup = strdup("/");
        }
        pszPort = strchr(pszStart, ':');
        if (pszPort) {
            nPort = atoi(pszPort+1);
            pszPort[0] = '\0';
        } else {
            nPort = 80;
        }
        pszHost = strdup(pszStart);
        *pstHost = stEIStringCopy(pszHost);
        cstring_new_len(str, strlen(pszPathDup)*2);
        urlencode(pszPathDup, str->str);
        *pstPath = stEIStringCopy(str->str);
        cstring_del(str);
        *pnPort = nPort;

        nRet = true;
    } while(0);

    // LOG(EDEBUG, "%s,%d,%s\n", pszHost, nPort, pszPathDup);

    if (pszUrlDup) free(pszUrlDup);
    if (pszHost) free(pszHost);
    if (pszPathDup) free(pszPathDup);

    return nRet;
}

static bool bHostResolve(const char *c_pszhost, struct in_addr *pstAddr)
{
    struct hostent *he;
    if ((he = gethostbyname(c_pszhost)) == NULL) {
        LOG(EDEBUG, "gethostbyname(%s) failed: %s", c_pszhost, strerror(GET_ERROR()));
    } else {
        memcpy(pstAddr, he->h_addr_list[0], sizeof(*pstAddr));
        return false;
    }
    return true;
}
bool bHttpOpen(SEIHttpInfo_t *pstHttpInfo, const char *pszUrl, const char *pszExtraHeader, void *pvBody, int32_t iSize)
{
    memset(pstHttpInfo, 0, sizeof(SEIHttpInfo_t));
    
    // 解析URL
    if (!bHttpUrlParse(pszUrl, 
            &pstHttpInfo->stRequest.pstHost, 
            &pstHttpInfo->stRequest.pstPath, 
            &pstHttpInfo->stRequest.nPort)) {
        LOG(EDEBUG, "host:%s,path:%s,port:%d", 
            pstHttpInfo->stRequest.pstHost->sBuffer, 
            pstHttpInfo->stRequest.pstPath->sBuffer, 
            pstHttpInfo->stRequest.nPort);
        return false;
    }

    // 初始化网络
    SSockClient_t *pstSockClient = &pstHttpInfo->stSockClient;
    bSockInit(pstSockClient);
    pstSockClient->nServerPort = pstHttpInfo->stRequest.nPort;
    bHostResolve(pstHttpInfo->stRequest.pstHost->sBuffer, &pstSockClient->iServerIp);
    pstSockClient->bCreate(pstSockClient, inet_ntoa(pstSockClient->iServerIp), pstHttpInfo->stRequest.nPort);

    // 构造HTTP请求内容
    SEIHttpRequest_t *pstRequest = &pstHttpInfo->stRequest;
    if (NULL == pvBody || iSize <= 0) {
        strcpy(pstRequest->szMethod, "GET");
    }
    else {
        strcpy(pstRequest->szMethod, "POST");
        pstRequest->pstExtraHeader = stEIStringCopy(pszExtraHeader);
        pstRequest->pstBody = stEICharCopy(pvBody, iSize);
        pstRequest->content_length = iSize;
    }

    return true;
}

int http_header(map_void_t *pmap, char *pszString, bool bNorFirst)
{
    char *key, *value;
    const char *s = NULL;
    int  len;

    if (bNorFirst) {
        s = ":";
    } else {
        s = " ";
    }

    /* 获取第一个子字符串 */
    key = strtok(pszString, s);
    /* 继续获取其他的子字符串 */
    if (key) {
        value = strtok(NULL, s);
        if (bNorFirst) value += 1;
        EIString **ppstValue = (EIString **)map_get(pmap, key);
        if (ppstValue && *ppstValue) {
            stEIStringRealloc(*ppstValue, ", ", 2);
            stEIStringRealloc(*ppstValue, value, strlen(value));
        } else {
            EIString *pstValue = stEIStringCopy(value);
            map_set(pmap, key, pstValue);
        }
    }
    // LOG(EDEBUG, "key:%s, value:#%s#", key, value);

    return 0;
}

bool bDefaultSockCallback(struct SSockClient *pstClient, void *pvUserData, void *pvData, int iLen)
{
    int iParseLen;
    char *pPrev, *pNext;
    SEIHttpInfo_t *pstHttpInfo = pvUserData;
    SEIHttpParser_t *pstHttpParser = &pstHttpInfo->stHttpParser;
    SEIHttpResponse_t *pstResponse = &pstHttpInfo->stResponse;
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
        if (!pstHttpParser->bHeadEnd) { // 解析http头部
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
                    // pstHttpParser->iTotalLen -= (pPrev-(char *)pstHttpParser->sRecvBuffer);
                    // strncpy(pstHttpParser->sRecvBuffer, pPrev, pstHttpParser->iTotalLen);
                    // pstHttpParser->iCurLen = 0;
                    // pPrev = pstHttpParser->sRecvBuffer;
                    EIString **ppstValue = (EIString **)map_get(&pstResponse->mapResponse, "Transfer-Encoding");
                    if (ppstValue && *ppstValue && (*ppstValue)->sBuffer) {
                        if (!strcmp((*ppstValue)->sBuffer, "chunked")) {
                            pstHttpParser->bTrunked = true;
                            continue;
                        }
                    }
                    ppstValue = (EIString **)map_get(&pstResponse->mapResponse, "Content-Length");
                    if (ppstValue && *ppstValue && (*ppstValue)->sBuffer) {
                        pstHttpParser->iContentLength = atoi((*ppstValue)->sBuffer);
                    }
                }
            } else {
                pstHttpParser->iCurLen -= (pPrev-(char *)pstHttpParser->sRecvBuffer);
                strncpy(pstHttpParser->sRecvBuffer, pPrev, pstHttpParser->iTotalLen);
                break;
            }
        } else if (!pstHttpParser->bBodyEnd) { // 解析body部分
            if (!pstResponse->pstBody) {
                pstResponse->pstBody = stEvIStringNew(0);
            }
            // 使用Content-Length字段解析body部分
            if (pstHttpParser->iContentLength > 0) {
                if (pstHttpParser->iLength < pstHttpParser->iContentLength) {
                    int iLen = pstHttpParser->iTotalLen - pstHttpParser->iCurLen;
                    stEIStringRealloc(pstResponse->pstBody, 
                        pstHttpParser->sRecvBuffer+pstHttpParser->iCurLen, iLen);
                    pstHttpParser->iLength += iLen;
                    pstHttpParser->iCurLen = 0;
                } 
                if (pstHttpParser->iLength >= pstHttpParser->iContentLength) {
                    pstHttpParser->bBodyEnd = true;
                    continue;
                }
                else {
                    break;
                }
            }
            // 使用Trunked字段解析body部分
            else {
                if (!pstHttpParser->bIsTrunkLength) {
                    if ((pNext = strstr(pPrev, "\r\n")) != NULL) {
                        iParseLen = pNext - pPrev;
                        *pNext = 0;
                        pstHttpParser->iLength = strtol(pPrev, NULL, 16) + 2;
                        pPrev = pNext + 2;
                        // LOG(EDEBUG, "length:%d", pstHttpParser->iLength);
                        pstHttpParser->iCurLen = pPrev - (char *)pstHttpParser->sRecvBuffer;
                        pstHttpParser->bIsTrunkLength = true;
                        if (2 == pstHttpParser->iLength) {
                            pstHttpParser->bBodyEnd = true;
                            continue;
                        }
                    } else {
                        // 说明解析数据不够，需要重新读取进来
                        int iLeftLen = pstHttpParser->iTotalLen - pstHttpParser->iCurLen;
                        if (iLeftLen <= 0) break;
                        strncpy(pstHttpParser->sRecvBuffer, pPrev, iLeftLen);
                        pstHttpParser->iCurLen = iLeftLen;
                        break;
                    }
                }
                if (pstHttpParser->bIsTrunkLength) {
                    if (pstHttpParser->iTotalLen - pstHttpParser->iCurLen >= pstHttpParser->iLength) {
                        stEIStringRealloc(pstResponse->pstBody, 
                            pstHttpParser->sRecvBuffer+pstHttpParser->iCurLen, 
                            pstHttpParser->iLength);
                        pstHttpParser->bIsTrunkLength = false;
                        pstHttpParser->iCurLen += pstHttpParser->iLength;
                        pPrev = (char *)pstHttpParser->sRecvBuffer + pstHttpParser->iCurLen;
                        pstHttpParser->iLength = 0;
                        if (pstHttpParser->iCurLen >= pstHttpParser->iTotalLen) {
                            pstHttpParser->iCurLen = 0;
                            break;
                        }
                        continue;
                    } else {
                        stEIStringRealloc(pstResponse->pstBody, 
                            pstHttpParser->sRecvBuffer+pstHttpParser->iCurLen, 
                            pstHttpParser->iTotalLen - pstHttpParser->iCurLen);
                        pstHttpParser->iLength -= pstHttpParser->iTotalLen - pstHttpParser->iCurLen;
                        pstHttpParser->iCurLen = 0;
                        pstHttpParser->iTotalLen = 0;
                        break;
                    }
                }
            }
        } else { // 解析结束
            LOG(ETRACE, "http解析结束");
            return true;
        }
    }
    
    return false;
}

EIString *EISnprintf(const char *fmt, ...)
{
    int iLen, iSize = 0, iStep = 128;
    va_list ap;
    EIString *pstString = NULL;
    
    iSize += iStep;
    pstString = stEvIStringNew(iSize+1);
    va_start(ap, fmt);
    iLen = vsnprintf(pstString->sBuffer, iSize, fmt, ap);
    va_end(ap);
    if (iLen < 0) {
        while(iLen < 0) {
            if (pstString) vEIStringDelete(pstString);
            pstString = NULL;
            iSize += iStep;
            pstString = stEvIStringNew(iSize+1);
            va_start(ap, fmt);
            iLen = vsnprintf(pstString->sBuffer, iSize+1, fmt, ap);
            va_end(ap);
        }
    } else {
        if (iLen > iStep) {
            if (pstString) vEIStringDelete(pstString);
            iSize = iLen;
            pstString = stEvIStringNew(iSize+1);
            va_start(ap, fmt);
            iLen = vsnprintf(pstString->sBuffer, iSize+1, fmt, ap);
            va_end(ap);
        }
    }

    return pstString;
}
bool bHttpConnect(SEIHttpInfo_t *pstHttpInfo, fnSockCallback cb)
{
    char buf[1024];
    char *pBuffer = buf;
    bool bBody = false;
    int iLoop = 1;
    EIString *pstHeader = NULL;
    int iSize = 0;
    SSockClient_t *pstSockClient = &pstHttpInfo->stSockClient;
    SEIHttpRequest_t *pstRequest = &pstHttpInfo->stRequest;
    pstSockClient->bConnect(pstSockClient);
    if (!strcmp(pstHttpInfo->stRequest.szMethod, "GET")) {
        pstHeader = EISnprintf("GET %.*s HTTP/1.1\r\n"
            "Host: %.*s\r\n"
            "Accept: */*\r\n"
            "User-Agent: kuili/1.0\r\n"
            "Connection: keep-alive\r\n",
            strlen(pstRequest->pstPath->sBuffer), pstRequest->pstPath->sBuffer, 
            strlen(pstRequest->pstHost->sBuffer), pstRequest->pstHost->sBuffer);
    } else if (!strcmp(pstHttpInfo->stRequest.szMethod, "POST")) {
        pstHeader = EISnprintf("POST %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Accept: */*\r\n"
            "User-Agent: kuili/1.0\r\n"
            "Connection: keep-alive\r\n"
            "Content-Length: %d\r\n", 
            pstRequest->pstPath->sBuffer, pstRequest->pstHost->sBuffer, pstRequest->pstBody->lSize);
        bBody = true;
    }
    // LOG(EDEBUG, "header:#%s#", pstHeader->sBuffer);
    pstSockClient->iSend(pstSockClient, pstHeader->sBuffer, strlen(pstHeader->sBuffer));

    // 发送额外头信息
    if (pstRequest->pstExtraHeader && pstRequest->pstExtraHeader->sBuffer) {
        pstSockClient->iSend(pstSockClient, 
            pstRequest->pstExtraHeader->sBuffer, 
            pstRequest->pstExtraHeader->lSize);
    }

    // 发送header和body分隔符
    pstSockClient->iSend(pstSockClient, "\r\n", strlen("\r\n"));
    
    if (bBody) {
        pstSockClient->iSend(pstSockClient, 
                pstRequest->pstBody->sBuffer, pstRequest->pstBody->lSize);
    }
    pstSockClient->bEventLoop(pstSockClient, cb, pstHttpInfo, &iLoop, 1000);
    if (pstHeader) vEIStringDelete(pstHeader);
    return true;
}

bool bHttpClose(SEIHttpInfo_t *pstHttpInfo)
{
    int i;
    SEIHttpResponse_t *pstResponse = &pstHttpInfo->stResponse;
    SSockClient_t *pstSockClient = &pstHttpInfo->stSockClient;
    if (pstSockClient) {
        pstSockClient->vClose(pstSockClient);
    }
    if (pstResponse->pstBody) vEIStringDelete(pstResponse->pstBody);
    if (pstResponse->bMapInit) {
        const char *key;
        map_void_t *m = &pstResponse->mapResponse;
        map_iter_t iter = map_iter(m);
        while ((key = map_next(m, &iter))) {
            EIString *pstValue = *map_get(m, key);
            // LOG(EDEBUG, "key:%s -> value:%s", key, pstValue->sBuffer);
            vEIStringDelete(pstValue);
        }
        map_deinit(m);
    }
    return true;
}

bool bConnectHttpServer(SEIHttpInfo_t *pstHttpInfo, const char *pszUrl, const char *pszExtraHeader, void* pvBody, int iSize)
{
    bool bRet = false;
    EIString *pstHost = NULL;
    EIString *pstPath = NULL;
    int16_t nPort;

    do {
        // 打开HTTP
        if (!bHttpOpen(pstHttpInfo, pszUrl, pszExtraHeader, pvBody, iSize))
            break;
        // 连接HTTP
        if (!bHttpConnect(pstHttpInfo, bDefaultSockCallback))
            break;
        // LOG(EDEBUG, "%s", pstHttpInfo->stResponse.pstBody);
        bRet = true;
    } while(0);

    // SEIHttpResponse_t *pstResponse = &pstHttpInfo->stResponse;
    // LOG(EDEBUG, "body:%s", pstResponse->pstBody->sBuffer);

    return bRet;
}