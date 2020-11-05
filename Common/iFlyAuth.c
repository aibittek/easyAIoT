#include <date.h>
#include <EILog.h>
#include <hmac_sha256.h>
#include <base64.h>

#include "iFlyAuth.h"

void vGetAuth(const char *pszAPPKey, const char *pszAPPSecret, const char *pszBaseUrl, 
        const char *pszReuqestLine, const char *pszGMTDate, char *pAuthData, int iLen)
{
    char szSignatureOrigin[1024];
    char szSignatureOriginSha[65] = {0};
    char szSignatureOriginBase64[1024];
    char szAuthOri[4096];
    snprintf(szSignatureOrigin, 1024, "host: %s\ndate: %s\n%s",
            pszBaseUrl, pszGMTDate, pszReuqestLine);
    LOG(EVERBOSE, "%s", szSignatureOrigin);
    hmac_sha256(szSignatureOrigin, strlen(szSignatureOrigin), pszAPPSecret, strlen(pszAPPSecret), szSignatureOriginSha);
    // LOG(EVERBOSE, "%s\n", szSignatureOriginSha);
    iBase64Encode(szSignatureOriginSha, szSignatureOriginBase64, 32);
    LOG(EVERBOSE, "%s", szSignatureOriginBase64);
    snprintf(szAuthOri, 4096, "api_key=\"%s\", algorithm=\"hmac-sha256\", headers=\"host date request-line\", signature=\"%s\"",
            pszAPPKey, szSignatureOriginBase64);
    LOG(EVERBOSE, "%s", szAuthOri);
    iBase64Encode(szAuthOri, pAuthData, strlen(szAuthOri));
    LOG(EVERBOSE, "%s", pAuthData);
}

void testGetAuth()
{
    const char *appid = "5e5f1e5e";
    const char *app_secret = "3e45fc07dcdcf5306863c3321a4c9771";
    const char *app_key = "632bfb3990e4848cb8cb568182cda851";

    char szDate[64];
    char szAuthData[512];

    datetime.format("GMT", szDate, sizeof(szDate));
    vGetAuth(app_key, app_secret, "api.xf-yun.com", 
        "POST /v1/private/s67c9c78c HTTP/1.1", szDate, 
        szAuthData, sizeof(szAuthData));
}