#include <EIPlatform.h>
#include <sha256.h>
#include <md5.h>
#include <base64.h>
#include <hmac_sha256.h>
#include <urlencode.h>
#include <EIString.h>
#include <ringbuffer.h>
#include <date.h>
#include <EISock.h>
#include <EIHttpClient.h>
#include <EILog.h>
#include <nlp.h>
#include <iat.h>
#include <FaceCompare.h>
#include <tts.h>
#include <AudioRecorder.h>

int main(int argc, char *argv[])
{
    // 修改为自己应用的appid，key和secret
    const char *appid = "xxxxxxxx";
    const char *key = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    const char *secret = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

    LOG(EDEBUG, "尝试说一些话，看看有什么有趣的事情会发生(●'◡'●), 其他有趣的应用参考Doc/readme.pdf文档");
    iat(appid, key, secret);

    return 0;
}