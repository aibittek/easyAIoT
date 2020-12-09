#ifndef _WSCLIENT_H
#define _WSCLIENT_H

#include <stdint.h>
#include "EIHttpClient.h"

#define WEBSOCKET_SHAKE_KEY_LEN     16

bool bWSConnect(SEIHttpInfo_t *pstHttpInfo, fnWebsocketCallback cb, void *pvUserData);

/**
 * @brief 连接到websocket服务端
 * @param  c_pszUrl         websocket服务器的URL
 * @param  cb               websocket服务器的响应信息
 * @return true             连接成功
 * @return false            连接失败
 */
bool bWebsocketConnect(const char *c_pszUrl, fnWebsocketCallback cb, void *pvUserData);

#endif