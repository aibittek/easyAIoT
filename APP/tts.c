#include <EILog.h>
#include <date.h>
#include <iFlyAuth.h>
#include <wsclient.h>
#include <cJSON_user_define.h>
#include <APPConfig.h>
#include <file.h>

#include "tts.h"

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
        if (fp)
            fclose(fp);
        if (pszAudio)
            free(pszAudio);
    }
    JSON_SERIALIZE_END(json_root, iRet);

    return iStatus;
}

static const void *g_total = NULL;
static bool bWSCallback(SSockClient_t *pstSock, EEIWS_MESSAGE eType, void *pvMessage, int iLen)
{
    bool bEnd = false;
    switch (eType)
    {
    case EEIWS_ON_HAND_SHAKE: // websocket服务握手成功
        LOG(EDEBUG, "收到了101消息，并且验证key成功");
        char szText[1024] = {0};
        char szData[4096] = {0};
        const char *pszOrigin = g_total;
        iBase64Encode(pszOrigin, szText, strlen(pszOrigin));
        sprintf(szData, "{\"common\":{\"app_id\":\"%s\"},\"business\":{\"aue\":\"raw\",\"auf\":\"audio/L16;rate=16000\",\"vcn\":\"xiaoyan\",\"tte\":\"utf8\",\"ent\":\"aisound\"},\"data\":{\"status\":2,\"text\":\"%s\"}}", appconfig.appid, szText);
        iWSSend(pstSock, szData, strlen(szData), EEIWS_OPCODE_TXTDATA);
        break;
    case EEIWS_ON_MESSAGE: // 收到websocket消息
        LOG(EDEBUG, "收到了消息：%s", pvMessage);
        {
            if (2 == iWriteData(pvMessage, iLen)) {
                bEnd = true;
            }
        }
        break;
    case EEIWS_ON_CLOSE: // 断开websocket连接
        LOG(EDEBUG, "收到了关闭消息");
        break;
    case EEIWS_ON_ERROR: // websocket错误信息
        LOG(EDEBUG, "收到了错误消息");
        break;
    default:
        break;
    }
    return bEnd;
}

cstring_t *getTTS(const char *appid, const char *key, const char *app_secret, const char *text)
{
    char szFullUrl[4096];
    char szAuth[1024], szDate[64];

    char *szBaseUrl = "ws://tts-api.xfyun.cn/v2/tts?authorization=%s&date=%s&host=%s";

    datetime.format("GMT", szDate, sizeof(szDate));
    vGetAuth(key, app_secret, "ws-api.xfyun.cn",
             "GET /v2/tts HTTP/1.1", szDate, szAuth, sizeof(szAuth));
    LOG(EDEBUG, "auth:%s\n", szAuth);

    // 2. 构造完整的URL
    snprintf(szFullUrl, sizeof(szFullUrl), szBaseUrl, szAuth, szDate, "ws-api.xfyun.cn");
    LOG(EDEBUG, "url:%s", szFullUrl);

    // 3. 连接websocket服务器
    g_total = (void *)text;
#if 0
    SEIHttpInfo_t stHttpInfo;
    bHttpOpen(&stHttpInfo, szFullUrl, NULL, NULL, 0);
    bWSConnect(&stHttpInfo, bWSCallback);

    bHttpClose(&stHttpInfo);
#else
    bWebsocketConnect(szFullUrl, bWSCallback);
#endif
    return NULL;
}

void vTestTTS()
{
    const char *appid = "5e5f1e5e";
    const char *app_secret = "3e45fc07dcdcf5306863c3321a4c9771";
    const char *app_key = "632bfb3990e4848cb8cb568182cda851";

    strcpy(appconfig.appid, appid);
    strcpy(appconfig.appkey, app_key);
    strcpy(appconfig.appsecret, app_secret);

    cstring_t *pcm = getTTS(appid, app_key, app_secret, "欢迎来到比特人生的世界");
    while (1)
        ;
}