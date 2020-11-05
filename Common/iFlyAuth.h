/**
 * @file iFlyAuth.h
 * @brief ifytek authorization
 * @author likui 55239610@qq.com
 * @version 1.0
 * @date 2020-11-03
 * 
 * @copyright MIT (c) 2020-2021 likui
 * 
 */

#ifndef __IFLY_AUTH_H
#define __IFLY_AUTH_H

// apikey
// api.xf-yun.com
// /v1/private/s67c9c78c
// Fri, 17 Jul 2020 06:26:58 GMT
/**
 * @brief 构造鉴权参数authorization字段，参考讯飞云相关文档
 * @param  pszAPIKey        用户的APPID
 * @param  pszBaseUrl       请求的URL基地址
 * @param  pszReuqestLine   请求request-line
 * @param  pszGMTDate       请求GMT时间戳
 * @param  pAuthData        获取到的Auth数据，如果失败数据内容长度为0
 * @param  iLen             pAuthData参数的数组长度
 */
void vGetAuth(const char *pszAPPKey, const char *pszAPPSecret, const char *pszBaseUrl, 
        const char *pszReuqestLine, const char *pszGMTDate, char *pAuthData, int iLen);

#endif