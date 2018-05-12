/*
 * socket.h
 *
 *  Created on: 2015��7��8��
 *      Author: jk
 */

#ifndef USER_ELECTROMBILE_SOCKET_H_
#define USER_ELECTROMBILE_SOCKET_H_

#include "eat_type.h"
typedef enum
{
	ADDR_TYPE_IP,
	ADDR_TYPE_DOMAIN
}ADDR_TYPE;

/*
 * 消息发送失败处理函数，如:将消息持久化待下次发送
 * userdata为回调函数的附加参数，由调用方确定，如不需要可置为null
 */
typedef int (*MSG_RESEND_FAILED_HANDLER)(void* msg, int length, void* userdata);

int socket_init(u8 addr_type, u8 *ip_addr, char *domain, int port, char *apn_name, char *apn_user_name, char *apn_password);
int socket_setup(u8 addr_type, u8 *ip_addr, char *domain, int port);
s32 socketClose();
/*
int socket_connect(void);
void socket_close(void);
s32 eat_msg_send(void* data, s32 len);
*/

int eat_msg_send(void* data, int len);

/*
 * 直接发送消息，无需重传
 */
int socket_sendDataDirectly(void* data, int len);

int eat_msg_send_packed(void* data, int len, u8 force);

/*
 * 发送消息并等待ack，如果没有收到ack则重传
 */
int socket_sendDataWaitAck(void* data, int len, MSG_RESEND_FAILED_HANDLER pfn, void* userdata);

typedef enum
{
    SOC_CONNECTING  = 0x01,  /* Notify for read */
    SOC_CONNECTED   = 0x02,  /* Notify for write */
    SOC_CLOSED      = 0x04,  /* Notify for accept */
} soc_state_enum;
soc_state_enum socketState(void);

#endif /* USER_ELECTROMBILE_SOCKET_H_ */
