#ifndef __COMMON_H
#define __COMMON_H

#include <Windows.h>
#include <stdio.h>
#include <string.h>

#define SUCCESS 0
#define MY_ERROR -1

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#ifdef _DEBUG
#define debug_log(msg) do{printf("%s,%s() : %s\n", __FILENAME__, __func__, msg);}while(0)
#else
#define debug_log
#endif

#endif