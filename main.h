enum CMD
{
    CMD_THREAD_JS_EVENT
};


typedef struct
{
    u8 cmd;
    u32 ctx;
    u8 length;
    u8 data[];
}MSG_THREAD;

typedef struct
{
    float X;
    float Y;
    float Z;
}ACC;

typedef struct
{
    u32 lat;
    u32 lon;
    u8 speed;
    u16 heading;
}POSITION;

typedef struct
{
    float A;
    float B;
    float G;
}ROTMATRIX;

#define ACC_STORE_SIZE 100
ACC acc_store[ACC_STORE_SIZE]={0};
u8 acc_pointer=0;

#define POS_STORE_SIZE 10
POSITION pos_store[POS_STORE_SIZE]={0};
u8 pos_pointer=0;

ROTMATRIX rot_matrix={0};



/*
eat_bool sendMsg(EatUser_enum peer, void* msg, u8 len)
{
    return eat_send_msg_to_user(eat_get_task_id(), peer, EAT_TRUE, len, EAT_NULL, &msg);
}
*/
