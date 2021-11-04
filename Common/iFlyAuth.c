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
//     LOG(EDEBUG, "%s", szSignatureOrigin);
    hmac_sha256(szSignatureOrigin, strlen(szSignatureOrigin), pszAPPSecret, strlen(pszAPPSecret), szSignatureOriginSha);
//     LOG(EDEBUG, "%s\n", szSignatureOriginSha);
    iBase64Encode(szSignatureOriginSha, szSignatureOriginBase64, 32);
//     LOG(EDEBUG, "%s", szSignatureOriginBase64);
    snprintf(szAuthOri, 4096, "api_key=\"%s\", algorithm=\"hmac-sha256\", headers=\"host date request-line\", signature=\"%s\"",
            pszAPPKey, szSignatureOriginBase64);
//     LOG(EDEBUG, "%s", szAuthOri);
    iBase64Encode(szAuthOri, pAuthData, strlen(szAuthOri));
//     LOG(EDEBUG, "%s", pAuthData);
}

void vGetAuth2(const char *pszAPPKey, const char *pszAPPSecret, const char *pszHost, 
        const char *pszRequestPath, const char *pszReuqestLine, const char *pszAlgo,
        const char *pszGMTDate, const char *pszDigest, char *pAuthData, int iLen)
{
char szSignatureOrigin[1024];
    char szSignatureOriginSha[65] = {0};
    char szSignatureOriginBase64[1024];
    char szAuthOri[4096];
    snprintf(szSignatureOrigin, 1024, "host: %s\ndate: %s\n%s\ndigest: SHA-256=%s",
            pszHost, pszGMTDate, pszRequestPath, pszDigest);
    //LOG(ETRACE, "%s", szSignatureOrigin);
    hmac_sha256(szSignatureOrigin, strlen(szSignatureOrigin), pszAPPSecret, strlen(pszAPPSecret), szSignatureOriginSha);
    // LOG(ETRACE, "%s\n", szSignatureOriginSha);
    iBase64Encode(szSignatureOriginSha, szSignatureOriginBase64, 32);
    LOG(ETRACE, "%s", szSignatureOriginBase64);
    snprintf(pAuthData, iLen, "api_key=\"%s\", algorithm=\"%s\", headers=\"%s\", signature=\"%s\"",
            pszAPPKey, pszAlgo, pszReuqestLine, szSignatureOriginBase64);
    LOG(ETRACE, "%s", pAuthData);
//     iBase64Encode(szAuthOri, pAuthData, strlen(szAuthOri));
//     LOG(ETRACE, "%s", pAuthData);
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