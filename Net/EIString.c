#include "EIString.h"

struct EIString* stEvIStringNew(uint32_t lSize)
{
    struct EIString* pstString = (struct EIString*)pvEIMalloc(sizeof(struct EIString));
    if (!pstString) return NULL;
    
    char *psBuffer = pvEIMalloc(lSize+1);
    if (psBuffer) {
        pstString->sBuffer = psBuffer;
        pstString->lSize = lSize;
        pstString->bIsAlloc = true;
    } else {
        pstString->sBuffer = NULL;
        pstString->lSize = 0;
        pstString->bIsAlloc = false;
    }
    if (!pstString->sBuffer) {
        vEIFree(pstString);
        pstString = NULL;
    }
    return pstString;
}
void vEIStringDelete(struct EIString* pstString)
{
    if (pstString && pstString->bIsAlloc) {
        if (pstString->bIsAlloc) {
            vEIFree(pstString->sBuffer);
            pstString->sBuffer = NULL;
            pstString->lSize = 0;
            pstString->bIsAlloc = false;
        }
        vEIFree(pstString);
    }
}
struct EIString* stEICharCopy(const void *pvBuffer, size_t lSize)
{
    if (NULL == pvBuffer) return NULL;
    struct EIString* pstDstString = (struct EIString*)pvEIMalloc(sizeof(struct EIString));
    if (!pstDstString) return NULL;

    pstDstString->sBuffer = (char *)pvEIMalloc(lSize+1);
    if (pstDstString->sBuffer) {
        pstDstString->lSize = lSize;
        memcpy(pstDstString->sBuffer, pvBuffer, lSize);
        pstDstString->sBuffer[lSize] = '\0';
        pstDstString->bIsAlloc = true;
    } else {
        vEIFree(pstDstString);
        pstDstString = NULL;
    }
    return pstDstString;
}
struct EIString* stEIStringCopy(const char *pszBuffer)
{
    if (NULL == pszBuffer) return NULL;
    struct EIString* pstDstString = (struct EIString*)pvEIMalloc(sizeof(struct EIString));
    if (!pstDstString) return NULL;

    pstDstString->sBuffer = strdup(pszBuffer);
    if (pstDstString->sBuffer) {
        pstDstString->lSize = strlen(pszBuffer);
        pstDstString->bIsAlloc = true;
    } else {
        vEIFree(pstDstString);
        pstDstString = NULL;
    }
    return pstDstString;
}
struct EIString* stEIStringDup(struct EIString* pstSrcString)
{
    if (NULL == pstSrcString || NULL == pstSrcString->sBuffer) return NULL;
    struct EIString* pstDstString = (struct EIString*)pvEIMalloc(sizeof(struct EIString));
    if (!pstDstString) return NULL;

    pstDstString->sBuffer = strdup(pstSrcString->sBuffer);
    pstDstString->lSize = pstSrcString->lSize;
    pstDstString->bIsAlloc = true;

    return pstDstString;
}

struct EIString* stEIStringRealloc(struct EIString* pstString, const void *value, int iLen)
{
    if (NULL == pstString || NULL == pstString->sBuffer) return NULL;
    char *sNewBuffer = pvEIRealloc(pstString->sBuffer, iLen+pstString->lSize);
    if (sNewBuffer) {
        pstString->sBuffer = sNewBuffer;
        memcpy(pstString->sBuffer+pstString->lSize, value, iLen);
        pstString->lSize += iLen;
    }
    
    return pstString;
}