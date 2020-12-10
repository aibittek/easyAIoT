#include <date.h>
#include <EILog.h>
#include <cstring.h>
#include <wsclient.h>
#include <SimpleThread.h>
#include <AudioRecorder.h>
#include <iFlyAuth.h>
#include <base64.h>
#include <cJSON_user_define.h>
#include <APPConfig.h>

#include "iat.h"

#define IAT_PARAM "{\"result_level\":\"plain\",\"auth_id\":\"27853aa9684eb19789b784a89ea5befd\",\"data_type\":\"audio\",\"sample_rate\":\"16000\",\"scene\":\"main_box\"}"
#define IAT_URL "http://iat-api.xfyun.cn"

typedef enum EFRAME_TYPE
{
    EFRAME_FIRST = 0,
    EFRAME_CONTINE,
    EFRAME_LAST
} EFRAME_TYPE;

static void vFirstRequest(const char *pAudio, char *pszRequest, int iLen)
{
    JSON_SERIALIZE_CREATE_OBJECT_START(json_common_obj);
    JSON_SERIALIZE_ADD_STRING_TO_OBJECT(json_common_obj, "app_id", appconfig.appid);
    // JSON_SERIALIZE_CREATE_END(json_common_obj);

    JSON_SERIALIZE_CREATE_OBJECT_START(json_business_obj);
    JSON_SERIALIZE_ADD_STRING_TO_OBJECT(json_business_obj, "language", "zh_cn");
    JSON_SERIALIZE_ADD_STRING_TO_OBJECT(json_business_obj, "domain", "iat");
    JSON_SERIALIZE_ADD_STRING_TO_OBJECT(json_business_obj, "accent", "mandarin");
    // JSON_SERIALIZE_ADD_STRING_TO_OBJECT(json_business_obj, "dwa", "wpgs");
    // JSON_SERIALIZE_CREATE_END(json_business_obj);

    JSON_SERIALIZE_CREATE_OBJECT_START(json_data_obj);
    JSON_SERIALIZE_ADD_INT_TO_OBJECT(json_data_obj, "status", EFRAME_FIRST);
    JSON_SERIALIZE_ADD_STRING_TO_OBJECT(json_data_obj, "format", "audio/L16;rate=16000");
    JSON_SERIALIZE_ADD_STRING_TO_OBJECT(json_data_obj, "encoding", "raw");
    JSON_SERIALIZE_ADD_STRING_TO_OBJECT(json_data_obj, "audio", pAudio);
    // JSON_SERIALIZE_CREATE_END(json_data_obj);

    JSON_SERIALIZE_CREATE_OBJECT_START(json_frame_obj);
    JSON_SERIALIZE_ADD_OBJECT_TO_OBJECT(json_frame_obj, "common", json_common_obj);
    JSON_SERIALIZE_ADD_OBJECT_TO_OBJECT(json_frame_obj, "business", json_business_obj);
    JSON_SERIALIZE_ADD_OBJECT_TO_OBJECT(json_frame_obj, "data", json_data_obj);
    JSON_SERIALIZE_STRING(json_frame_obj, pszRequest, iLen);
    JSON_SERIALIZE_CREATE_END(json_frame_obj);
}

static void vContinueRequest(const char *pAudio, char *pszRequest, int iLen)
{
    // 构造data中的字段
    JSON_SERIALIZE_CREATE_OBJECT_START(json_data_obj);
    JSON_SERIALIZE_ADD_INT_TO_OBJECT(json_data_obj, "status", EFRAME_CONTINE);
    JSON_SERIALIZE_ADD_STRING_TO_OBJECT(json_data_obj, "format", "audio/L16;rate=16000");
    JSON_SERIALIZE_ADD_STRING_TO_OBJECT(json_data_obj, "encoding", "raw");
    JSON_SERIALIZE_ADD_STRING_TO_OBJECT(json_data_obj, "audio", pAudio);

    // 把以上构造的对象json_data_obj放到data所表述的json_frame_obj中
    JSON_SERIALIZE_CREATE_OBJECT_START(json_frame_obj);
    JSON_SERIALIZE_ADD_OBJECT_TO_OBJECT(json_frame_obj, "data", json_data_obj);
    // 把json对象序列化到pszRequest数组中，用于后续的发送
    JSON_SERIALIZE_STRING(json_frame_obj, pszRequest, iLen);
    JSON_SERIALIZE_CREATE_END(json_frame_obj);
}

static void vLastRequest(const char *pAudio, char *pszRequest, int iLen)
{
    JSON_SERIALIZE_CREATE_OBJECT_START(json_data_obj);
    JSON_SERIALIZE_ADD_INT_TO_OBJECT(json_data_obj, "status", EFRAME_LAST);
    JSON_SERIALIZE_ADD_STRING_TO_OBJECT(json_data_obj, "format", "audio/L16;rate=16000");
    JSON_SERIALIZE_ADD_STRING_TO_OBJECT(json_data_obj, "encoding", "raw");
    JSON_SERIALIZE_ADD_STRING_TO_OBJECT(json_data_obj, "audio", pAudio);
    // JSON_SERIALIZE_CREATE_END(json_data_obj);

    JSON_SERIALIZE_CREATE_OBJECT_START(json_frame_obj);
    JSON_SERIALIZE_ADD_OBJECT_TO_OBJECT(json_frame_obj, "data", json_data_obj);
    JSON_SERIALIZE_STRING(json_frame_obj, pszRequest, iLen);
    JSON_SERIALIZE_CREATE_END(json_frame_obj);
}

static int iGetResponse(const char *pszData, int iSize)
{
    int ret, code;
    char message[1024];
    char sid[1024];
    int status = 0;
    char w[128];
    JSON_DESERIALIZE_START(json_root, pszData, ret);
    JSON_DESERIALIZE_GET_INT(json_root, "code", code, ret, JSON_CTRL_BREAK);
    JSON_DESERIALIZE_GET_STRING_COPY(json_root, "message", message, sizeof(message), ret, JSON_CTRL_NULL);
    JSON_DESERIALIZE_GET_STRING_COPY(json_root, "sid", sid, sizeof(sid), ret, JSON_CTRL_NULL);
    // 1. 解析data对象，不存在就break出去，不再解析
    JSON_DESERIALIZE_GET_OBJECT(json_root, "data", json_data, ret, JSON_CTRL_BREAK);
    JSON_DESERIALIZE_GET_INT(json_data, "status", status, ret, JSON_CTRL_BREAK);
    // 2. 解析data中的result对象，不存在就break出去，不再解析
    JSON_DESERIALIZE_GET_OBJECT(json_data, "result", json_result, ret, JSON_CTRL_BREAK);
    // 3. 解析result中的ws数组，不存在就break出去
    JSON_DESERIALIZE_GET_ARRAY(json_result, "ws", json_array, ret, JSON_CTRL_BREAK);
    JSON_DESERIALIZE_ARRAY_FOR_EACH_START(json_array, sub_item, pos, total);
    // 4. 解析ws数组中的cw数组，不存在就break出去
    JSON_DESERIALIZE_GET_ARRAY(sub_item, "cw", cw_item, ret, JSON_CTRL_BREAK);
    JSON_DESERIALIZE_ARRAY_FOR_EACH_START(cw_item, cw_sub_item, pos, total);
    // 5. 遍历cw数组,找到w字符串
    JSON_DESERIALIZE_GET_STRING_COPY(cw_sub_item, "w", w, sizeof(w), ret, JSON_CTRL_NULL);
    // 6. 获得实时转写的结果，直接打印出来
    fprintf(stderr, "%s", w);
    JSON_DESERIALIZE_ARRAY_FOR_EACH_END();
    JSON_DESERIALIZE_ARRAY_FOR_EACH_END();
    JSON_DESERIALIZE_END(json_root, ret);

    if (ret < 0) return ret;
    return status;
}

static void RecordCB(void *pvHandle, int32_t iType, void *pvUserData, void *pvData, int32_t iLen)
{
    int iRet;
    static char sBase64Buffer[8192] = {0};
    static char szRequest[8192] = {0};

    memset(sBase64Buffer, 0, sizeof(sBase64Buffer));
    memset(szRequest, 0, sizeof(szRequest));

    SSockClient_t *pstSock = (SSockClient_t *)pvUserData;
    if (!pstSock)
        return;
    static int iStatus = EFRAME_FIRST;
    switch (iType)
    {
    case AUDIO_OPEN:
        LOG(EDEBUG, "record device open success");
        break;
    case AUDIO_DATA:
        iBase64Encode(pvData, sBase64Buffer, iLen);
        // LOG(EDEBUG, "get audio data, len:%d,%s", iLen, sBase64Buffer);
        switch (iStatus)
        {
        case EFRAME_FIRST:
            vFirstRequest(sBase64Buffer, szRequest, sizeof(szRequest) - 1);
            iStatus = EFRAME_CONTINE;
            break;
        case EFRAME_CONTINE:
            vContinueRequest(sBase64Buffer, szRequest, sizeof(szRequest) - 1);
            break;
        case EFRAME_LAST:
            vLastRequest(sBase64Buffer, szRequest, sizeof(szRequest) - 1);
            break;
        default:
            break;
        }
        iWSSend(pstSock, szRequest, strlen(szRequest), EEIWS_OPCODE_TXTDATA);
        break;
    case AUDIO_CLOSE:
        Recorder.stop();
        break;
    default:
        break;
    }
}

static void *pvRealRecordThread(void *params)
{
    // 录音开始，录音数据存放在ringbuffer，这些操作在回调函数RecordCB完成
    AudioConfig_t stAudioConfig = {16000, 16, 1, RecordCB, (void *)params};
    Recorder.open(&stAudioConfig);
    Recorder.start();

    // 释放资源
    Recorder.close();
    LOG(EDEBUG, "record thread exit");
    return NULL;
}

static bool bWSCallback(SSockClient_t *pstSock, void* pUserData, EEIWS_MESSAGE eType, void *pvMessage, int iLen)
{
    int iRet = 0;
    bool bEnd = false;
    switch (eType)
    {
    case EEIWS_ON_HAND_SHAKE: // 收到了101消息，websocket服务握手成功
        LOG(EDEBUG, "websocket handshake success");
        ThreadFun funArr[1];
        funArr[0].fun = pvRealRecordThread;
        funArr[0].params = (void *)pstSock;
        vStartThread(1, funArr);

        break;
    case EEIWS_ON_MESSAGE: // 收到websocket消息
        iRet = iGetResponse(pvMessage, iLen);
        if (2 == iRet) {
            // LOG(EDEBUG, "response finish");
            // Recorder.close();
            // bEnd = true;
        } else if (iRet < 0) {
            LOG(EDEBUG, "len:%d, recv:%s", iLen, pvMessage);
            // Recorder.close();
            // bEnd = true;
        } else {
            // LOG(EDEBUG, "msg:%s", pvMessage);
        }
        break;
    case EEIWS_ON_CLOSE: // 断开websocket连接
        LOG(EDEBUG, "websocket close");
        break;
    case EEIWS_ON_ERROR: // websocket错误信息
        LOG(EDEBUG, "websocket error");
        break;
    default:
        break;
    }
    return bEnd;
}

void iat(const char *appid, const char *key, const char *secret)
{
    char szFullUrl[4096];
    char szAuth[1024], szDate[64];
    char *szBaseUrl = "ws://iat-api.xfyun.cn/v2/iat?host=%s&date=%s&authorization=%s";

    strcpy(appconfig.appid, appid);
    strcpy(appconfig.appkey, key);
    strcpy(appconfig.appsecret, secret);

    // 1. 构造websocket auth字段
    datetime.format("GMT", szDate, sizeof(szDate));
    vGetAuth(key, secret, "iat-api.xfyun.cn",
             "GET /v2/iat HTTP/1.1", szDate, szAuth, sizeof(szAuth));
    LOG(ETRACE, "auth:%s\n", szAuth);

    // 2. 构造完整的URL
    snprintf(szFullUrl, sizeof(szFullUrl), szBaseUrl, "iat-api.xfyun.cn", szDate, szAuth);
    LOG(ETRACE, "url:%s", szFullUrl);

    // 3. 连接服务器
    bWebsocketConnect(szFullUrl, bWSCallback, NULL);
}
