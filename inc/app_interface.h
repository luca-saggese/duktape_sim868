#if !defined (__APP_INTERFACE_H__)
#define __APP_INTERFACE_H__

/*app bin config MACRO begin*/
#define APP_MAGIC          0x004D4D4D 
#define APP_INFO_ID        "FILE_INFO" 
#define FILE_TYPE          0x0103
#define FLASH_TYPE         7

typedef struct CFG_Header {
    u32        magic;
    u16        size;
    u16        type;
} CFG_Header_st;

typedef struct APP_FILE_INFO_v1_
{
    CFG_Header_st   hdr;
    char            identifier[12];       // including '\0'
    u32             file_ver;

    u16             file_type;
    u8              flash_dev;
    u8              sig_type;

    u32             load_addr;
    u32             file_len;
    u32             max_size;
    u32             offset;
    u32             sig_len;
    u32             jump_offset;
    u32             attr;
} APP_FILE_INFO_v1;

typedef struct APP_BIN_INFO_v2_ {
    CFG_Header_st   hdr;
    u8              platform_id[128];
    u8              project_id[64];

    u32             ver;
    u32             combination;
    u32             combination_ex;
    u32             extsram_size;

    u32             minor_ver;
    u32             base_addr; /* For flash tool to check overlap region */
    u32             sds_len;       /* For flash tool to check overlap region */
    u32             ver_req;
    u32             addr;
    u32             len;
    u8              check[16];
    u32             reserved[12];
} APP_BIN_INFO_v2;

typedef struct
{
    APP_FILE_INFO_v1 file;
#if 0
    APP_BIN_INFO_v2  bin;
#endif
}APP_CFG_st;
#define CFG_HEADER(type, ver) { (APP_MAGIC|((ver)<<24)) , sizeof(type##_v##ver), type}
/*app bin config MACRO end*/

/*Ê¡ÂÔ......*/

#endif
