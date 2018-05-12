/*
 * request.c
 *
 *  Created on: 2016年2月4日
 *      Author: jk
 */

#include <string.h>
#include <stdlib.h>
#include "eat_interface.h"
#include "commands.h"
#include "msg.h"
#include "log.h"
#include "socket.h"
#include "version.h"

#define MAX_GPS_COUNT 10

int eat_send_cmd_login(void){
  MSG_LOGIN_REQ* msg = alloc_msg(CMD_LOGIN, sizeof(MSG_LOGIN_REQ));
  u8 imei[MAX_IMEI_LENGTH] = {0};
  char simei[17]="0";
  char a1[2]="00";
  int i;

  if (!msg)
  {
      LOG_ERROR("alloc login message failed!");
      return -1;
  }

  msg->version[0] = VERSION_MAJOR;
  msg->version[1] = VERSION_MINOR;
  msg->version[2] = VERSION_MICRO;
  msg->device_type = DEVICE_TYPE;

  eat_get_imei(&imei[0], MAX_IMEI_LENGTH);

  //memcpy(&msg->IMEI[0], &imei[0], MAX_IMEI_LENGTH);

  strcat(simei,(char*) imei);
  for(i=0;i<8;i++){
    strncpy(a1, simei+(i*2), 2); 
    msg->IMEI[i] = strtol(a1, NULL, 16);
  }

  LOG_DEBUG("sending login message.");
  eat_msg_send(msg, sizeof(MSG_LOGIN_REQ));

  return 0;
}

int eat_send_sim_data(char* buf){
  MSG_SIM_INFO* msg = alloc_msg(CMD_SIM_INFO, sizeof(MSG_SIM_INFO));
  u8 imsi[MAX_IMSI_LENGTH + 2] = {'\0'};//len: len >=17[length of imsi(15 or 16)+ 1('\0')]
  if (!msg)
  {
    LOG_ERROR("alloc login message failed!");
    return -1;
  }

  eat_get_imsi(imsi,MAX_IMSI_LENGTH + 2);

  sscanf(buf,"%20s",msg->CCID);
  LOG_DEBUG("CCID: %20s",msg->CCID);

  sscanf(imsi,"%15s",msg->IMSI);
  LOG_DEBUG("IMSI: %15s",msg->IMSI);

  LOG_DEBUG("send SIM_info message.");
  eat_msg_send_packed(msg, sizeof(MSG_SIM_INFO), 0);
  return 0;
}

void eat_send_heartbeat(void){
  u8 msgLen = sizeof(MSG_HEADER) + sizeof(short);
  MSG_PING_REQ* msg = alloc_msg(CMD_PING, msgLen);
  msg->status = EAT_TRUE;   //TODO: to define the status bits

  eat_msg_send(msg, msgLen);
}

int eat_send_gps_pos(GPS* gps){
  MSG_GPS* msg = alloc_msg(CMD_GPS, sizeof(MSG_GPS));
  if (!msg)
  {
    LOG_ERROR("alloc GPS message failed!");
    return -1;
  }

  memcpy(&msg->gps, gps, sizeof(GPS));
  //TODO: controllare cosa non va nel memcpy e dobbiamo ricopiare i dati
  msg->gps.timestamp = htonl(gps->timestamp);
  msg->gps.course = htons(gps->course);

  LOG_DEBUG("send GPS message.");
  eat_msg_send_packed(msg, sizeof(MSG_GPS), 0);
  return 0;
}

int eat_send_crash(GPS* gps[], ACCELERATION* accelerations[]){
  u8 msgLen = sizeof(MSG_HEADER) + sizeof(gps) + sizeof(accelerations);
  MSG_CRASH* msg;

  LOG_DEBUG("msgLen %d, count (gps) %d, count (accelerations) %d", msgLen, sizeof(gps)/sizeof(GPS), sizeof(accelerations)/sizeof(ACCELERATION));

  msg = alloc_msg(CMD_CRASH, msgLen);
  if (!msg)
  {
    LOG_ERROR("alloc Crash message failed!");
    return -1;
  }

  memcpy(&msg->gps, gps, sizeof(gps));
  memcpy(&msg->accelerations, accelerations, sizeof(accelerations));

  eat_msg_send_packed(msg, msgLen, 1);
  return 0;
}

/*
int cmd_SMS(const void* msg){
    return 0;
}


void cmd_Wild(const void* m, int len){
    u8 msgLen = sizeof(MSG_HEADER) + len;
    MSG_HEADER* msg = alloc_msg(CMD_WILD, msgLen);

    memcpy(msg + 1, m, len);

    eat_msg_send(msg, msgLen);
}

int cmd_Seek(unsigned int value){
    MSG_433* seek_msg = alloc_msg(CMD_433, sizeof(MSG_433));
    if (!seek_msg)
    {
        LOG_ERROR("alloc seek message failed!");
        return -1;
    }

    LOG_DEBUG("send seek value message:%d", value);
    seek_msg->intensity = htonl(value);
    eat_msg_send(seek_msg, sizeof(MSG_433));

    return 0;
}


*/
