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

#include "duktape.h"
#include "duk_module_node.h"

#include "internal.h"



#include "eat_clib_define.h" //only in main.c


#include "eat_interface.h"
#include "eat_gps.h" 
#include "eat_fs.h"

#include "eat_mem.h"
#define DYNAMIC_MEM_SIZE 1024*400
static unsigned char app_dynamic_mem[DYNAMIC_MEM_SIZE];
duk_context *global_ctx;

/********************************************************************
 * Macros
 ********************************************************************/
#define EAT_UART_RX_BUF_LEN_MAX 2000
#define INDEX_FILE L"D:\\index.js"
#define NMEA_BUFF_SIZE 1024
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
//static duk_context *jsctx;
/********************************************************************
 * External Functions declaration
 ********************************************************************/
extern void APP_InitRegions(void);

/********************************************************************
 * Local Function declaration
 ********************************************************************/
void app_main(void *data);
void app_func_ext1(void *data);

/********************************************************************
 * C Bindings
 ********************************************************************/
 

/*

static duk_ret_t my_eat_gps_power_req(duk_context *ctx) {
  int arg = duk_to_number(ctx, 1);
  eat_gps_power_req((eat_bool) arg);
  return 0;  // one return value 
}

static duk_ret_t my_eat_gps_power_status(duk_context *ctx) {
  int res = eat_gps_power_status();
  duk_push_number(ctx, res);
  return 1;  // one return value 
}

static duk_ret_t my_eat_gps_status_get(duk_context *ctx) {
  int res = eat_gps_status_get();
  duk_push_number(ctx, res);
  return 1;  // one return value 
}

static duk_ret_t my_eat_get_version(duk_context *ctx) {
  const char * res = eat_get_version();
  duk_push_string(ctx, res);
  return 1;  // one return value 
}
*/

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

    duk_eval_string(ctx, resolved_id);
    return 1;  /*nrets*/
}

int load_file(duk_context *ctx, char *filename) {
    static char readBuf[2048]='\0';
    unsigned int readLen=2048;
    unsigned int Len=2048;
    int FileHandle1 = eat_fs_Open((const unsigned short *)filename,FS_READ_WRITE);
    if(FileHandle1>EAT_FS_NO_ERROR){
      while(readLen==Len){
        int fs_Op_ret=eat_fs_Read(FileHandle1, readBuf, Len, &readLen);  
        if(EAT_FS_NO_ERROR!=fs_Op_ret){
          eat_trace("Read File Fail,and Return Error is %d,Readlen is %d",fs_Op_ret,readLen);
          eat_fs_Close(FileHandle1);
          return 0;        
        } else {                     
          duk_push_string(ctx, readBuf);        
        }
      }
      duk_eval_noresult(ctx);
      eat_fs_Close(FileHandle1);

      return 1;
    }else{
      eat_trace("unable to read file %s", filename);
      return 0;
    }
}

duk_ret_t cb_load_module(duk_context *ctx) {
    const char * FILE_NAME = duk_get_string(ctx, 1);
    load_file(ctx, (char *)FILE_NAME);
    return 1;  /*nrets*/
}
/*
duk_ret_t my_eat_timer_start(duk_context *ctx) {
    EatTimer_enum id = (EatTimer_enum)duk_get_int(ctx, 0);
    unsigned int period = duk_get_uint(ctx, 1);
    eat_timer_start(id, period);
    return 1; 
}

duk_ret_t my_eat_timer_stop(duk_context *ctx) {
    EatTimer_enum id = (EatTimer_enum)duk_get_int(ctx, 0);
    eat_timer_stop(id);
    return 1;  
}

duk_ret_t my_eat_gps_nmea_info_output(duk_context *ctx) {
    int id = (EatTimer_enum)duk_get_int(ctx, 0);
    eat_gps_nmea_info_output(id, gps_info_buf,NMEA_BUFF_SIZE);
    duk_push_string(ctx, gps_info_buf);
    return 1;  
}
*/
//

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

  register_bindings(ctx);

  
/*  
  duk_push_c_function(ctx, my_eat_gps_power_req, 1);
  duk_put_global_string(ctx, "eat_gps_power_req");
  
  duk_push_c_function(ctx, my_eat_get_version, 0);
  duk_put_global_string(ctx, "eat_get_version");
  
  duk_push_c_function(ctx, my_eat_gps_power_status, 0);
  duk_put_global_string(ctx, "eat_gps_power_status");
  
  duk_push_c_function(ctx, my_eat_gps_status_get, 0);
  duk_put_global_string(ctx, "eat_gps_status_get");

  duk_push_c_function(ctx, my_eat_timer_start, 2);
  duk_put_global_string(ctx, "eat_timer_start");

  duk_push_c_function(ctx, my_eat_timer_stop, 1);
  duk_put_global_string(ctx, "eat_timer_stop");

  duk_push_c_function(ctx, my_eat_gps_nmea_info_output, 1);
  duk_put_global_string(ctx, "eat_gps_nmea_info_output");
*/
  
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
		(app_user_func)EAT_NULL,//app_user1,
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

 
  
}




/*
//Register your functions e.g. into the global object:



static duk_ret_t native_adder(duk_context *ctx) {
  int i;
  int n = duk_get_top(ctx);  // #args 
  double res = 0.0;

  for (i = 0; i < n; i++) {
    res += duk_to_number(ctx, i);
  }

  duk_push_number(ctx, res);
  return 1;  // one return value 
}

duk_push_c_function(ctx, native_print, 1); //1: nargs
duk_put_global_string(ctx, "print");
duk_push_c_function(ctx, native_adder, DUK_VARARGS);
duk_put_global_string(ctx, "adder");

//You can then call your function from Ecmascript code:

duk_eval_string_noresult(ctx, "print('2+3=' + adder(2, 3));");
*/

int evalJs(char *code) {
  //duk_eval_string(jsctx, code);
  //ard
  //eat_trace(code, (int) duk_get_int(jsctx, -1));
  
  return 0;
}

void event_register_handler(EatEvent_enum event, event_handler_func handler)
{ 
    EventHandlerTable[event] = handler;
}

eat_bool eat_modem_data_parse(u8* buffer, u16 len, u8* param1, u8* param2)
{
    eat_bool ret_val = EAT_FALSE;
    u8* buf_ptr = NULL;
    /*param:%d,extern_param:%d*/
    buf_ptr = (u8*)strstr((const char *)buffer,"param");
    if( buf_ptr != NULL)
    {
        sscanf((const char *)buf_ptr, "param:%d,extern_param:%d",(int*)param1, (int*)param2);
        eat_trace("data parse param1:%d param2:%d",*param1, *param2);
        ret_val = EAT_TRUE;
    }
    return ret_val;
}

//Read data from Modem
void mdm_rx_proc(void)
{
    u8 buf[2048];
    u8 param1,param2;
    u16 len = 0;
    len = eat_modem_read(buf, 2048);
    eat_trace(buf);/*
    if(len > 0)
    {
        //Get the testing parameter
        if(eat_modem_data_parse(buf,len,&param1,&param2))
        {
          //evalJs("2+1");
          //evalJs("eat_trace('hello world')");
            //eat_module_test_adc(param1, param2);
        }
        else
        {
            eat_trace("From Mdm:%s",buf);
        }
    }*/

}

static void uart_rx_proc(const EatEvent_st* event)
{
    u16 len;
    EatUart_enum uart = event->data.uart.uart;

    len = eat_uart_read(uart, rx_buf, EAT_UART_RX_BUF_LEN_MAX);

    if(len != 0)
    {
        rx_buf[len] = '\0';
        eat_trace("> %s", rx_buf);

        duk_eval_string(global_ctx, rx_buf);
        
        //eat_uart_write(uart, rx_buf, len);
        
    }
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
    }

    eat_trace(" app_main ENTRY");
    mem_ini_flag = eat_mem_init(app_dynamic_mem, sizeof(app_dynamic_mem));
    if(EAT_TRUE == mem_ini_flag)
    {
      eat_trace("eat_mem_init() OK\n"); 
    }else
    {
      eat_trace("eat_mem_init() ERROR\n");
    } 


    global_ctx = duk_create_heap_default();
    eat_trace("heap created");

    register_bindings2(global_ctx);

    if(load_file(global_ctx, (char *)INDEX_FILE)) {
      eat_trace("index.js loaded");
    }



    //duk_eval_string(global_ctx, "1+2");
    //eat_trace("1+2=%d\n", (int) duk_get_int(global_ctx, -1));
    
    duk_eval_string(global_ctx, "eat_trace('js is working!');");
    //duk_eval_string(global_ctx, "eat_trace( eat_get_version());");
    //duk_destroy_heap(ctx);


    //event_register_handler(EAT_EVENT_TIMER, timer_proc);
    //event_register_handler(EAT_EVENT_KEY, key_proc);
    //event_register_handler(EAT_EVENT_MDM_READY_RD, (event_handler_func)mdm_rx_proc);
    //event_register_handler(EAT_EVENT_MDM_READY_WR, mdm_tx_proc);
    //event_register_handler(EAT_EVENT_INT, int_proc);
    event_register_handler(EAT_EVENT_UART_READY_RD, uart_rx_proc);
    //event_register_handler(EAT_EVENT_UART_READY_WR, uart_tx_proc);
    //event_register_handler(EAT_EVENT_UART_SEND_COMPLETE, uart_send_complete_proc);
    //event_register_handler(EAT_EVENT_USER_MSG, user_msg_proc);
    //event_register_handler(EAT_EVENT_ADC, (event_handler_func)adc_event_proc);
    
    //jsctx = duk_create_heap_default();
    //register_bindings(jsctx);
    


  
    while(EAT_TRUE)
    {
        eat_get_event(&event);
        if(event.event==EAT_EVENT_TIMER){
           sprintf(strbuf, "if(event_callback){ event_callback({event:%d, timer_id:%d}); }", event.event, event.data.timer.timer_id );
        }else if(event.event==EAT_EVENT_KEY){
           sprintf(strbuf, "if(event_callback){ event_callback({event:%d, key_value:%d, is_pressed:%d}); }", event.event, event.data.key.key_value, event.data.key.is_pressed );
        }else if(event.event==EAT_EVENT_INT){
           sprintf(strbuf, "if(event_callback){ event_callback({event:%d, pin:%d, level:%d}); }", event.event, event.data.interrupt.pin, event.data.interrupt.level );
        }else if(event.event==EAT_EVENT_UART_READY_RD || event.event==EAT_EVENT_UART_READY_WR || event.event==EAT_EVENT_UART_SEND_COMPLETE){
           sprintf(strbuf, "if(event_callback){ event_callback({event:%d, uart:%d}); }", event.event, event.data.uart.uart );
        }else if(event.event==EAT_EVENT_ADC){
           sprintf(strbuf, "if(event_callback){ event_callback({event:%d, pin:%d, v:%d}); }", event.event, event.data.adc.pin, event.data.adc.v );
        }else if(event.event==EAT_EVENT_USER_MSG){
           sprintf(strbuf, "if(event_callback){ event_callback({event:%d, src:%d, use_point:%d}); }", event.event, event.data.user_msg.src, event.data.user_msg.use_point );
        }else{
          sprintf(strbuf, "if(event_callback){ event_callback({event:%d}); }", event.event );
        }

        duk_eval_string(global_ctx, strbuf);
        //eat_trace("%s-%d:msg %x", __FILE__, __LINE__,event.event);
        func = EventHandlerTable[event.event];
        if(event.event < EAT_EVENT_NUM && func != NULL)
        {
            (*func)(&event);
        }
    }
}

