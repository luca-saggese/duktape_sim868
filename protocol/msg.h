/*
 * msg.h
 *
 *  Created on: 2015��7��8��
 *      Author: jk
 */

#ifndef USER_ELECTROMBILE_MSG_H_
#define USER_ELECTROMBILE_MSG_H_

#include <stdio.h>

#include "protocol.h"


#define htonl(l) ((((l >> 24) & 0x000000ff)) | \
                  (((l >>  8) & 0x0000ff00)) | \
                  (((l) & 0x0000ff00) <<  8) | \
                  (((l) & 0x000000ff) << 24))
#define ntohl(l) htonl(l)

#define htons(s) ((((s) >> 8) & 0xff) | \
                  (((s) << 8) & 0xff00))
#define ntohs(s) htons(s)

void* alloc_msg(char cmd, size_t length) __attribute__((malloc));
void* alloc_rspMsg(const MSG_HEADER* pMsg);


void free_msg(void* msg);

#endif /* USER_ELECTROMBILE_MSG_H_ */
