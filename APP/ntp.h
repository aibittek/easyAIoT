#ifndef __NTP_H
#define __NTP_H

#include "EIPlatform.h"

// YEAR70_BY_SECONDS��1900-1970��֮�������
// 1900-1970��17�����ֵ꣬Ϊ((1970 - 1900) * 365 + 17) * 24 * 60 * 60 = 2208988800UL;
#define YEAR70_BY_SECONDS 2208988800UL /* 1900�ꡫ1970��֮���ʱ������ */

/**
 * @struct x_ntp_timestamp_t
 * @brief  NTP ʱ�����
 */
typedef struct x_ntp_timestamp_t
{
    unsigned int  xut_seconds;    ///< �� 1900������������������
    unsigned int  xut_fraction;   ///< С�����ݣ���λ��΢������4294.967296( = 2^32 / 10^6 )��
} x_ntp_timestamp_t;

/**
 * @enum  em_ntp_mode_t
 * @brief NTP����ģʽ�����ö��ֵ��
 */
typedef enum em_ntp_mode_t
{
    ntp_mode_unknow     = 0,  ///< δ����
    ntp_mode_initiative = 1,  ///< �����Ե���ģʽ
    ntp_mode_passive    = 2,  ///< �����Ե���ģʽ
    ntp_mode_client     = 3,  ///< �ͻ���ģʽ
    ntp_mode_server     = 4,  ///< ������ģʽ
    ntp_mode_broadcast  = 5,  ///< �㲥ģʽ���鲥ģʽ
    ntp_mode_control    = 6,  ///< ����Ϊ NTP ���Ʊ���
    ntp_mode_reserved   = 7,  ///< Ԥ�����ڲ�ʹ��
} em_ntp_mode_t;

/**
 * @struct x_ntp_packet_t
 * @brief  NTP ���ĸ�ʽ��
 */
typedef struct x_ntp_packet_t
{
    unsigned char     xct_li_ver_mode;      ///< 2 bits����Ծָʾ����3 bits���汾�ţ�3 bits��NTP����    ģʽ���ο� em_ntp_mode_t ���ö��ֵ��
    unsigned char     xct_stratum    ;      ///< ϵͳʱ�ӵĲ�����ȡֵ��ΧΪ1~16����������ʱ�ӵ�׼ȷ    �ȡ�����Ϊ1��ʱ��׼ȷ����ߣ�׼ȷ�ȴ�1��16���εݼ�������Ϊ16��ʱ�Ӵ���δͬ��״̬��������Ϊ�ο�ʱ��
    unsigned char     xct_poll       ;      ///< ��ѯʱ�䣬����������NTP����֮���ʱ����
    unsigned char     xct_percision  ;      ///< ϵͳʱ�ӵľ���

    unsigned int      xut_root_delay     ;  ///< ���ص����ο�ʱ��Դ������ʱ��
    unsigned int      xut_root_dispersion;  ///< ϵͳʱ����������ο�ʱ�ӵ�������
    unsigned int      xut_ref_indentifier;  ///< �ο�ʱ��Դ�ı�ʶ

    x_ntp_timestamp_t xtmst_reference;      ///< ϵͳʱ�����һ�α��趨����µ�ʱ��
    x_ntp_timestamp_t xtmst_originate;      ///< NTP�������뿪���Ͷ�ʱ���Ͷ˵ı���ʱ��
    x_ntp_timestamp_t xtmst_receive  ;      ///< NTP�����ĵ�����ն�ʱ���ն˵ı���ʱ��
    x_ntp_timestamp_t xtmst_transmit ;      ///< Ӧ�����뿪Ӧ����ʱӦ���ߵı���ʱ��
} x_ntp_packet_t;

typedef struct NTPPacket
{
    union {
        struct _ControlWord
        {
            unsigned int uLI : 2;      // 00 = no leap, clock ok
            unsigned int uVersion : 3; // version 3 or version 4
            unsigned int uMode : 3;    // 3 for client, 4 for server, etc.
            unsigned int uStratum : 8; // 0 is unspecified, 1 for primary reference system,
            // 2 for next level, etc.
            int nPoll : 8;      // seconds as the nearest power of 2
            int nPrecision : 8; // seconds to the nearest power of 2
        };

        int nControlWord; // 4
    };

    int nRootDelay;           // 4
    int nRootDispersion;      // 4
    int nReferenceIdentifier; // 4

    int64_t n64ReferenceTimestamp; // 8
    int64_t n64OriginateTimestamp; // 8
    int64_t n64ReceiveTimestamp;   // 8

    int nTransmitTimestampSeconds;   // 4
    int nTransmitTimestampFractions; // 4
}NTPPacket;

time_t dwGetNTPtime(const char *pszUrl);

#endif
