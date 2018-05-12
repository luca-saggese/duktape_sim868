/*
 * can.c
 *
 *  Created on: 2015/6/25
 *      Author: jk
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <eat_interface.h>
#include <eat_gps.h>
#include "mma8452/mma8452.h"
#include "timer.h"
#include "thread.h"
#include "log.h"
#include "protocol/protocol.h"


GPS last_gps_pos={0};

void read_gps(){
  float lat,lon,alt,utc,speed,course;
  int ttf,num;
  char infobuf[200]={0};
  u32 size = 200;
  GPS pos={0};
  //TODO: controllare se fix
	eat_bool ret = eat_gps_nmea_info_output(0, &infobuf[0], size);
  if(ret){
    sscanf((const char *)infobuf, "$GPSIM,%f,%f,%f,%f,%d,%d,%f,%f",
      &lat, &lon, &alt, &utc, &ttf, &num, &speed, &course);
    pos.timestamp=0;
    pos.latitude=(u32) lat*30000;
    pos.longitude=(u32) lon*30000;
    pos.speed=speed;
    pos.course=course;
    pos.altitude=alt;
    pos.fixMode=1;
    pos.meters=0;
    eat_trace("GPS lat:%f lon%f", lat, lon);
  }



  //TODO: salva in round array
  //TODO: se passato troppo invia messaggio gps
}

void read_acc(){
  float rot[3]={0};
  float accelG[3]={0};
  const u8 scale = 8;
  
  eat_mma_read(scale, &rot[0]);

  //rotate
  eat_acc_rotate(&rot[0], &accelG[0]);

  eat_trace("ACC X:%f Y:%f Z:%f",accelG[0],accelG[1],accelG[2]);

  //TODO: salvare su round array
  //TODO: se supera limite creare e inviare crash
  //TODO: driving behaviour
}


void app_timer_thread(void *data)
{
  EatEvent_st event;
	//MSG_THREAD* msg = 0;
	u8 msgLen = 0;

  LOG_INFO("Timer thread start.");
  mma8452_init();
  eat_sleep(500);
	mma8452_config(8, 0);
  eat_gps_power_req(EAT_TRUE);
        
  //eat_timer_start(TIMER_GPS, TIMER_GPS_PERIOD);
  //eat_timer_start(TIMER_ACCEL, TIMER_ACCEL_PERIOD);

  while(EAT_TRUE)
  {
    eat_get_event_for_user(THREAD_TIMER, &event);
    switch(event.event)
    {
      case EAT_EVENT_TIMER :
        switch (event.data.timer.timer_id)
        {
          case TIMER_GPS:
              //LOG_DEBUG("TIMER_GPS expire.");
              read_gps();
              eat_timer_start(TIMER_GPS, TIMER_GPS_PERIOD);
              break;
          case TIMER_ACCEL:
              //LOG_DEBUG("TIMER_ACCEL expire.");
              read_acc();
              eat_timer_start(TIMER_ACCEL, TIMER_ACCEL_PERIOD);
              break;

          default:
            LOG_ERROR("timer[%d] expire!", event.data.timer.timer_id);
              break;
        }
        break;
               

      case EAT_EVENT_MDM_READY_RD:
        LOG_DEBUG("can AT read.");
        break;

      case EAT_EVENT_USER_MSG:
        /*  msg = (MSG_THREAD*) event.data.user_msg.data_p;
          msgLen = event.data.user_msg.len;

          switch (msg->cmd)
          {  
              default:
                  LOG_ERROR("cmd(%d) not processed!", msg->cmd);
                  break;
          }*/
          break;

      default:
        LOG_ERROR("event(%d) not processed!", event.event);
          break;
    }
  }
}
