/*
 * client.c
 *
 *  Created on: 2015/7/9
 *      Author: jk
 */

#include <stdio.h>
#include <string.h>


#include <eat_interface.h>

#include "client.h"
#include "socket.h"
//#include "mileage.h"
#include  "msg.h"
#include "log.h"
//#include "uart.h"
//#include "data.h"
//#include "setting.h"
//#include "thread.h"
#include "thread_msg.h"
#include "timer.h"
//#include "seek.h"

#include "receive.h"





typedef int (*MSG_PROC)(const void* msg);       //TODO: add the message length parameter
typedef struct
{
    char cmd;
    MSG_PROC pfn;
}MC_MSG_PROC;


static MC_MSG_PROC msgProcs[] =
{
  {CMD_LOGIN, cmd_Login_rsp},
  /*{CMD_PING,  cmd_Ping_rsp},
  {CMD_CELL, cmd_Cell_rsp},
  {CMD_ALARM, cmd_Alarm_rsp},
  {CMD_SMS,   cmd_Sms_rsp},
	{CMD_DEFEND, cmd_Defend_rsp},
  {CMD_SEEK,  cmd_Seek_rsp},
	{CMD_LOCATE, cmd_Location_rsp},
	{CMD_SET_AUTOSWITCH, cmd_AutodefendSwitchSet_rsp},
  {CMD_GET_AUTOSWITCH, cmd_AutodefendSwitchGet_rsp},
  {CMD_SET_PERIOD, cmd_AutodefendPeriodSet_rsp},
  {CMD_GET_PERIOD, cmd_AutodefendPeriodGet_rsp},
  {CMD_SET_SERVER, cmd_Server_rsp},
  {CMD_SET_TIMER, cmd_Timer_rsp},
  {CMD_BATTERY, cmd_Battery_rsp},
  {CMD_UPGRADE_START, cmd_UpgradeStart_rsp},
  {CMD_UPGRADE_DATA, cmd_UpgradeData_rsp},
  {CMD_UPGRADE_END, cmd_UpgradeEnd_rsp},
  
  {CMD_REBOOT, cmd_Reboot_rsp},
  //{CMD_DEVICE_INFO_GET, cmd_DeviceInfo_rsp},
  //{CMD_ITINERARY, cmd_Itinerary_rsp},
  {CMD_SIM_INFO, cmd_SimInfo_rsp},
  {CMD_LOGINFO_GET, cmd_LogInfo_rsp},
  {CMD_GSM_STRENTH, cmd_GSMSignal_rsp},
  {CMD_GPS_STRENTH, cmd_GPSSignal_rsp},
  {CMD_GET_AT, cmd_GetAT_rsp},*/
}; 

int handle_one_msg(const void* m, int msgLen)
{
    MSG_HEADER* msg = (MSG_HEADER*)m;
    size_t i = 0;

    for (i = 0; i < sizeof(msgProcs) / sizeof(msgProcs[0]); i++)
    {
        if (msgProcs[i].cmd == msg->cmd)
        {
            MSG_PROC pfn = msgProcs[i].pfn;
            if (pfn)
            {
                return pfn(msg);
            }
            else
            {
                LOG_ERROR("Message %d not processed!", msg->cmd);
                return -1;
            }
        }
    }

    LOG_ERROR("unknown message %d!", msg->cmd);
    return -1;
}

int client_proc(const void *m, int msgLen)
{
    const MSG_HEADER *msg = (const MSG_HEADER *)m;
    size_t leftLen = 0;

    LOG_HEX(m,msgLen);

    if(msgLen < MSG_HEADER_LEN)
    {
        LOG_ERROR("message length not enough: %zu(at least(%zu)", msgLen, sizeof(MSG_HEADER));
        return -1;
    }

    leftLen = msgLen;
    while(leftLen >= ntohs(msg->length) + MSG_HEADER_LEN - 2) //-2 perchè la lunghezza include il serial number (2 bytes)
    {
        const unsigned char *status = (const unsigned char *)(&(msg->signature));
        if((status[0] != 0x67) || (status[1] != 0x67))
        {
            LOG_ERROR("receive message header signature error:%x", (unsigned)ntohs(msg->signature));
            return -1;
        }
        handle_one_msg(msg, ntohs(msg->length) + MSG_HEADER_LEN);
        leftLen = leftLen - MSG_HEADER_LEN - ntohs(msg->length);
        msg = (const MSG_HEADER *)((const char *)m + msgLen - leftLen);
        LOG_HEX((const char *)msg,leftLen);
    }
    return 0;
}


