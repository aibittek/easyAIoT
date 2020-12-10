// hmac.h
#ifndef hmac_h
#define hmac_h

#include <stdint.h>
#include <stddef.h>

/**
 * @brief HamcSHA256加密
 * @param  data             加密数据
 * @param  len              加密数据长度
 * @param  key              加密key
 * @param  len_key          加密key长度
 * @param  out              加密内容，out长度大于等于32
 */
void hmac_sha256(const unsigned char *data, size_t len, const unsigned char *key, int len_key, unsigned char *out);

/**
 * @brief HamcSHA256加密
 * @param  data             加密数据
 * @param  len              加密数据长度
 * @param  key              加密key
 * @param  len_key          加密key长度
 * @param  out              加密内容，out长度大于等于65
 */
void hamcSha256String(const unsigned char *data, size_t len, const unsigned char *key, int len_key, unsigned char *out);

#endif /* hmac_h */
