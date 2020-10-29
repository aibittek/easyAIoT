#include <stdio.h>
#include <stdint.h>
#include "ntp.h"
#include "EILog.h"
#include "EIPlatform.h"
#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")
#endif

static uint8_t packetBuffer[48] = {0xe3,0,6,0xec,0,0,0,0,0,0,0,0,0x31,0x4e,0x31,0x34};
unsigned long dwGetNTPtimes()
{
    unsigned long ulTime = 0;
    const char *pszUrl = "cn.ntp.org.cn";
    short nPort = 123;
    char szIpv4[16] = {0};
    char sRecv[96] = {0};

#if defined(_WIN32)
    WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return 0;
	}
#endif

    sock_t sclient = socket(AF_INET, SOCK_DGRAM, 0);
	if (sclient == INVALID_SOCKET) {
		printf("invalid socket\n");
		return 0;
	}
    
    struct hostent *he;
    if ((he = gethostbyname(pszUrl)) == NULL) {
        return 0;
    }
    memcpy(szIpv4, he->h_addr_list[0], sizeof(szIpv4));
 
	struct sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(nPort);
	serAddr.sin_addr.s_addr = inet_addr(szIpv4);
    sendto(sclient, packetBuffer, 48, 0, (struct sockaddr *)&serAddr, sizeof(serAddr));
	
    struct sockaddr_in serverAddr;
    int iLen = sizeof(serverAddr);
    int iRecvSize = recvfrom(sclient, sRecv, sizeof(sRecv), 0, (struct sockaddr *)&serverAddr, &iLen);

    ulTime = (unsigned long)sRecv[40];
    ulTime = sRecv[40] << 24 | sRecv[41] << 16 | sRecv[42] << 8 | sRecv[43];
    ulTime -= 2208988800;
    
    printf("ulTime:%u\r\n", ulTime);
    printf("%u %u:%u:%u\r\n", ulTime/31556736+1970, (ulTime/3600)%24+8, (ulTime/60)%60, ulTime%60);

    closesocket(sclient);

#if defined(_WIN32)
    WSACleanup();
#endif
}

time_t dwGetNTPtime(const char *pszUrl)
{
    time_t ttime = 0;

#ifdef _WIN32
    WSADATA wsaData;   
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
        return 0;
#endif

    sock_t sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == INVALID_SOCKET)
        return 0;

    struct hostent *he;
    struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(123);
    if ((he = gethostbyname(pszUrl)) != NULL) {
        memcpy(&addr.sin_addr.s_addr, he->h_addr_list[0], sizeof(addr.sin_addr.s_addr));
    }

    fd_set pending_data;
    struct timeval block_time;

    int count = 0, result;
    NTPPacket ntpSend = {0};
    ntpSend.nControlWord = 0x1B;
    NTPPacket ntpRecv;
    if ((result = sendto(sockfd, (const char *)&ntpSend, sizeof(NTPPacket), 0, (struct sockaddr *)&addr, sizeof(struct sockaddr))) < 0) {
        LOG(EERROR, "sendto error");
        return 0;
    }
    FD_ZERO(&pending_data);
    FD_SET(sockfd, &pending_data);

    //timeout 10 sec
    block_time.tv_sec = 10;
    block_time.tv_usec = 0;

    struct sockaddr_in addrSever = { 0 };
	int nServerAddrLen=sizeof(struct sockaddr_in);
    if (select(sockfd + 1, &pending_data, NULL, NULL, &block_time) > 0)
    {
        //获取的时间为1900年1月1日到现在的秒数
        if ((count = recvfrom(sockfd, (char *)&ntpRecv, sizeof(NTPPacket), 0, (struct sockaddr *)&addrSever,&nServerAddrLen) > 0))
            ttime = ntohl(ntpRecv.nTransmitTimestampSeconds) - YEAR70_BY_SECONDS;
    }

    closesocket(sockfd);
#ifdef _WIN32
    WSACleanup();
#endif
    return ttime;
}
