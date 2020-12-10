#ifndef __BASE64_H
#define __BASE64_H

/**
 * @brief Base64编码
 * @param  pcData           需要编码的数据
 * @param  pcBase64         Base64编码后的数据
 * @param  pcDataLen        需要编码数据的长度
 * @return int 编码后数据的长度
 */
int iBase64Encode(const char *pcData, char *pcBase64, int pcDataLen);

/**
 * @brief Base64解码
 * @param  base64           需要解码的Base64字符串
 * @param  dedata           解码后的数据内容
 * @return int 解码后数据的长度
 */
int iBase64Decode(const char *base64, unsigned char *dedata);
#endif
