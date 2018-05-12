//
// Created by jk on 2015/7/1.
//

#ifndef ELECTROMBILE_FIRMWARE_THREAD_MSG_H
#define ELECTROMBILE_FIRMWARE_THREAD_MSG_H

#include <eat_type.h>
#include <eat_interface.h>

#include "protocol.h"


enum CMD
{
    CMD_THREAD_GPS,
    CMD_THREAD_GPSHDOP,
    CMD_THREAD_SMS,
    CMD_THREAD_VIBRATE,
    CMD_THREAD_CRASH,
    CMD_THREAD_LOCATION,
    CMD_THREAD_AUTOLOCK,
    CMD_THREAD_SMS_LOCATION,
    CMD_THREAD_FTP_UPGRADE,
    CMD_THREAD_CAN,
    CMD_THREAD_GPS_ACTIVATE,
};


typedef struct
{
    u8 cmd;
    u8 length;
    u8 data[];
}MSG_THREAD;


typedef struct
{
    short mcc;  //mobile country code
    short mnc;  //mobile network code
    char  cellNo;// cell count
    CELL cell[MAX_CELL_NUM];
}CELL_INFO;       //Cell Global Identifier

#pragma anon_unions
typedef struct
{
    eat_bool isGps;    //TRUE: GPS; FALSE: 基站信息
    union
    {
        GPS gps;
        CELL_INFO cellInfo;
    };
    u32 timestamp;
}LOCAL_GPS;

typedef struct
{
    char state;
}AUTOLOCK_INFO;

#define allocMsg(len) eat_mem_alloc(len)
#define freeMsg(msg) eat_mem_free(msg)

eat_bool sendMsg(EatUser_enum peer, void* msg, u8 len);

#endif //ELECTROMBILE_FIRMWARE_MSG_H
