/*
 * msg_queue.c
 *
 *  Created on: 2016年4月20日
 *      Author: jk
 */


#include "msg_queue.h"
#include "socket.h"
#include "list.h"
#include "mem.h"
#include "log.h"
#include "protocol.h"
#include "timer.h"

typedef struct
{
    struct list_head list;
    void* data;
    int length;
    MSG_RESEND_FAILED_HANDLER pfn;
    void* userdata;
    int resend_times;
}MSG_NODE;

int list_length=0;

LIST_HEAD(head);

//每个消息最多重传3次，3次不成功后丢弃
#define MAX_RETRY_TIME  (3)

#define MAX_MSG_LIST_LENGTH 100


//每分钟启动一次重传
#define TIMER_RESEND_PERIOD (120 * 1000)

int msg_count(void){
  return list_length;
}

/*
*push the msg into queue to wait for ack
*/
void msg_push(void* data, int len, MSG_RESEND_FAILED_HANDLER pfn, void* userdata)
{
    MSG_NODE *msg = malloc(sizeof(MSG_NODE));

    if(list_length>MAX_MSG_LIST_LENGTH){
      LOG_DEBUG("queue full discard message");
      return;
    }


    msg->data = data;
    msg->length = len;
    msg->pfn = pfn;
    msg->userdata = userdata;
    msg->resend_times = 0;

    //INIT_LIST_HEAD(&msg->list);       //no need to init, it will be set in list_add
    list_add(&msg->list, &head);
    list_length++;

    LOG_DEBUG("pushed one message in the msg_queue");

    return;
}

/*
*when time reaching, resend the msg in the queue
*/
void msg_resend(void)
{
  struct list_head *iter = NULL;
  MSG_NODE* node = NULL;

  list_for_each(iter, &head)
  {
    node = list_entry(iter, MSG_NODE, list);

    if(node->resend_times++ > MAX_RETRY_TIME)//resend failed more than 3 times , call call-back fun ,do sth
    {
      if(node->pfn)
      {
        node->pfn(node->data, node->length, node->userdata);
      }

      free(node->data);
      free(node->userdata);

      list_del(iter);
      list_length--;
      free(node);
    }
    else
    {
      LOG_DEBUG("resend one msg in msg_queue");
      eat_msg_send(node->data, node->length);
    }
  }
  
  LOG_DEBUG("all messages sent, closing socket");
  socketClose();
  return;
}

/*
*when receive ack, excute the fun stored and free it
*/
void msg_ack(const void* m)
{
    struct list_head *iter = NULL;
    MSG_HEADER* msg = NULL;
    MSG_NODE* node = NULL;
    const MSG_HEADER* ack = m;

    list_for_each(iter, &head)
    {
        node = list_entry(iter, MSG_NODE, list);
        msg = node->data;

        if(ack->cmd == msg->cmd && ack->seq == msg->seq)
        {
            LOG_DEBUG("receive one ack and free the msg stored");

            free(node->data);
            free(node->userdata);

            list_del(iter);
            list_length--;
            free(node);

            break;  //一个seq的消息只可能存在一次
        }
    }

    return;
}

void msg_startResend(void)
{
    eat_timer_start(TIMER_MSG_RESEND, TIMER_RESEND_PERIOD);
}

void msg_stopResend(void)
{
    eat_timer_stop(TIMER_MSG_RESEND);
}
