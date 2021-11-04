#ifndef _SIMPLE_THREAD_H
#define _SIMPLE_THREAD_H

#ifdef _WIN32  
	#include <windows.h>  
	#include <direct.h>  
	#include <io.h>  
	#include <process.h>
#else  
	#include <unistd.h>  
	#include <getopt.h>  
	#include <sys/types.h>  
	#include <pthread.h>
#endif

typedef struct ThreadFun ThreadFun;
struct ThreadFun{
	void* params;  //线程函数的参数 
#if defined(_WIN32)
	unsigned int (*fun)(void *params);   //线程函数指针 
#else
	void* (*fun)(void *params);   //线程函数指针 
#endif
};

/**
 * @brief 线程创建
 * @param  iThreadNum       线程创建的个数
 * @param  funArray         线程处理函数组
 */
void vStartThread(int iThreadNum, ThreadFun funArray[]);

#endif