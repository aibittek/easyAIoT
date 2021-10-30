#include "FaceCompare.h"

int main()
{
    const char *appid = "60016549";
    const char *apisecret = "4f49ab1f8b4c430f3536e54d111eb705";
    const char *apikey = "69988d3cd6417055f6575c6e81eb2b5e";

    double dScore = fFaceCompare(appid, apisecret, apikey, "./1.jpg", "2.jpg");
    printf("score=%.2f\n", dScore);

    return 0;
}