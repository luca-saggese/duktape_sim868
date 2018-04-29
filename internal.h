void register_bindings(duk_context *ctx);
void duk_sms_init(Sms_New_Message_Ind eat_sms_new_message_cb, Sms_Ready_Ind eat_sms_ready_cb, 
                  Sms_Flash_Message_Ind eat_sms_flash_message_cb, Sms_Send_Completed eat_sms_send_cb );
