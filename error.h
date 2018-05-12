/*
 * error.h
 *
 *  Created on: 2016年2月4日
 *      Author: jk
 */

#ifndef USER_INC_ERROR_H_
#define USER_INC_ERROR_H_

enum
{
    SUCCESS = 0,
    ERR_OPEN_BEARER_FAILED = -1,
    ERR_HOLD_BEARER_FAILED  = -2,
    ERR_SOCKET_CREAT_FAILED = -3,
    ERR_SOCKET_OPTION_FAILED = -4,
    ERR_GET_HOSTBYNAME_FAILED = -5,
    ERR_WAITING_HOSTNAME2IP = -6,
    ERR_SOCKET_CONNECTED = -7,
    ERR_SOCKET_WAITING = -8,
    ERR_SOCKET_FAILED = -9,
};


#endif /* USER_INC_ERROR_H_ */
