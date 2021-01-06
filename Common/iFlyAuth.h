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

/**
 * @brief 构造鉴权参数authorization字段，参考讯飞云相关文档
 * @param  pszAPPKey        用户的APPID
 * @param  pszAPPSecret     用户的蜜月
 * @param  pszBaseUrl       请求Host
 * @param  pszReuqestLine   请求行
 * @param  pszGMTDate       当前GMT时间
 * @param  pAuthData        获取到的Auth数据，如果失败数据内容长度为0
 * @param  iLen             pAuthData参数的数组长度
 */
void vGetAuth(const char *pszAPPKey, const char *pszAPPSecret, const char *pszBaseUrl, 
        const char *pszReuqestLine, const char *pszGMTDate, char *pAuthData, int iLen);

/**
 * @brief 构造鉴权参数authorization字段，参考讯飞云相关文档
 * @param  pszAPPKey        用户的APPID
 * @param  pszAPPSecret     用户的蜜月
 * @param  pszHost          请求Host
 * @param  pszRequestPath   请求路径
 * @param  pszReuqestLine   请求行
 * @param  pszAlgo          请求算法
 * @param  pszGMTDate       当前GMT时间
 * @param  pAuthData        获取到的Auth数据，如果失败数据内容长度为0
 * @param  iLen             pAuthData参数的数组长度
 */
void vGetAuth2(const char *pszAPPKey, const char *pszAPPSecret, const char *pszHost, 
        const char *pszRequestPath, const char *pszReuqestLine, const char *pszAlgo,
        const char *pszGMTDate, const char *pszDigest, char *pAuthData, int iLen);
#endif