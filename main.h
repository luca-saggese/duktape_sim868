enum CMD
{
    CMD_THREAD_JS_EVENT
};


typedef struct
{
    u8 cmd;
    u32 ctx,
    u8 length;
    u8 data[];
}MSG_THREAD;

/*
eat_bool sendMsg(EatUser_enum peer, void* msg, u8 len)
{
    return eat_send_msg_to_user(eat_get_task_id(), peer, EAT_TRUE, len, EAT_NULL, &msg);
}
*/