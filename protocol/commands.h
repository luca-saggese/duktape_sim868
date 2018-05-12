/*
 * commands.h
 *
 *  Created on: 2016年2月4日
 *      Author: jk
 */

#ifndef USER_MAIN_REQUEST_H_
#define USER_MAIN_REQUEST_H_

#include "protocol.h"

int eat_send_cmd_login(void);
int cmd_SMS(const void* msg);
void eat_send_heartbeat(void);
int cmd_Seek(unsigned int value);
int eat_send_gps_pos(GPS* gps);

int eat_send_crash(GPS* gps[], ACCELERATION* accelerations[]);
int cmd_Itinerary_check(void);
int cmd_alarm(char alarm_type);



int eat_send_sim_data(char* buf);


#endif /* USER_MAIN_REQUEST_H_ */
