#ifndef _TINYCAP_H
#define _TINYCAP_H

typedef int (*capture_callback)(void *pvData, int iLen);
void vStartCapture(capture_callback cb);

#endif
