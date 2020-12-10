#ifndef __MD5_H
#define __MD5_H

/**
 * @brief md5加密
 * @param  initial_msg      加密消息串
 * @param  initial_len      加密消息串长度
 * @param  digest           加密后的内容，注意：长度固定16字节，在使用时一般会把每一个字节使用16进制转化为字符串
 */
void vMD5(const unsigned char *msg, unsigned int len, unsigned char *digest);

/**
 * @brief md5加密
 * @param  initial_msg      加密消息串
 * @param  initial_len      加密消息串长度
 * @param  digest           加密后的32字节小写字符串内容
 */
void md5String(const unsigned char *msg, unsigned int len, unsigned char *digest);

#endif
