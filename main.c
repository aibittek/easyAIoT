#include <BankCard.h>
#include <tts.h>
#include <EILog.h>
#include <IDCard.h>

int main(int argc, char *argv[])
{
    const char *appid = "5d2f27d2";
    const char *apikey = "1d41b77bf9baa26e885e95cddc2b5671";
    const char *appsecret = "8110566cd9dd13066f9a1e38aeb12a48";
    
    // 银行卡识别
    // char bankInfo[256] = {0};
    // const char *pathname = "../Res/bankcard.jpg";
    // BandCardRecognition(appid, apikey, pathname, bankInfo, sizeof(bankInfo));
    // LOG(EDEBUG, "%s\n", bankInfo);

    // 身份证识别
    IDCard_t idcard;
    getIDCard(appid, appsecret, apikey, "../Res/idcard.jpg", &idcard);
    LOG(EDEBUG, "address:%s", idcard.address);
    LOG(EDEBUG, "birthday:%s", idcard.birthday);
    LOG(EDEBUG, "id_number:%s", idcard.id_number);
    LOG(EDEBUG, "name:%s", idcard.name);
    LOG(EDEBUG, "people:%s", idcard.people);
    LOG(EDEBUG, "sex:%s", idcard.sex);
    LOG(EDEBUG, "type:%s", idcard.type);

    // getTTS(appid, apikey, appsecret, "一件20块，两件30块，多买一件少5块", pathname);

    // fFaceCompare(appid, appsecret, apikey, "../Res/1.jpg", "../Res/2.jpg");

    // 修改为自己的appid，key、secret和auth_id,网址https://aiui.xfyun.cn/app/<替换自己的appid>/info
    // const char *pszParam = "{\"result_level\":\"plain\",\"auth_id\":\"eac05cd035e40b0c4302673f2c2b2837\",\"data_type\":\"audio\",\"sample_rate\":\"16000\",\"scene\":\"main_box\"}";
    // // 1.读取语音文件
    // cstring_t *pAudioData = readFile("../Res/test.pcm");
    // if (!pAudioData)
    // {
    //     LOG(EERROR, "read audio file error");
    //     return;
    // }
    // LOG(EDEBUG, "pcm len:%d", pAudioData->length(pAudioData));
    // // 2.提供信息进行语音识别成文本
    // cstring_t *pResult = getNlpResult(appid, apikey, pszParam,
    //                                   pAudioData->str, pAudioData->len);
    // if (pResult)
    // {
    //     LOG(EDEBUG, "识别结果：%s", pResult->str);
    //     cstring_del(pResult);
    // }
    // cstring_del(pAudioData);
    return 0;
}