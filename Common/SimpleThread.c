#include "SimpleThread.h"

void vStartThread(int iThreadNum, ThreadFun funArray[])
{
#ifdef _WIN32
    int i;
    HANDLE handle[iThreadNum];
	for(i = 0; i < iThreadNum; i++) {
	   handle[i] = (HANDLE) _beginthreadex(NULL, 0, funArray[i].fun, funArray[i].params, 0, NULL);  
	}

    //等待线程结束
	//WaitForMultipleObjects(threadNum, handle, TRUE, INFINITE); 
    for(i = 0; i < iThreadNum; i++) {
		CloseHandle(handle[i]);    //关闭句柄 
	}
#else  

    pthread_mutex_t mutex;
	pthread_cond_t cond;
    pthread_t pt[iThreadNum];
    int i = 0;
    
    // pthread_mutex_init(&mutex, NULL);
    // pthread_cond_init(&cond, NULL);
    for(i = 0; i < iThreadNum; i++) {
    	pthread_create(&pt[i], NULL, funArray[i].fun, funArray[i].params); 
	}
//	pthread_exit(0);  //等待全部线程结束函数才结束
#endif
}