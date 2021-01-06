#include <itr.h>

int main(int argc, char *argv[])
{
    const char *appid = "5d2f27d2";
    const char *apikey = "a8331910d59d41deea317a3c76d47b60";
    const char *apisecret = "8110566cd9dd13066f9a1e38aeb12a48";

    getITRResult(appid, apikey, apisecret, "../Res/itr.png");
    return 0;
}