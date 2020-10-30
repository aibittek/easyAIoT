#include "EasyIoT.h"
#include "EIString.h"
#include "EISock.h"
#include "EIHttpClient.h"
#include "EILog.h"
#include "nlp.h"
#include "AudioRecorder.h"

#define APPID "5d2f27d2"
#define AIUI_API_KEY "a605c4712faefae730cc84b62c0eb92f"
#define AIUI_PARAM "{\"result_level\":\"plain\",\"auth_id\":\"27853aa9684eb19789b784a89ea5befd\",\"data_type\":\"audio\",\"sample_rate\":\"16000\",\"scene\":\"main_box\"}"

static EIString *pstGetFile(const char *pszFilename)
{
    bool bRet = false;
    struct EIString *pstBody = NULL;

    FILE* fp = fopen(pszFilename, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        int iSize = ftell(fp);
        pstBody = stEvIStringNew(iSize);
        if (pstBody->sBuffer) {
            int iRet = fread(pstBody->sBuffer, 1, iSize, fp);
            if (iRet == iSize) bRet = true;
        }
        fclose(fp);
    }
    return pstBody;
}

int iHttpTest2()
{
    char *pszUrl1 = "http://www.baidu.com:8080/ai/bi/ci/?param1=1&param2=2";
    char *pszUrl2 = "http://www.baidu.com/ai/bi/ci/?param1=1&param2=2";
    char *pszUrl3 = "http://www.baidu.com";

    // 3.连接云端
    SEIHttpInfo_t stHttpInfo;
    bConnectHttpServer(&stHttpInfo, pszUrl3, NULL, NULL, 0);
    LOG(EDEBUG, "status:%d", stHttpInfo.stResponse.iStatus);
    
    // 关闭HTTP
    bHttpClose(&stHttpInfo);

    // vEIStringDelete(pstExtraHeader);
    // vEIStringDelete(pstBody);
    return 0;
}

bool bSockCallback(struct SSockClient *pstClient, void *pvUserData, void *pvData, int iLen)
{
    printf("bSockCallback called\r\n");
    return true;
}
void iHttpTest1()
{
    int iLoop = 1;
    const char *header = "GET / HTTP/1.1\r\n"
        "Host: www.baidu.com\r\n"
        "Connection: keep-alive\r\n"
        "Accept: */*\r\n\r\n";

    static const char header2[] = 
    {
        "GET / HTTP/1.1\r\n\
        Host: www.baidu.com\r\n\
        Connection: keep-alive\r\n\
        User-Agent: AIRobotToy/1.0\r\n\
        Accept: */*\r\n\
        Accept-Encoding: gzip, deflate\r\n\r\n"
    };
    SSockClient_t stSockClient;
    bSockInit(&stSockClient);
    stSockClient.bCreate(&stSockClient, "14.215.177.39", 80);
    stSockClient.bConnect(&stSockClient);
    printf("header:%s\n", header);
    stSockClient.iSend(&stSockClient, (void *)header, strlen(header)+1);
    stSockClient.bEventLoop(&stSockClient, bSockCallback, NULL, &iLoop, 1000);
}

int main(int argc, char *argv[])
{
    extern void vAudioRecordTest();
    vAudioRecordTest();
    
    return 0;
}