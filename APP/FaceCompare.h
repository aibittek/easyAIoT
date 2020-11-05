/**
 * @file faceCompare.h
 * @brief 
 * @author likui 55239610@qq.com
 * @version 1.0
 * @date 2020-11-03
 * 
 * @copyright MIT (c) 2020-2021 likui
 * 
 */
#ifndef __FACE_COMPARE_H
#define __FACE_COMPARE_H

/**
 * @brief iflytek face compare API
 * @param  pszAPPID         iflytek appid
 * @param  pszAPPSecret     iflytek appsecret
 * @param  pszAPPKey        iflytek appkey
 * @param  pszImagePath1    compare image path
 * @param  pszImagePath2    compare image path
 * @return float            score of similarity
 */
double fFaceCompare(const char *pszAPPID, 
        const char *pszAPPSecret, const char *pszAPPKey,
        const char *pszImagePath1, const char *pszImagePath2);

#endif