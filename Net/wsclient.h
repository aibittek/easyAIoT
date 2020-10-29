#ifndef _WSCLIENT_H
#define _WSCLIENT_H

#include <stdint.h>
#include "EIHttpClient.h"

#define WEBSOCKET_SHAKE_KEY_LEN     16

bool bWSConnect(SEIHttpInfo_t *pstHttpInfo, fnWebsocketCallback cb);

#endif