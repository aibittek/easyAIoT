# EasyIoT（EI）

1.  支持tcp、udp、http、websocket、mqtt的client/server
2.  支持跨平台，windows/linux/lwip



## 计划

第一期：支持tcp、udp、http、websocket的client端[3月中旬]

第二期：支持mqtt的client[3月下旬]

第三期：支持tcp、udp、http、websocket、mqtt的server端[4月份]

第四期：支持SSL/TLS[5月份]

第五期：支持freertos/RT-Thread



## 接口说明

应用层接口：

typedef void* (pvEIEventCallback)();

iEITcpClient(const char *pszDomain, short nPort, );



网络适配层：

// 支持的协议

enum EEIProtocol

{

​	EEIProtocol_UDP,

​	EEIProtocol_TCP,

​	EEIProtocol_HTTP,

​	EEIProtocol_WEBSOCKET,

​	EEIProtocol_MQTT

};

// 网络操作抽象接口

struct SEINetOperation {

​		int (*iOpen)

};

// epoll/select提供的接口

onOpen,onClose,onError,onRecv接口