/*
 * timer.h
 *
 *  Created on: 2015/6/25
 *      Author: jk
 */

#ifndef USER_ELECTROMBILE_TIMER_H_
#define USER_ELECTROMBILE_TIMER_H_

#include <eat_timer.h>

#define TIMER_GPS  	        EAT_TIMER_1     //GPS现场中获取定时获取GPS位置
#define TIMER_GPS_PERIOD    1000
#define TIMER_ACCEL         EAT_TIMER_2     //震动线程中检测车辆震动状况
#define TIMER_ACCEL_PERIOD  100



#define TIMER_MSG_RESEND    EAT_TIMER_3     //消息重传
#define TIMER_LOOP          EAT_TIMER_4

#define TIMER_GPS_SEND  EAT_TIMER_5
//#define TIMER_SEEK  	EAT_TIMER_6
//#define TIMER_SOCKET	EAT_TIMER_7
//#define TIMER_HEARTBEAT	EAT_TIMER_8
#define TIMER_CRASH_ALARM EAT_TIMER_8
#define TIMER_SEEKAUTOOFF EAT_TIMER_9
#define TIMER_UPDATE_RTC EAT_TIMER_10
#define TIMER_MOVE_ALARM EAT_TIMER_11
#define TIMER_VOLTAGE_GET EAT_TIMER_12
#define TIMER_FTP EAT_TIMER_13
#define TIMER_CAN EAT_TIMER_14
#define TIMER_FAST_LOOP EAT_TIMER_15

#define TIMER_FAST_LOOP_PERIOD 200

void app_timer_thread(void *data);

#endif /* USER_ELECTROMBILE_TIMER_H_ */
