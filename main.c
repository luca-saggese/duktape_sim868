/********************************************************************
 *                Copyright Simcom(shanghai)co. Ltd.                   *
 *---------------------------------------------------------------------
 * FileName      :   main.c
 * version       :   0.10
 * Description   :   
 * Authors       :   maobin
 * Notes         :
 *---------------------------------------------------------------------
 *
 *    HISTORY OF CHANGES
 *---------------------------------------------------------------------
 *0.10  2012-09-24, maobin, create originally.
 *
 *--------------------------------------------------------------------
 * File Description
 * AT+CEAT=param1,param2
 * param1 param2 
 *   
 *--------------------------------------------------------------------
 ********************************************************************/
/********************************************************************
 * Include Files
 ********************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "eat_modem.h"
#include "eat_interface.h"
#include "eat_uart.h"
#include "eat_sms.h"
#include "duktape.h"
#include "duk_module_node.h"

#include "internal.h"



#include "eat_clib_define.h" //only in main.c



#include "eat_gps.h" 
#include "eat_fs.h"
#include "timer.h"
#include "spi.h"

#include "eat_mem.h"
#define DYNAMIC_MEM_SIZE 1024*300
static unsigned char app_dynamic_mem[DYNAMIC_MEM_SIZE];
duk_context *global_ctx=0;
u8 heap_init=0;

/********************************************************************
 * Macros
 ********************************************************************/
#define EAT_UART_RX_BUF_LEN_MAX 2000
#define INDEX_FILE L"D:\\index.js"
#define NMEA_BUFF_SIZE 1024
#define VERSION_STR "sim868_duktape v0.0.1"

#define EAT_TIMER_GPS EAT_TIMER_16
#define EAT_TIMER_ACC EAT_TIMER_15
/********************************************************************
 * Types
 ********************************************************************/
typedef void (*app_user_func)(void*);
typedef void (*event_handler_func)(const EatEvent_st *event);

/********************************************************************
 * Extern Variables (Extern /Global)
 ********************************************************************/
 
/********************************************************************
 * Local Variables:  STATIC
 ********************************************************************/
static EatEntryPara_st app_para={0};
static event_handler_func EventHandlerTable[EAT_EVENT_NUM] = {0};
static u8 rx_buf[EAT_UART_RX_BUF_LEN_MAX + 1] = {0};
static u16 wr_uart_offset = 0; //用来标记uart未写完的数据
static u16 wr_uart_len = 0; //下次需要往串口写的数据长度
static char gps_info_buf[NMEA_BUFF_SIZE]="";
u8 ret_pin=1;
static char spp_buffer[500]={0};
//static duk_context *jsctx;
/********************************************************************
 * External Functions declaration
 ********************************************************************/
extern void APP_InitRegions(void);
void initHeap();

/********************************************************************
 * Local Function declaration
 ********************************************************************/
void app_main(void *data);
void app_func_ext1(void *data);

/********************************************************************
 * C Bindings
 ********************************************************************/
 
/*require load*/
duk_ret_t cb_resolve_module(duk_context *ctx) {
    /*
     *  Entry stack: [ requested_id parent_id ]
     */

    const char *requested_id = duk_get_string(ctx, 0);
    const char *parent_id = duk_get_string(ctx, 1);  /* calling module */
    const char *resolved_id;

    /* Arrive at the canonical module ID somehow. */
    
    resolved_id=requested_id;

    duk_eval_string_noresult(ctx, resolved_id);
    return 1;  /*nrets*/
}

int load_file(duk_context *ctx, char *filename) {
    static char * readBuf;
    unsigned int readLen=2048;
    //unsigned int Len=2048;
    int fs_Op_ret;
    unsigned int size;
    int FileHandle1 = eat_fs_Open((const unsigned short *)filename,FS_READ_WRITE);
    if(FileHandle1>EAT_FS_NO_ERROR){
      eat_fs_GetFileSize(FileHandle1, &size);
      readBuf=eat_mem_alloc(size+1);
      fs_Op_ret=eat_fs_Read(FileHandle1, readBuf, size, &readLen);  
      readBuf[readLen]='\0';
      if(EAT_FS_NO_ERROR!=fs_Op_ret){
        eat_trace("Read File Fail,and Return Error is %d,Readlen is %d",fs_Op_ret,readLen);
        eat_fs_Close(FileHandle1);
        return 0;        
      } else {    
        eat_trace("filesize:%d readLen:%d", size, readLen);                 
        duk_push_string(ctx, readBuf);        
      }
      duk_eval_noresult(ctx);
      eat_fs_Close(FileHandle1);
      eat_mem_free(readBuf);

      return 1;
    }else{
      eat_trace("unable to read file");
      return 0;
    }
}

duk_ret_t cb_load_module(duk_context *ctx) {
    const char * FILE_NAME = duk_get_string(ctx, 1);
    load_file(ctx, (char *)FILE_NAME);
    return 1;  /*nrets*/
}

static void eat_sms_new_message_cb(EatSmsNewMessageInd_st smsNewMessage)
{
  char strbuf[1024];
  sprintf(strbuf, "if(typeof eat_sms_new_message_cb !== 'undefined'){eat_sms_new_message_cb(%d);}", smsNewMessage.index);
  duk_eval_string_noresult(global_ctx, strbuf);
  //  eat_read_sms(smsNewMessage.index, eat_sms_read_cb);
}

static eat_sms_flash_message_cb(EatSmsReadCnf_st smsFlashMessage)
{
    u8 format = 0;

    eat_trace("flash message.");
    eat_get_sms_format(&format);
    if(1 == format)//TEXTʽ
    {
        eat_trace("recv TEXT sms.");
        eat_trace("msg=%s.",smsFlashMessage.data);
        eat_trace("datetime=%s.",smsFlashMessage.datetime);
        eat_trace("name=%s.",smsFlashMessage.name);
        eat_trace("status=%d.",smsFlashMessage.status);
        eat_trace("len=%d.",smsFlashMessage.len);
        eat_trace("number=%s.",smsFlashMessage.number);
    }
    else//PDU
    {
        eat_trace("recv PDU sms.");
        eat_trace("msg=%s",smsFlashMessage.data);
        eat_trace("len=%d",smsFlashMessage.len);
    }
}

static void eat_sms_ready_cb(eat_bool result)
{
  char strbuf[1024];
  sprintf(strbuf, "if(typeof eat_sms_ready_cb !== 'undefined'){eat_sms_ready_cb(%d);}", result);
  duk_eval_string_noresult(global_ctx, strbuf);
}

static void eat_sms_send_cb(eat_bool result)
{
  char strbuf[1024];
  sprintf(strbuf, "if(typeof eat_sms_send_cb !== 'undefined'){eat_sms_send_cb(%d);}", result);
  duk_eval_string_noresult(global_ctx, strbuf);
}

duk_ret_t _duk_sms_init(duk_context *ctx) {
	duk_sms_init(eat_sms_new_message_cb, eat_sms_ready_cb, eat_sms_flash_message_cb, eat_sms_send_cb);
	return 0;
}


static register_bindings2(duk_context *ctx){
  /* After initializing the Duktape heap or when creating a new
   * thread with a new global environment:
   */
  duk_push_object(ctx);
  duk_push_c_function(ctx, cb_resolve_module, DUK_VARARGS);
  duk_put_prop_string(ctx, -2, "resolve");
  duk_push_c_function(ctx, cb_load_module, DUK_VARARGS);
  duk_put_prop_string(ctx, -2, "load");
  duk_module_node_init(ctx);

  duk_push_c_function(ctx, _duk_sms_init, 0);
	duk_put_global_string(ctx, "duk_sms_init");

  register_bindings(ctx);
  //duk_sms_init(eat_sms_new_message_cb, eat_sms_ready_cb, eat_sms_flash_message_cb, eat_sms_send_cb);

}

/********************************************************************
 * Local Function
 ********************************************************************/

#pragma arm section rodata = "APP_CFG"
APP_ENTRY_FLAG 
#pragma arm section rodata

#pragma arm section rodata="APPENTRY"
	const EatEntry_st AppEntry = 
	{
		app_main,
		app_func_ext1,
		(app_user_func)app_timer_thread,//app_user1,
		(app_user_func)EAT_NULL,//app_user2,
		(app_user_func)EAT_NULL,//app_user3,
		(app_user_func)EAT_NULL,//app_user4,
		(app_user_func)EAT_NULL,//app_user5,
		(app_user_func)EAT_NULL,//app_user6,
		(app_user_func)EAT_NULL,//app_user7,
		(app_user_func)EAT_NULL,//app_user8,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL
	};
#pragma arm section rodata

void app_func_ext1(void *data)
{
  
	/*This function can be called before Task running ,configure the GPIO,uart and etc.
	   Only these api can be used:
		 eat_uart_set_debug: set debug port
		 eat_pin_set_mode: set GPIO mode
		 eat_uart_set_at_port: set AT port
	*/
	EatUartConfig_st cfg ={
                            EAT_UART_BAUD_115200,
                            EAT_UART_DATA_BITS_8,
                            EAT_UART_STOP_BITS_1,
                            EAT_UART_PARITY_NONE
                        };
	
	eat_uart_set_debug(EAT_UART_1);
	eat_uart_set_debug_config(EAT_UART_DEBUG_MODE_UART, &cfg);
  eat_uart_set_at_port(EAT_UART_USB);// UART1 is as AT PORT
  //eat_modem_set_poweron_urc_dir(EAT_USER_0);

/*
  ret_pin=ret_pin && eat_pin_set_mode(EAT_PIN5_UART1_DCD, EAT_PIN_MODE_SPI);
  ret_pin=ret_pin && eat_pin_set_mode(EAT_PIN6_UART1_DTR, EAT_PIN_MODE_SPI);
  ret_pin=ret_pin && eat_pin_set_mode(EAT_PIN7_UART1_RI, EAT_PIN_MODE_SPI);
  ret_pin=ret_pin && eat_pin_set_mode(EAT_PIN14_SIM1_DET, EAT_PIN_MODE_SPI);
  */
  //ret_pin=ret_pin && eat_gpio_setup(EAT_PIN5_UART1_DCD,EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);

/*

EatPinName_enum SPI_PIN_CS = EAT_PIN5_UART1_DCD;
EatPinName_enum SPI_PIN_CLK = EAT_PIN6_UART1_DTR;
EatPinName_enum SPI_PIN_MOSI = EAT_PIN7_UART1_RI;
EatPinName_enum SPI_PIN_MISO = EAT_PIN14_SIM1_DET;
EatPinName_enum SPI_PIN_CD = EAT_PIN42_STATUS;
*/
  //eat_spi_config(EAT_PIN14_SIM1_DET, EAT_PIN7_UART1_RI, EAT_PIN42_STATUS, EAT_PIN5_UART1_DCD);
  eat_spi_init_int(EAT_SPI_CLK_6M, EAT_SPI_3WIRE, EAT_SPI_BIT8, EAT_TRUE, EAT_TRUE);
}




void event_register_handler(EatEvent_enum event, event_handler_func handler)
{ 
    EventHandlerTable[event] = handler;
}



static void exec_js(char *data, u8 cid){
  char buff[100]={0};
  sprintf(buff, "< %s", data);
  eat_trace(buff);
  if(cid) eat_bt_spp_write(cid, buff);
  duk_eval_string(global_ctx, data);
  if (duk_get_type(global_ctx, -1) == DUK_TYPE_NUMBER) {
    sprintf(buff, "> %d", duk_get_int(global_ctx, -1));
  }else if (duk_get_type(global_ctx, -1) == DUK_TYPE_STRING) {
    sprintf(buff, "> %s", (char *)duk_get_string(global_ctx, -1));
  }
  eat_trace(buff);
  if(cid) eat_bt_spp_write(cid, buff);
}

void modem_rx_proc(void)
{
  unsigned char buf[1024]={0};
  unsigned char buf2[100]={0};
  char *ptr=(char*)&buf;
  char *ptr2;
  int i=0,j=0;
  u8 cid=0;
  char strbuf[200];
  u16 len = 0;
  len = eat_modem_read((unsigned char*)&buf, 1024);

  //+BTSPPDATA: 3,2,te
  ptr2=strstr((const char *)&buf,"+BTSPPDATA:");
  if(ptr2){
    i=sscanf((const char *)buf + (ptr2-ptr), "+BTSPPDATA: %d,%d,%s", &cid, &len, &buf2);
    eat_trace("BTSPPDATA cid:%d ,len:%d ,buf2:%s",cid,len,buf2);
    ptr=ptr2;
    ptr+=16+len;
    if(*(ptr-1)=='\r'){
      spp_buffer[0]='\0';
      eat_bt_spp_write(cid, "\r\n");
      exec_js( spp_buffer, cid);
    }else{
      eat_bt_spp_write(cid, buf2);
      strcat((char*)&spp_buffer, (char*)&buf2);
    }
  }
  ptr2=strstr((const char *)&buf,"+BTCONNECTING:");
  if(ptr2){
    eat_bt_spp_accept(1);
  }

  if(len==0)
    return;
  while (*ptr) {
    if(*ptr=='\'') *ptr='_';
    if(*ptr=='\r' || (*(ptr+1)=='\0' && *ptr!='\n')){
      strncpy((char *)&buf2,(const char *) ptr - i + j, i - j);
      buf2[i-j]='\0';
      if(strcmp(buf2, "SMS Ready")==0){
        duk_sms_init(eat_sms_new_message_cb, eat_sms_ready_cb, eat_sms_flash_message_cb, eat_sms_send_cb);
      }

      if(strlen(buf2)>0){
        sprintf(strbuf, "if(typeof modem_callback !== 'undefined') modem_callback(\"%s\"); else eat_trace(\"%s\");", buf2, buf2 );
        if(heap_init && 0){
          duk_eval_string_noresult(global_ctx, strbuf);
        }else{
          eat_trace("%s %d", buf2, eat_get_task_id());
        }
      }
      j=i+2;
    }
    i++;
    ptr++;
  }
}



static void uart_rx_proc(const EatEvent_st* event)
{
    u16 len;
    EatUart_enum uart = event->data.uart.uart;
    len = eat_uart_read(uart, rx_buf, EAT_UART_RX_BUF_LEN_MAX);
    if(len != 0) {
      if(rx_buf[len-1]=='\r')
        rx_buf[len-1] = '\0';
      else
        rx_buf[len-2] = '\0';
      exec_js(rx_buf, 0);
    }
}





static void my_fatal(void *udata, const char *msg) {
    (void) udata;  /* ignored in this case, silence warning */

    /* Note that 'msg' may be NULL. */
    eat_trace("*** FATAL ERROR: %s\nREBOOTING..", (msg ? msg : "no message"));
    eat_sleep(1000);
    eat_reset_module();
}

void initHeap(){
  global_ctx = duk_create_heap(NULL, NULL, NULL, NULL, my_fatal);
  eat_trace("heap created");

  register_bindings2(global_ctx);

  if(load_file(global_ctx, (char *)INDEX_FILE)) {
    eat_trace("index.js loaded");
  }

  duk_eval_string_noresult(global_ctx, "eat_trace('js is working!');");
  
}

void app_main(void *data)
{
    duk_context *ctx;
    char strbuf[1024];
    eat_bool mem_ini_flag=EAT_FALSE;
    EatEvent_st event;
    EatEntryPara_st *para;
    event_handler_func func = NULL;
    APP_InitRegions();//Init app RAM, first step
    APP_init_clib(); //C library initialize, second step
    


    para = (EatEntryPara_st*)data;

    memcpy(&app_para, para, sizeof(EatEntryPara_st));
    eat_trace(" App Main ENTRY  update:%d result:%d", app_para.is_update_app,app_para.update_app_result);
    if(app_para.is_update_app && app_para.update_app_result)
    {
        eat_update_app_ok();
        eat_trace("------------update successfull-------------");
    }

    eat_trace("booting: version:%s, build_time=%s %s. core(version:%s, buildno=%s, buildtime=%s)",
            VERSION_STR, __DATE__, __TIME__, eat_get_version(), eat_get_buildno(), eat_get_buildtime());


    eat_trace("setting pins: %d", ret_pin);

    eat_trace(" app_main ENTRY");
    mem_ini_flag = eat_mem_init(app_dynamic_mem, sizeof(app_dynamic_mem));
    if(EAT_TRUE == mem_ini_flag)
    {
      eat_trace("eat_mem_init() OK\n"); 
    }else
    {
      eat_trace("eat_mem_init() ERROR\n");
    } 

    initHeap();
    
    //duk_eval_string_noresult(global_ctx, "eat_trace( eat_get_version());");
    //duk_destroy_heap(ctx);


    //event_register_handler(EAT_EVENT_TIMER, timer_proc);
    //event_register_handler(EAT_EVENT_KEY, key_proc);
    event_register_handler(EAT_EVENT_MDM_READY_RD, (event_handler_func)modem_rx_proc);
    //event_register_handler(EAT_EVENT_MDM_READY_WR, mdm_tx_proc);
    //event_register_handler(EAT_EVENT_INT, int_proc);
    event_register_handler(EAT_EVENT_UART_READY_RD, uart_rx_proc);
    //event_register_handler(EAT_EVENT_UART_READY_WR, uart_tx_proc);
    //event_register_handler(EAT_EVENT_UART_SEND_COMPLETE, uart_send_complete_proc);
    //event_register_handler(EAT_EVENT_USER_MSG, user_msg_proc);
    //event_register_handler(EAT_EVENT_ADC, (event_handler_func)adc_event_proc);
    
    //jsctx = duk_create_heap_default();
    //register_bindings(jsctx);
    heap_init=1;

    //init bluetooth
    eat_modem_write("AT+BTPOWER=1\r\nAT+BTSPPCFG=MC,1\r\n",strlen("AT+BTPOWER=1\r\nAT+BTSPPCFG=MC,1\r\n"));


/*
    eat_timer_start(EAT_TIMER_ACC, 50);
    eat_timer_start(EAT_TIMER_GPS, 1000);
*/

    while(EAT_TRUE)
    {
        eat_get_event(&event);
        if(event.event==EAT_EVENT_TIMER){
           sprintf(strbuf, "if(typeof event_callback !== 'undefined'){ event_callback({event:%d, timer_id:%d}); }", event.event, event.data.timer.timer_id );
        }else if(event.event==EAT_EVENT_KEY){
           sprintf(strbuf, "if(typeof event_callback !== 'undefined'){ event_callback({event:%d, key_value:%d, is_pressed:%d}); }", event.event, event.data.key.key_value, event.data.key.is_pressed );
        }else if(event.event==EAT_EVENT_INT){
           sprintf(strbuf, "if(typeof event_callback !== 'undefined'){ event_callback({event:%d, pin:%d, level:%d}); }", event.event, event.data.interrupt.pin, event.data.interrupt.level );
        }else if(event.event==EAT_EVENT_UART_READY_RD || event.event==EAT_EVENT_UART_READY_WR || event.event==EAT_EVENT_UART_SEND_COMPLETE){
           sprintf(strbuf, "if(typeof event_callback !== 'undefined'){ event_callback({event:%d, uart:%d}); }", event.event, event.data.uart.uart );
        }else if(event.event==EAT_EVENT_ADC){
           sprintf(strbuf, "if(typeof event_callback !== 'undefined'){ event_callback({event:%d, pin:%d, v:%d}); }", event.event, event.data.adc.pin, event.data.adc.v );
        }else if(event.event==EAT_EVENT_USER_MSG){
           sprintf(strbuf, "if(typeof event_callback !== 'undefined'){ event_callback({event:%d, src:%d, use_point:%d}); }", event.event, event.data.user_msg.src, event.data.user_msg.use_point );
        }else{
          sprintf(strbuf, "if(typeof event_callback !== 'undefined'){ event_callback({event:%d}); }", event.event );
        }

        duk_eval_string_noresult(global_ctx, strbuf);
        //eat_trace("%s-%d:msg %x", __FILE__, __LINE__,event.event);
        func = EventHandlerTable[event.event];
        if(event.event < EAT_EVENT_NUM && func != NULL)
        {
            (*func)(&event);
        }
    }
}

