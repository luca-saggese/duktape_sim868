/*
 * socket.c
 *
 *  Created on: 2015/7/8/
 *      Author: jk
 */
#include <stdio.h>
#include <eat_interface.h>
#include <eat_socket.h>

#include "log.h"
#include "socket.h"
//#include "setting.h"
#include "log.h"
#include "client.h"
#include "msg.h"
//#include "data.h"
#include "timer.h"
//#include "fsm.h"
#include "error.h"
#include "commands.h"
#include "msg_queue.h"

static char* getEventDescription(soc_event_enum event);
static char* getStateDescription(cbm_bearer_state_enum state);
static void  hostname_notify_cb(u32 request_id, eat_bool result, u8 ip_addr[4]);


static s8 socket_id = 0;

static u32 request_id = 0;
static soc_state_enum socket_state=SOC_CLOSED;

#define DESC_DEF(x)	case x:\
                        return #x

/*
SOC_READ    = 0x01,   Notify for read
SOC_WRITE   = 0x02,   Notify for write
SOC_ACCEPT  = 0x04,   Notify for accept
SOC_CONNECT = 0x08,   Notify for connect
SOC_CLOSE   = 0x10,   Notify for close
SOC_ACKED   = 0x20,   Notify for acked
*/
static char* getEventDescription(soc_event_enum event)
{
	switch (event)
	{
#ifdef LOG_DEBUG_FLAG
		DESC_DEF(SOC_READ);
		DESC_DEF(SOC_WRITE);
		DESC_DEF(SOC_ACCEPT);
		DESC_DEF(SOC_CONNECT);
		DESC_DEF(SOC_CLOSE);
		DESC_DEF(SOC_ACKED);
#endif
		default:
		{
			static char soc_event[10] = {0};
			sprintf(soc_event, "%d", event);
			return soc_event;
		}
	}
}

/*
CBM_DEACTIVATED             = 0x01,  deactivated
CBM_ACTIVATING              = 0x02,  activating
CBM_ACTIVATED               = 0x04,  activated
CBM_DEACTIVATING            = 0x08,  deactivating
CBM_CSD_AUTO_DISC_TIMEOUT   = 0x10,  csd auto disconnection timeout
CBM_GPRS_AUTO_DISC_TIMEOUT  = 0x20,  gprs auto disconnection timeout
CBM_NWK_NEG_QOS_MODIFY      = 0x040,  negotiated network qos modify notification
CBM_WIFI_STA_INFO_MODIFY
*/
static char* getStateDescription(cbm_bearer_state_enum state)
{
	switch (state)
	{
#ifdef LOG_DEBUG_FLAG
		DESC_DEF(CBM_DEACTIVATED);
		DESC_DEF(CBM_ACTIVATING);
		DESC_DEF(CBM_ACTIVATED);
		DESC_DEF(CBM_DEACTIVATING);
		DESC_DEF(CBM_CSD_AUTO_DISC_TIMEOUT);
		DESC_DEF(CBM_GPRS_AUTO_DISC_TIMEOUT);
		DESC_DEF(CBM_NWK_NEG_QOS_MODIFY);
		DESC_DEF(CBM_WIFI_STA_INFO_MODIFY);
#endif
		default:
		{
			static char bearer_state[10] = {0};
			sprintf(bearer_state, "%d", state);
			return bearer_state;
		}
	}
}


soc_state_enum socketState(){
  return socket_state;
}

int socket_connect2IP(u8 ip_addr[4], int port)
{
    s8 rc = SOC_SUCCESS;

    sockaddr_struct address = {SOC_SOCK_STREAM};

    address.sock_type = SOC_SOCK_STREAM;
    address.addr_len = 4;

    address.addr[0] = ip_addr[0];
    address.addr[1] = ip_addr[1];
    address.addr[2] = ip_addr[2];
    address.addr[3] = ip_addr[3];

    LOG_DEBUG("ip: %d.%d.%d.%d:%d.", address.addr[0], address.addr[1], address.addr[2], address.addr[3], port);


    address.port = port;                /* TCP server port */
    rc = eat_soc_connect(socket_id, &address);
    if(rc >= 0)
    {
        LOG_DEBUG("socket id of new connection is :%d.", rc);
        eat_send_cmd_login();
        return ERR_SOCKET_CONNECTED;
    }
    else if (rc == SOC_WOULDBLOCK)
    {
        LOG_DEBUG("Connection is in progressing...");
        return ERR_SOCKET_WAITING;
    }
    else
    {
        LOG_ERROR("Connect return error:%d!", rc);
        return ERR_SOCKET_FAILED;
    }
}


static void hostname_notify_cb(u32 request_id, eat_bool result, u8 *ip_addr)
{
	if (result == EAT_TRUE)
	{
		//LOG_DEBUG("hostname notify:%s -> %d.%d.%d.%d.", setting.domain, ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3], setting.port);

		socket_connect2IP(ip_addr, 3006); //TODO:this should be done in function action_hostname2ip
		//fsm_run(EVT_HOSTNAME2IP);
	}
	else
	{
	    LOG_ERROR("hostname_notify_cb error: request_id = %d", request_id);
	    //fsm_run(EVT_HOSTNAME2IP_FAILED);
	}

    return;
}

static void soc_notify_cb(s8 s,soc_event_enum event,eat_bool result, u16 ack_size)
{
  u8 buffer[1152] = {0};//1K + 128 for upgrade module
  s32 rc = 0;

  LOG_DEBUG("SOCKET notify:socketid(%d), event(%s).", s, getEventDescription(event));
  
  switch (event)
  {
    case SOC_READ:

      rc = eat_soc_recv(socket_id, buffer, 1152);//1K + 128 for upgrade module
      if (rc > 0)
      {
        client_proc(buffer, rc);
      }
      else
      {
        LOG_ERROR("eat_soc_recv error:rc=%d!", rc);
      }

      break;

  case SOC_CONNECT:
    if(result)
    {
      socket_state=SOC_CONNECTED;
      LOG_DEBUG("SOC_CONNECT success.");
      //fsm_run(EVT_SOCKET_CONNECTED);
    }
    else
    {
      LOG_ERROR("SOC_CONNECT failed, maybe the server is OFF!");
      //fsm_run(EVT_SOCKET_CONNECT_FAILED);
      eat_soc_close(s);
    }
    break;

  case SOC_CLOSE:
    LOG_INFO("SOC_CLOSE:socketid = %d", s);
    eat_soc_close(s);
    //fsm_run(EVT_SOCKET_DISCONNECTED);
    break;

  case SOC_ACKED:
    LOG_DEBUG("acked size of send data: %d.", ack_size);
    break;

  default:
    LOG_INFO("SOC_NOTIFY %d not handled", event);
    break;
  }
}

static void bear_notify_cb(cbm_bearer_state_enum state, u8 ip_addr[4])
{
  LOG_DEBUG("bear_notify state: %s.", getStateDescription(state));
	switch (state)
	{
    case CBM_ACTIVATED:
      LOG_DEBUG("local ip is %d.%d.%d.%d", ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);

      //fsm_run(EVT_BEARER_HOLD);
      break;

    case CBM_DEACTIVATED:
      socket_state=SOC_CLOSED;
      //fsm_run(EVT_BEARER_DEACTIVATED);
      break;

    case CBM_GPRS_AUTO_DISC_TIMEOUT:
      eat_reset_module();
      break;

    case CBM_ACTIVATING:
    case CBM_DEACTIVATING:
      //no need to handle
      break;

    default:
      LOG_ERROR("BEAR_NOTIFY %d not handled", state);
      break;
	}
}

int socket_init(u8 addr_type, u8 *ip_addr, char *domain, int port, char *apn_name, char *apn_user_name, char *apn_password)
{
  s8 rc = eat_gprs_bearer_open(apn_name, apn_user_name, apn_password, bear_notify_cb); 
  
  if (rc == CBM_WOULDBLOCK)
  {
    socket_state=SOC_CONNECTING;
    LOG_DEBUG("opening bearer...");
  }
  else if (rc == CBM_OK)
  {
    LOG_DEBUG("open bearer success.");
    rc = eat_gprs_bearer_hold();
    if (rc == CBM_OK)
    {
      LOG_DEBUG("hold bearer success.");
      return socket_setup(addr_type, ip_addr, domain, port);
    }
    else
    {
      LOG_ERROR("hold bearer failed!");
      return ERR_HOLD_BEARER_FAILED;
    }
  }
  else
  {
    LOG_ERROR("open bearer failed: rc = %d", rc);
    return ERR_OPEN_BEARER_FAILED;
  }
  return SUCCESS;
}

int socket_connect(u8 addr_type, u8 ip_addr[4], char *domain, int port)
{
  s8 rc = SOC_SUCCESS;

  if (addr_type == ADDR_TYPE_IP)
  {
    return socket_connect2IP(ip_addr, 3006);
  }
  else
  {
    u8 ipaddr[4] = {0};
    u8 len = 0;

    eat_soc_gethost_notify_register(hostname_notify_cb);
    rc = eat_soc_gethostbyname(domain, ipaddr, &len, request_id++);
    if (rc == SOC_WOULDBLOCK)
    {
      LOG_DEBUG("eat_soc_gethostbyname wait callback.");
      return ERR_WAITING_HOSTNAME2IP;
    }
    else if (rc == SOC_SUCCESS)
    {
      LOG_DEBUG("host:%s -> %d.%d.%d.%d:%d.", domain, ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3], port);
      return socket_connect2IP(ipaddr, 3006);
    }
    else
    {
      LOG_ERROR("eat_soc_gethostbyname error!");
      return ERR_GET_HOSTBYNAME_FAILED;
    }
  }
}

int socket_setup(u8 addr_type, u8 *ip_addr, char *domain, int port)
{
	s8 rc = SOC_SUCCESS;
	s8 val = EAT_TRUE;

  eat_soc_notify_register(soc_notify_cb);
  socket_id = eat_soc_create(SOC_SOCK_STREAM, SOC_SOCK_STREAM);
  if (socket_id < 0)
  {
    LOG_ERROR("eat_soc_create return error :%d!", socket_id);
    return ERR_SOCKET_CREAT_FAILED;
  }
  else
  {
    LOG_DEBUG("eat_soc_create ok, socket_id = %d.", socket_id);
  }

  rc = eat_soc_setsockopt(socket_id, SOC_NBIO, &val, sizeof(val));
  if (rc != SOC_SUCCESS)
  {
    LOG_ERROR("eat_soc_setsockopt set SOC_NBIO failed: %d!", rc);
    return ERR_SOCKET_OPTION_FAILED;
  }

  rc = eat_soc_setsockopt(socket_id, SOC_NODELAY, &val, sizeof(val));
  if (rc != SOC_SUCCESS)
  {
    LOG_ERROR("eat_soc_setsockopt set SOC_NODELAY failed: %d!", rc);
    return ERR_SOCKET_OPTION_FAILED;
  }

  val = SOC_READ | SOC_WRITE | SOC_CLOSE | SOC_CONNECT;
  rc = eat_soc_setsockopt(socket_id, SOC_ASYNC, &val, sizeof(val));
  if (rc != SOC_SUCCESS)
  {
    LOG_ERROR("eat_soc_setsockopt set SOC_ASYNC failed: %d!", rc);
    return ERR_SOCKET_OPTION_FAILED;
  }

  return socket_connect(addr_type, ip_addr, domain, port);
}

s32 socketClose(){
  eat_soc_close(socket_id);
  return eat_gprs_bearer_release();
}

s32 eat_msg_send(void* data, s32 len)
{
  s32 rc;

  LOG_HEX((const char*)data, len);

  rc = eat_soc_send(socket_id, data, len);
  if (rc >= 0)
  {
    LOG_DEBUG("socket send data successful.");
  }
  else
  {
    /*if (rc == SOC_PIPE || rc == SOC_NOTCONN)
    {
      fsm_run(EVT_SOCKET_DISCONNECTED);       //这个地方仅为保护作用，正常的socket断链应该会通过soc_notify_cb来通知
    }*/
    LOG_ERROR("socket send data failed:%d!", rc);
  }

  free_msg(data);  //TODO: is it ok to free the msg here???
  return rc;
}

s32 eat_msg_send_packed(void* data, s32 len, u8 force){
  msg_push(data, len, NULL, NULL);
  if(force || (msg_count()>SEND_LIST_LENGTH && socketState()==SOC_CLOSED)){
    //socketopen(con i settings salvati da qualche parte);
    LOG_DEBUG("coda piena,inviare");
    return 1;
  }
  return 0;
}

s32 socket_sendDataDirectly(void* data, s32 len)
{
  s32 rc = eat_msg_send(data, len);
  free_msg(data);
  return rc;
}

s32 socket_sendDataWaitAck(void* data, s32 len, MSG_RESEND_FAILED_HANDLER pfn, void* userdata)
{
  s32 rc = eat_msg_send(data, len);
  msg_push(data, len, pfn, userdata);
  return rc;
}
