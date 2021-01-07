#include <its.h>

int main(int argc, char *argv[])
{
    const char *appid = "xxxxxxxx";
    const char *apikey = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    const char *apisecret = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

    getITSResult(appid, apikey, apisecret, "cn", "en", "欢迎来到比特人生的世界");
    return 0;
}