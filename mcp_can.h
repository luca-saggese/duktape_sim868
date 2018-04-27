/*
  mcp_can.h
  2012 Copyright (c) Seeed Technology Inc.  All right reserved.
  Author:Loovee
  Contributor: Cory J. Fowler
  2014-1-16
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-
  1301  USA
*/

#ifndef _MCP2515_H_
#define _MCP2515_H_

#include "mcp_can_dfs.h"

#include <eat_type.h>

#define MAX_CHAR_IN_MESSAGE 8
#define SPI_RESET_PIN EAT_PIN6_UART1_DTR

    u8 can_begin(u8 speedset);                              // init can                    
    u8 can_init_Mask(u8 num, u8 ext, u32 ulData);           // init Masks                  
    u8 can_init_Filt(u8 num, u8 ext, u32 ulData);           // init filters                
    u8 can_sendMsgBuf(u32 id, u8 ext, u8 len, u8 *buf);  // send buf                    
    u8 can_readMsgBuf(u8 *len, u8 buf[]);                       // read buf                    
    u8 can_checkReceive(void);                                       // if something received       
    u8 can_checkError(void);                                         // if something error          
    u32 can_getCanId(void);                                          // get can id when receive     
    void can_init_spi(void);

//extern MCP_CAN CAN;*/
#endif
