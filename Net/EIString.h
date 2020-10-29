#ifndef _EISTRING_H_
#define _EISTRING_H_

#include <EIPlatform.h>
#include "map.h"
typedef struct EIString
{
    char *sBuffer;
    uint32_t lSize;
    bool bIsAlloc;
}EIString;
struct EIString* stEvIStringNew(uint32_t lSize);
void vEIStringDelete(struct EIString* pstString);
struct EIString* stEICharCopy(const void *pvBuffer, size_t lSize);
struct EIString* stEIStringCopy(const char *pszBuffer);
struct EIString* stEIStringDup(struct EIString* pstString);
struct EIString* stEIStringRealloc(struct EIString* pstString, const void *value, int iLen);

#endif