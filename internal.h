#include "duktape.h"
#include "eat_sms.h"

void register_bindings(duk_context *ctx);
u8 eat_mma_read(u8 scale, float *accelG);
void duk_sms_init(Sms_New_Message_Ind eat_sms_new_message_cb, Sms_Ready_Ind eat_sms_ready_cb, 
                  Sms_Flash_Message_Ind eat_sms_flash_message_cb, Sms_Send_Completed eat_sms_send_cb );
void eat_bt_spp_write(u8 id, char *data);
s8 eat_bt_spp_accept(u8 id);

