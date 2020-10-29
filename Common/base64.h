#ifndef __BASE64_H
#define __BASE64_H

int iBase64Encode(const char *pcData, char *pcBase64, int pcDataLen);
int iBase64Decode(const char *base64, unsigned char *dedata);
#endif
