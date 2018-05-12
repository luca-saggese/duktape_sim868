/*
 * protocol.h
 *
 *  Created on: 2015/6/29
 *      Author: jk
 *
 *  Copyright (c) 2015 Wuhan Xiaoan Technology Co., Ltd. All rights reserved.
 *
 *  Change log:
 *      2.15    去掉CMD_LOGIN中的CCID字段
 *              增加CMD_SIM_INFO命令字
 *
 */

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

enum
{
    PROTOCOL_VERSION_251 = 215,
};

#define PROTOCOL_VERSION    PROTOCOL_VERSION_251

#define START_FLAG (0x6767)
#define MAX_IMEI_LENGTH 16
#define MAX_CCID_LENGTH 20
#define MAX_IMSI_LENGTH 15
#define MAX_CELL_NUM 7
#define TEL_NO_LENGTH 11
#define ACC_COUNT 10
#define CRASH_GPS_COUNT 10
#define CRASH_ACC_COUNT 100

enum
{
    CMD_GET_AT          = 98,
    CMD_GSM_STRENTH     = 94,
    CMD_433_STRENTH     = 93,
    CMD_GPS_STRENTH     = 92,
    CMD_LOGINFO_GET     = 91,
    
    /* base commands */
    CMD_WILD            =  0,
    CMD_LOGIN           =  1,
    CMD_GPS             =  2,
    CMD_PING            =  3,
    CMD_ALARM           =  4,
    CMD_STATE           =  5,
    CMD_SMS             =  6,
    CMD_OBD_DATA        =  7,
    CMD_CRASH           =  8,
    
    /* more commands */
    CMD_CELL            = 10,
    CMD_433             = 11,
    CMD_DEFEND          = 12,
    CMD_SEEK            = 13,
    CMD_LOCATE          = 14,
    CMD_SET_SERVER      = 15,
    CMD_SET_TIMER       = 16,
    CMD_SET_AUTOSWITCH  = 17,
    CMD_GET_AUTOSWITCH  = 18,
    CMD_SET_PERIOD      = 19,
    CMD_GET_PERIOD      = 20,
    CMD_ITINERARY       = 21,
    CMD_BATTERY         = 22,
    CMD_DEFEND_ON       = 23,
    CMD_DEFEND_OFF      = 24,
    CMD_DEFEND_GET      = 25,
    CMD_DEFEND_NOTIFY   = 26,
    CMD_UPGRADE_START   = 27,
    CMD_UPGRADE_DATA    = 28,
    CMD_UPGRADE_END     = 29,
    CMD_SIM_INFO        = 30,
    
    CMD_GPS_PACK        = 31,
    
    CMD_REBOOT          = 32,
    CMD_DEVICE_INFO_GET = 33,
    
};

enum
{
    MSG_SUCCESS = 0,
    MSG_VERSION_NOT_SUPPORTED = -1,
    MSG_DISK_NO_SPACE = -2,
    MSG_UPGRADE_CHECKSUM_FAILED = -3,
};

#pragma pack(push, 1)


 
 
 
/*
 * Message header definition
 */
typedef struct
{
    short signature;
    char  cmd;
    short length;
    short seq;
}__attribute__((__packed__)) MSG_HEADER;

#define MSG_HEADER_LEN sizeof(MSG_HEADER)


 /*
  * get log message structure
  */
 typedef struct
 {
     MSG_HEADER header;
     int managerSeq;
     char data[];
 }__attribute__((__packed__)) MSG_GET_AT_REQ;
 
 typedef struct
 {
     MSG_HEADER header;
     int managerSeq;
     char data[];
 }__attribute__((__packed__)) MSG_GET_AT_RSP;
 
 typedef struct
{
    MSG_HEADER header;
    int managerSeq;
}__attribute__((__packed__)) MSG_GET_HEADER;


/*
 * get log message structure
 */

typedef MSG_GET_HEADER MSG_GET_LOG_REQ;

typedef struct
{
    MSG_HEADER header;
    int managerSeq;
    char data[];
}__attribute__((__packed__)) MSG_GET_LOG_RSP;

/*
 * get 433 message structure
 */
typedef MSG_GET_HEADER MSG_GET_433_REQ;

typedef struct
{
    MSG_HEADER header;
    int managerSeq;
    char data[];
}__attribute__((__packed__)) MSG_GET_433_RSP;

/*
 * get GSM message structure
 */
typedef MSG_GET_HEADER MSG_GET_GSM_REQ;

/*
 * Login message structure
 */
typedef struct
{
    MSG_HEADER header;
    char IMEI[MAX_IMEI_LENGTH/2];
    char version[3];
    char device_type;
}__attribute__((__packed__)) MSG_LOGIN_REQ;

enum DeviceType{
    GX004 = 1,
    GX005 = 2
};

typedef MSG_HEADER MSG_LOGIN_RSP;

/*
 * GPS structure
 */
 
typedef struct
{
    u32 timestamp;
    u32 latitude;
    u32 longitude;
    u8 speed;
    u16 course;
    u16 altitude;
    u8 fixMode;
    u32 meters;
}__attribute__((__packed__)) GPS;

/*
 * acceleration structure
 */
typedef struct
{
    float X;
    float Y;
    float Z;
}__attribute__((__packed__)) ACCELERATION;

/*
 * Crash message structure
 */
typedef struct
{
    MSG_HEADER header;
    GPS gps[CRASH_GPS_COUNT];
    ACCELERATION accelerations[CRASH_ACC_COUNT];
}__attribute__((__packed__)) MSG_CRASH;

/*
 * packed GPS message structure
 */
typedef struct
{
    MSG_HEADER header;
    GPS gps[];
}__attribute__((__packed__)) MSG_GPS_PACK;

/*
 * GPS message structure
 */
typedef struct
{
    MSG_HEADER header;
    GPS gps;
    int timestamp;
}__attribute__((__packed__)) MSG_GPS;

/*
 * CELL structure
 */
typedef struct
{
   short lac;       //local area code
   short cellid;    //cell id
   char rxl;       //receive level
}__attribute__((__packed__)) CELL;

typedef struct
{
    short mcc;  //mobile country code
    short mnc;  //mobile network code
    char  cellNo;// cell count
    CELL cell[1];
}__attribute__((__packed__)) CGI;       //Cell Global Identifier

/*
 * CGI message structure
 */
 typedef struct
 {
     MSG_HEADER header;
     int timestamp;
     CGI cgi;
 }__attribute__((__packed__)) MSG_CGI;

/*
 * heartbeat message structure
 */
typedef struct
{
    MSG_HEADER header;
    short status;   //TODO: to define the status bits
}__attribute__((__packed__)) MSG_PING_REQ;

typedef MSG_HEADER MSG_PING_RSP;

/*
 * alarm message structure
 */
enum ALARM_TYPE
{
    ALARM_FENCE_OUT = 1,
    ALARM_FENCE_IN,
    ALARM_VIBRATE,
};

typedef struct
{
    MSG_HEADER header;
    GPS position;
    char alarmType;
}__attribute__((__packed__)) MSG_ALARM_REQ;

typedef struct
{
    MSG_HEADER header;
    GPS position;
    char alarmType;
    int acc_buffer[ACC_COUNT][3];
}__attribute__((__packed__)) MSG_CRASH_REQ;

typedef MSG_HEADER MSG_ALARM_RSP;

/*
 * SMS message structure
 */
typedef struct
{
    MSG_HEADER header;
    char telphone[TEL_NO_LENGTH + 1];
    char smsLen;
    char sms[];
}__attribute__((__packed__)) MSG_SMS_REQ;

typedef MSG_SMS_REQ MSG_SMS_RSP;

/*
 * seek message structure
 * the message has no response
 */
typedef struct
{
    MSG_HEADER header;
    int intensity;
}__attribute__((__packed__)) MSG_433;

/*
 * GPS set_time message structure
 */

typedef struct
{
    MSG_HEADER header;
    int timer;
}__attribute__((__packed__)) MSG_SET_TIMER_REQ;

typedef struct
{
    MSG_HEADER header;
    int result;
}__attribute__((__packed__)) MSG_SET_TIMER_RSP;

/*
*server set_ip/domain message structure
*this message has no response
*/
typedef struct
{
    MSG_HEADER header;
    int port;
    char server[];
}__attribute__((__packed__)) MSG_SET_SERVER;

/*
 * defend message structure
 */
 enum DEFEND_TYPE
{
    DEFEND_ON   = 0x01,
    DEFEND_OFF  = 0x02,
    DEFEND_GET  = 0x03,
};

typedef struct
{
    MSG_HEADER header;
    int token;
    char operator;     // refer to DEFEND_TYPE
}__attribute__((__packed__)) MSG_DEFEND_REQ;

typedef struct
{
    MSG_HEADER header;
    int token;
    char result;
}__attribute__((__packed__)) MSG_DEFEND_RSP;

/*
 * switch on the seek mode
 */
 enum SEEK_TYPE
{
    SEEK_OFF    = 0x01,
    SEEK_ON     = 0x02,
};

typedef struct
{
    MSG_HEADER header;
    int token;
    char operator;     //refer to SEEK_TYPE
}__attribute__((__packed__)) MSG_SEEK_REQ;

typedef struct
{
    MSG_HEADER header;
    int token;
    char result;
}__attribute__((__packed__)) MSG_SEEK_RSP;

typedef MSG_HEADER MSG_LOCATE;

typedef struct
{
    MSG_HEADER header;
    int timestamp;
    char isGps;
    GPS gps;
}__attribute__((__packed__)) MSG_GPSLOCATION_RSP;   //FIXME: change the name

typedef struct
{
    MSG_HEADER header;
    char isGps;
}__attribute__((__packed__)) MSG_CELLLOCATION_HEADER;   //FIXME: change the name



enum AUTODEFEND_SWITCH
{
    AUTO_DEFEND_OFF,
    AUTO_DEFEND_ON,
};

typedef struct
{
    MSG_HEADER header;
    int token;
    unsigned char onOff; //refer to AUTODEFEND_SWITCH
}__attribute__((__packed__)) MSG_AUTODEFEND_SWITCH_SET_REQ;

typedef struct
{
    MSG_HEADER header;
    int token;
    unsigned char result;
}__attribute__((__packed__)) MSG_AUTODEFEND_SWITCH_SET_RSP;

typedef struct
{
    MSG_HEADER header;
    int token;
}__attribute__((__packed__)) MSG_AUTODEFEND_SWITCH_GET_REQ;

typedef struct
{
    MSG_HEADER header;
    int token;
    unsigned char result;
}__attribute__((__packed__)) MSG_AUTODEFEND_SWITCH_GET_RSP;

typedef struct
{
    MSG_HEADER header;
    int token;
    unsigned char period;   //time unit: minutes
}__attribute__((__packed__)) MSG_AUTODEFEND_PERIOD_SET_REQ;

typedef struct
{
    MSG_HEADER header;
    int token;
    unsigned char result;
}__attribute__((__packed__)) MSG_AUTODEFEND_PERIOD_SET_RSP;


typedef struct
{
    MSG_HEADER header;
    int token;
}__attribute__((__packed__)) MSG_AUTODEFEND_PERIOD_GET_REQ;

typedef struct
{
    MSG_HEADER header;
    int token;
    unsigned char period;   //time unit: minutes
}__attribute__((__packed__)) MSG_AUTODEFEND_PERIOD_GET_RSP;

typedef struct
{
    MSG_HEADER header;
    int starttime;
    int endtime;
    int mileage;
}__attribute__((__packed__)) MSG_MILEAGE_REQ;

typedef struct
{
    MSG_HEADER header;
    char percent;
    char miles;
}__attribute__((__packed__)) MSG_BATTERY_RSP;


typedef struct
{
    MSG_HEADER header;
    char state;             //0 express OFF,1 express ON
}__attribute__((__packed__)) MSG_AUTODEFEND_STATE_REQ;

typedef struct
{
    MSG_HEADER header;
    int version;
    int size;
}__attribute__((__packed__)) MSG_UPGRADE_START;

typedef struct
{
    MSG_HEADER header;
    char code;      //MSG_SUCCESS means OK to upgrade
}__attribute__((__packed__)) MSG_UPGRADE_START_RSP;

typedef struct
{
    MSG_HEADER header;
    int offset;
    char data[];
}__attribute__((__packed__)) MSG_UPGRADE_DATA;

typedef struct
{
    MSG_HEADER header;
    int offset;
}__attribute__((__packed__)) MSG_UPGRADE_DATA_RSP;

typedef struct
{
    MSG_HEADER header;
    int checksum;
    int size;
}__attribute__((__packed__)) MSG_UPGRADE_END;

typedef struct
{
    MSG_HEADER header;
    char code;
}__attribute__((__packed__)) MSG_UPGRADE_END_RSP;

typedef struct
{
    MSG_HEADER header;
    char CCID[MAX_CCID_LENGTH];
    char IMSI[MAX_IMSI_LENGTH];
}__attribute__((__packed__)) MSG_SIM_INFO;

typedef MSG_HEADER MSG_DEBUG_REQ;
typedef struct
{
    MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MSG_DEBUG_RSP;

#pragma pack(pop)

#endif /* _PROTOCOL_H_ */
