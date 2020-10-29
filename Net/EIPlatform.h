#ifndef _EIPLATFORM_H
#define _EIPLATFORM_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#if defined(_WIN32)
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#define GET_ERROR() GetLastError()
#define vsnprintf _vsnprintf
#define snprintf _snprintf
typedef SOCKET sock_t;
#elif defined(__unix__)
#include <stdarg.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
typedef int sock_t;
#define SOCKET_ERROR -1
#define INVALID_SOCKET	(-1)
#define closesocket(x) close(x)
#define GET_ERROR() errno
#else
#error "not support platform."
#endif
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define pvEIMalloc malloc
#define vEIFree free
#define pvEIRealloc realloc

#endif
