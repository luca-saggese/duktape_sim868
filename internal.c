#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "eat_periphery.h"

#include "eat_modem.h"
#include "eat_interface.h"
#include "eat_uart.h"
#include "eat_gps.h"
#include "eat_socket.h"
#include "eat_sms.h"

#include "duktape.h"
#include "mma8452.h"

#include "log.h"

duk_context *_ctx;
duk_context *sms_ctx;

wchar_t *ctow(const char *buf, wchar_t *output) {
  wchar_t *cr = output;
  while (*buf) {
    *output++ = *buf++;
  }
  *output = 0;
  return cr;
}

duk_ret_t _eat_gpio_setup(duk_context *ctx) {
	EatPinName_enum pin = (EatPinName_enum) duk_get_int(ctx, 0);
	EatGpioDir_enum dir = (EatGpioDir_enum) duk_get_int(ctx, 1);
	EatGpioLevel_enum level = (EatGpioLevel_enum) duk_get_int(ctx, 2);
	eat_bool ret = eat_gpio_setup(pin, dir, level);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_gpio_write(duk_context *ctx) {
	EatPinName_enum pin = (EatPinName_enum) duk_get_int(ctx, 0);
	EatGpioLevel_enum level = (EatGpioLevel_enum) duk_get_int(ctx, 1);
	eat_bool ret = eat_gpio_write(pin, level);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_gpio_read(duk_context *ctx) {
	EatPinName_enum pin = (EatPinName_enum) duk_get_int(ctx, 0);
	EatGpioLevel_enum ret = eat_gpio_read(pin);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_gpio_set_pull(duk_context *ctx) {
	EatPinName_enum pin = (EatPinName_enum) duk_get_int(ctx, 0);
	eat_bool en = (eat_bool) duk_get_int(ctx, 1);
	EatGpioLevel_enum level = (EatGpioLevel_enum) duk_get_int(ctx, 2);
	eat_bool ret = eat_gpio_set_pull(pin, en, level);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_int_setup(duk_context *ctx) {
	EatPinName_enum pin = (EatPinName_enum) duk_get_int(ctx, 0);
	EatIntTrigger_enum trigger = (EatIntTrigger_enum) duk_get_int(ctx, 1);
	unsigned int debounce_10ms = (unsigned int) duk_get_int(ctx, 2);
	eat_gpio_int_callback_func callback = (eat_gpio_int_callback_func) duk_get_int(ctx, 3);
	eat_bool ret = eat_int_setup(pin, trigger, debounce_10ms, callback);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_int_release(duk_context *ctx) {
	EatPinName_enum pin = (EatPinName_enum) duk_get_int(ctx, 0);
	eat_bool ret = eat_int_release(pin);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_int_set_trigger(duk_context *ctx) {
	EatPinName_enum pin = (EatPinName_enum) duk_get_int(ctx, 0);
	EatIntTrigger_enum trigger = (EatIntTrigger_enum) duk_get_int(ctx, 1);
	eat_bool ret = eat_int_set_trigger(pin, trigger);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_spi_init(duk_context *ctx) {
	EatSpiClk_enum clk = (EatSpiClk_enum) duk_get_int(ctx, 0);
	EatSpiWire_enum wire = (EatSpiWire_enum) duk_get_int(ctx, 1);
	EatSpiBit_enum bit = (EatSpiBit_enum) duk_get_int(ctx, 2);
	eat_bool enable_SDI = (eat_bool) duk_get_int(ctx, 3);
	eat_bool enable_cs = (eat_bool) duk_get_int(ctx, 4);
	eat_bool ret = eat_spi_init(clk, wire, bit, enable_SDI, enable_cs);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_spi_write(duk_context *ctx) {
	unsigned char *data = (unsigned char *) duk_get_string(ctx, 0);
	unsigned char len = (unsigned char) duk_get_int(ctx, 1);
	eat_bool is_command = (eat_bool) duk_get_int(ctx, 2);
	eat_bool ret = eat_spi_write(data, len, is_command);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_spi_read(duk_context *ctx) {
	unsigned char len = (unsigned char) duk_get_int(ctx, 0);
  unsigned char data[1024]={0};
	unsigned char ret = eat_spi_read((unsigned char *)&data, len);
	duk_push_string(ctx, data);
	return 1;
}

duk_ret_t _eat_spi_write_read(duk_context *ctx) {
  unsigned char rdata[1024]={0};
	unsigned char *wdata = (unsigned char *) duk_get_string(ctx, 0);
	unsigned char wlen = (unsigned char) duk_get_int(ctx, 1);
	unsigned char rlen = (unsigned char) duk_get_int(ctx, 2);
	unsigned char ret = eat_spi_write_read(wdata, wlen, (unsigned char *)&rdata, rlen);
	duk_push_string(ctx, rdata);
	return 1;
}

duk_ret_t _eat_lcd_light_sw(duk_context *ctx) {
	eat_bool sw = (eat_bool) duk_get_int(ctx, 0);
	EatBLStep_enum step = (EatBLStep_enum) duk_get_int(ctx, 1);
	eat_lcd_light_sw(sw, step);
	return 1;
}

duk_ret_t _eat_kpled_sw(duk_context *ctx) {
	eat_bool sw = (eat_bool) duk_get_int(ctx, 0);
	eat_kpled_sw(sw);
	return 1;
}

duk_ret_t _eat_adc_get(duk_context *ctx) {
	EatPinName_enum pin = (EatPinName_enum) duk_get_int(ctx, 0);
	unsigned int period_tick = (unsigned int) duk_get_int(ctx, 1);
	eat_adc_callback_func callback = (eat_adc_callback_func) duk_get_int(ctx, 2);
	eat_bool ret = eat_adc_get(pin, period_tick, callback);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_adc_get_ms(duk_context *ctx) {
	EatPinName_enum pin = (EatPinName_enum) duk_get_int(ctx, 0);
	unsigned int period_ms = (unsigned int) duk_get_int(ctx, 1);
	eat_adc_callback_func callback = (eat_adc_callback_func) duk_get_int(ctx, 2);
	eat_bool ret = eat_adc_get_ms(pin, period_ms, callback);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_get_adc_sync(duk_context *ctx) {
	EatPinName_enum pin = (EatPinName_enum) duk_get_int(ctx, 0);
	u32 voltage = 0;
	eat_bool ret = eat_get_adc_sync(pin, &voltage);
	duk_push_int(ctx, voltage);
	return 1;
}

duk_ret_t _eat_pwm_start(duk_context *ctx) {
	unsigned int freq = (unsigned int) duk_get_int(ctx, 0);
	unsigned char duty = (unsigned char) duk_get_int(ctx, 1);
	eat_pwm_start(freq, duty);
	return 1;
}

duk_ret_t _eat_pwm_stop(duk_context *ctx) {
	eat_pwm_stop();
	return 1;
}

duk_ret_t _eat_pin_set_mode(duk_context *ctx) {
	EatPinName_enum pin = (EatPinName_enum) duk_get_int(ctx, 0);
	EatPinMode_enum mode = (EatPinMode_enum) duk_get_int(ctx, 1);
	eat_bool ret = eat_pin_set_mode(pin, mode);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_poweroff_key_sw(duk_context *ctx) {
	eat_bool sw = (eat_bool) duk_get_int(ctx, 0);
	eat_poweroff_key_sw(sw);
	return 1;
}

duk_ret_t _eat_sim_detect_en(duk_context *ctx) {
	eat_bool sw = (eat_bool) duk_get_int(ctx, 0);
	eat_sim_detect_en(sw);
	return 1;
}

duk_ret_t _eat_i2c_open(duk_context *ctx) {
	EAT_I2C_OWNER i2cowner = (EAT_I2C_OWNER) duk_get_int(ctx, 0);
	u8 chipid = (u8) duk_get_int(ctx, 1);
	u16 speed = (u16) duk_get_int(ctx, 2);
	s32 ret = eat_i2c_open(i2cowner, chipid, speed);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_i2c_write(duk_context *ctx) {
	EAT_I2C_OWNER i2cowner = (EAT_I2C_OWNER) duk_get_int(ctx, 0);
	u8 *writebuff = (u8 *) duk_get_string(ctx, 1);
	u16 w_len = (u16) duk_get_int(ctx, 2);
	s32 ret = eat_i2c_write(i2cowner, writebuff, w_len);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_i2c_read(duk_context *ctx) {
  u8 readbuff[1024]={0};
	EAT_I2C_OWNER i2cowner = (EAT_I2C_OWNER) duk_get_int(ctx, 0);
	u8 *writebuff = (u8 *) duk_get_string(ctx, 1);
	u16 w_len = (u16) duk_get_int(ctx, 2);
	u16 r_len = (u16) duk_get_int(ctx, 3);
	s32 ret = eat_i2c_read(i2cowner, writebuff, w_len, (u8 *)&readbuff, r_len);
	duk_push_string(ctx, readbuff);
	return 1;
}

duk_ret_t _eat_i2c_close(duk_context *ctx) {
	EAT_I2C_OWNER i2cowner = (EAT_I2C_OWNER) duk_get_int(ctx, 0);
	s32 ret = eat_i2c_close(i2cowner);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_usb_eint_register(duk_context *ctx) {
	eat_usb_eint_callback_func callback = (eat_usb_eint_callback_func) duk_get_int(ctx, 0);
	eat_bool ret = eat_usb_eint_register(callback);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_get_usb_status(duk_context *ctx) {
	eat_bool ret = eat_get_usb_status();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_get_module_temp_sync(duk_context *ctx) {
	s32 temp = 0;
	eat_bool ret = eat_get_module_temp_sync(&temp);
	duk_push_int(ctx, temp);
	return 1;
}
/*
duk_ret_t _eat_get_module_temp_asyn(duk_context *ctx) {
	eat_temp_callback_func callback = (eat_temp_callback_func) duk_get_c_function(ctx, 0);
	eat_bool ret = eat_get_module_temp_asyn(callback);
	duk_push_int(ctx, ret);
	return 1;
}
*/

duk_ret_t _eat_get_cbc(duk_context *ctx) {
  EAT_CBC_ST bmt={0};
	eat_bool ret = eat_get_cbc(&bmt);
	duk_push_int(ctx, ret);
	return 1;
}


duk_ret_t _eat_get_imei(duk_context *ctx) {
	u8 imei[15]={0};
	u8 * ret = eat_get_imei((u8*)&imei, 15);
	duk_push_string(ctx, imei);
	return 1;
}

duk_ret_t _eat_get_imsi(duk_context *ctx) {
	u8 imsi[17]={0};
	u8 * ret = eat_get_imsi((u8*)&imsi, 17);
	duk_push_string(ctx, imsi);
	return 1;
}

duk_ret_t _eat_get_task_id(duk_context *ctx) {
	u8 ret = eat_get_task_id();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_network_get_creg(duk_context *ctx) {
	EatCregState_enum ret = eat_network_get_creg();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_network_get_ber(duk_context *ctx) {
	s32 ret = eat_network_get_ber();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_network_get_csq(duk_context *ctx) {
	s32 ret = eat_network_get_csq();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_network_get_cgatt(duk_context *ctx) {
	u8 ret = eat_network_get_cgatt();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_modem_write(duk_context *ctx) {
	const unsigned char *data = (const unsigned char*) duk_get_string(ctx, 0);
	unsigned short len = strlen(data);
	unsigned short ret = eat_modem_write(data, len);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_modem_read(duk_context *ctx) {
	unsigned char data[2048] = {0};
	unsigned short len = (unsigned short) duk_get_int(ctx, 0);
	unsigned short ret = eat_modem_read((unsigned char *) &data, len);
	duk_push_string(ctx, data);
	return 1;
}

duk_ret_t _eat_modem_set_poweron_urc_dir(duk_context *ctx) {
	unsigned char task_id = (unsigned char) duk_get_int(ctx, 0);
	eat_bool ret = eat_modem_set_poweron_urc_dir(task_id);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_gps_power_req(duk_context *ctx) {
	eat_bool power_flag = (eat_bool) duk_get_int(ctx, 0);
	eat_gps_power_req(power_flag);
	return 1;
}

duk_ret_t _eat_gps_power_status(duk_context *ctx) {
	eat_bool ret = eat_gps_power_status();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_gps_start_mode_get(duk_context *ctx) {
	EatGpsRstMode ret = eat_gps_start_mode_get();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_gps_start_mode_set(duk_context *ctx) {
	EatGpsRstMode mode = (EatGpsRstMode) duk_get_int(ctx, 0);
	eat_bool ret = eat_gps_start_mode_set(mode);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_gps_nmea_info_output(duk_context *ctx) {
  char infobuf[2048]={0};
  u32 size = 2048;
	EatGpsNmeaOutput mode = (EatGpsNmeaOutput) duk_get_int(ctx, 0);
	eat_bool ret = eat_gps_nmea_info_output(mode, (char*)&infobuf, size);
	duk_push_string(ctx, infobuf);
	return 1;
}

duk_ret_t _eat_gps_sleep_set(duk_context *ctx) {
	eat_bool sleep_flag = (eat_bool) duk_get_int(ctx, 0);
	eat_bool ret = eat_gps_sleep_set(sleep_flag);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_gps_sleep_read(duk_context *ctx) {
	eat_bool ret = eat_gps_sleep_read();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_gps_status_get(duk_context *ctx) {
	EatGpsStatus ret = eat_gps_status_get();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_gps_geo_fence_set(duk_context *ctx) {
	u8 geo_id = (u8) duk_get_int(ctx, 0);
	EatGpsGeoFenceMode geo_mode = (EatGpsGeoFenceMode) duk_get_int(ctx, 1);
	EatGpsGeoFenceShape geo_shape = (EatGpsGeoFenceShape) duk_get_int(ctx, 2);
	double lat0 = (double) duk_get_int(ctx, 3);
	double lng0 = (double) duk_get_int(ctx, 4);
	double lat1_or_radius = (double) duk_get_int(ctx, 5);
	double lng1 = (double) duk_get_int(ctx, 6);
	double lat2 = (double) duk_get_int(ctx, 7);
	double lng2 = (double) duk_get_int(ctx, 8);
	double lat3 = (double) duk_get_int(ctx, 9);
	double lng3 = (double) duk_get_int(ctx, 10);
	eat_bool ret = eat_gps_geo_fence_set(geo_id, geo_mode, geo_shape, lat0, lng0, lat1_or_radius, lng1, lat2, lng2, lat3, lng3);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_gps_geo_fence_read(duk_context *ctx) {
	u8 geo_id = (u8) duk_get_int(ctx, 0);
	char strbuf[2048] = {0};
	u32 size = (u32) duk_get_int(ctx, 2);
	eat_bool ret = eat_gps_geo_fence_read(geo_id, (char *)strbuf, size);
	duk_push_string(ctx, strbuf);
	return 1;
}

duk_ret_t _eat_gps_register_msg_proc_callback(duk_context *ctx) {
	GPS_Msg_Proc_Ind gps_msg_callback = (GPS_Msg_Proc_Ind) duk_get_int(ctx, 0);
	eat_bool ret = eat_gps_register_msg_proc_callback(gps_msg_callback);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_fs_Open(duk_context *ctx) {
  wchar_t wfname[2048];
	const unsigned short *FileName = (const unsigned short *) ctow(duk_get_string(ctx, 0), (wchar_t *)&wfname);
	UINT Flag = (UINT) duk_get_int(ctx, 1);
	int ret = eat_fs_Open(FileName, Flag);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_fs_Close(duk_context *ctx) {
	FS_HANDLE FileHandle = (FS_HANDLE) duk_get_int(ctx, 0);
	int ret = eat_fs_Close(FileHandle);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_fs_Read(duk_context *ctx) {
  char *DataPtr;
  int ret;
	FS_HANDLE FileHandle = (FS_HANDLE) duk_get_int(ctx, 0);
	UINT Length = (UINT) duk_get_int(ctx, 1);
	UINT Read = 0;
  DataPtr=eat_mem_alloc(Length+1);
	ret = eat_fs_Read(FileHandle, DataPtr, Length, &Read);
  DataPtr[Length+1]='\0';
	duk_push_string(ctx, DataPtr);
  eat_mem_free(DataPtr);
	return 1;
}

duk_ret_t _eat_fs_Write(duk_context *ctx) {
	FS_HANDLE FileHandle = (FS_HANDLE) duk_get_int(ctx, 0);
	char *DataPtr = (char *) duk_get_string(ctx, 1);
	UINT Length = (UINT) duk_get_int(ctx, 2);
	UINT Written =0;
	int ret = eat_fs_Write(FileHandle, DataPtr, Length, &Written);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_fs_Seek(duk_context *ctx) {
	FS_HANDLE FileHandle = (FS_HANDLE) duk_get_int(ctx, 0);
	int Offset = (int) duk_get_int(ctx, 1);
	int Whence = (int) duk_get_int(ctx, 2);
	int ret = eat_fs_Seek(FileHandle, Offset, Whence);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_fs_Commit(duk_context *ctx) {
	FS_HANDLE FileHandle = (FS_HANDLE) duk_get_int(ctx, 0);
	int ret = eat_fs_Commit(FileHandle);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_fs_GetFileSize(duk_context *ctx) {
	FS_HANDLE FileHandle = (FS_HANDLE) duk_get_int(ctx, 0);
	UINT Size = 0;
	int ret = eat_fs_GetFileSize(FileHandle, &Size);
	duk_push_int(ctx, Size);
	return 1;
}

duk_ret_t _eat_fs_GetFilePosition(duk_context *ctx) {
	FS_HANDLE FileHandle = (FS_HANDLE) duk_get_int(ctx, 0);
	UINT Position = 0;
	int ret = eat_fs_GetFilePosition(FileHandle, &Position);
	duk_push_int(ctx, Position);
	return 1;
}

duk_ret_t _eat_fs_GetAttributes(duk_context *ctx) {
	const WCHAR *FileName = (const WCHAR *) duk_get_int(ctx, 0);
	int ret = eat_fs_GetAttributes(FileName);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_fs_SetAttributes(duk_context *ctx) {
	const WCHAR *FileName = (const WCHAR *) duk_get_int(ctx, 0);
	BYTE Attributes = (BYTE) duk_get_int(ctx, 1);
	int ret = eat_fs_SetAttributes(FileName, Attributes);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_fs_Delete(duk_context *ctx) {
	const WCHAR *FileName = (const WCHAR *) duk_get_int(ctx, 0);
	int ret = eat_fs_Delete(FileName);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_fs_CreateDir(duk_context *ctx) {
	const WCHAR *FileName = (const WCHAR *) duk_get_int(ctx, 0);
	int ret = eat_fs_CreateDir(FileName);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_fs_Truncate(duk_context *ctx) {
	FS_HANDLE FileHandle = (FS_HANDLE) duk_get_int(ctx, 0);
	int ret = eat_fs_Truncate(FileHandle);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_fs_RemoveDir(duk_context *ctx) {
	const WCHAR *DirName = (const WCHAR *) duk_get_int(ctx, 0);
	int ret = eat_fs_RemoveDir(DirName);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_fs_GetFolderSize(duk_context *ctx) {
	const WCHAR *FullPath = (const WCHAR *) duk_get_int(ctx, 0);
	int ret = eat_fs_GetFolderSize(FullPath);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_fs_GetDiskFreeSize(duk_context *ctx) {
	EatDisk_enum Disk = (EatDisk_enum) duk_get_int(ctx, 0);
	SINT64 Size = 0;
	int ret = eat_fs_GetDiskFreeSize(Disk, &Size);
	duk_push_int(ctx, Size);
	return 1;
}

duk_ret_t _eat_fs_GetDiskSize(duk_context *ctx) {
	EatDisk_enum Disk = (EatDisk_enum) duk_get_int(ctx, 0);
	SINT64 Size = 0;
	int ret = eat_fs_GetDiskSize(Disk, &Size);
	duk_push_int(ctx, Size);
	return 1;
}

duk_ret_t _eat_fs_Rename(duk_context *ctx) {
  wchar_t wfname[100];
  wchar_t wfname2[100];
	const WCHAR *FileName = (const WCHAR *) ctow(duk_get_string(ctx, 0), (wchar_t *)&wfname);
  const WCHAR *NewName = (const WCHAR *) ctow(duk_get_string(ctx, 1), (wchar_t *)&wfname2);
	int ret = eat_fs_Rename(wfname, wfname2);
	duk_push_int(ctx, ret);
	return 1;
}
/*
duk_ret_t _eat_fs_FindFirst(duk_context *ctx) {
	const WCHAR *NamePattern = (const WCHAR *) duk_get_int(ctx, 0);
	BYTE Attr = (BYTE) duk_get_int(ctx, 1);
	BYTE AttrMask = (BYTE) duk_get_int(ctx, 2);
	EAT_FS_DOSDirEntry *FileInfo = (EAT_FS_DOSDirEntry) duk_get_int(ctx, 3);
	WCHAR *FileName = (WCHAR) duk_get_int(ctx, 4);
	UINT MaxLength = (UINT) duk_get_int(ctx, 5);
	int ret = eat_fs_FindFirst(NamePattern, Attr, AttrMask, *FileInfo, *FileName, MaxLength);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_fs_FindNext(duk_context *ctx) {
	FS_HANDLE FileHandle = (FS_HANDLE) duk_get_int(ctx, 0);
	EAT_FS_DOSDirEntry *FileInfo = (EAT_FS_DOSDirEntry) duk_get_int(ctx, 1);
	WCHAR *FileName = (WCHAR) duk_get_int(ctx, 2);
	UINT MaxLength = (UINT) duk_get_int(ctx, 3);
	int ret = eat_fs_FindNext(FileHandle, *FileInfo, *FileName, MaxLength);
	duk_push_int(ctx, ret);
	return 1;
}
*/

duk_ret_t _eat_fs_FindClose(duk_context *ctx) {
	FS_HANDLE FileHandle = (FS_HANDLE) duk_get_int(ctx, 0);
	int ret = eat_fs_FindClose(FileHandle);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_fs_XDelete(duk_context *ctx) {
	const WCHAR *FullPath = (const WCHAR *) duk_get_int(ctx, 0);
	UINT Flag = (UINT) duk_get_int(ctx, 1);
	int ret = eat_fs_XDelete(FullPath, Flag);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_fs_Abort(duk_context *ctx) {
	UINT ActionHandle = (UINT) duk_get_int(ctx, 0);
	int ret = eat_fs_Abort(ActionHandle);
	duk_push_int(ctx, ret);
	return 1;
}
/*
duk_ret_t _eat_flash_erase(duk_context *ctx) {
	const void *address = (const void) duk_get_int(ctx, 0);
	unsigned int size = (unsigned int) duk_get_int(ctx, 1);
	eat_bool ret = eat_flash_erase(*address, size);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_flash_write(duk_context *ctx) {
	const void *address = (const void) duk_get_int(ctx, 0);
	const void *data = (const void) duk_get_int(ctx, 1);
	unsigned int len = (unsigned int) duk_get_int(ctx, 2);
	eat_bool ret = eat_flash_write(*address, *data, len);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_flash_read(duk_context *ctx) {
	void *buffer = (void) duk_get_int(ctx, 0);
	const void *address = (const void) duk_get_int(ctx, 1);
	unsigned int len = (unsigned int) duk_get_int(ctx, 2);
	void * ret = eat_flash_read(*buffer, *address, len);
	duk_push_int(ctx, ret);
	return 1;
}
*/

duk_ret_t _eat_get_flash_block_size(duk_context *ctx) {
	unsigned int ret = eat_get_flash_block_size();
	duk_push_int(ctx, ret);
	return 1;
}
/*
duk_ret_t _eat_update_app(duk_context *ctx) {
	const void *app_code_addr = (const void) duk_get_int(ctx, 0);
	const void *app_code_new_addr = (const void) duk_get_int(ctx, 1);
	unsigned int len = (unsigned int) duk_get_int(ctx, 2);
	EatPinName_enum pin_wd = (EatPinName_enum) duk_get_int(ctx, 3);
	EatPinName_enum pin_led = (EatPinName_enum) duk_get_int(ctx, 4);
	eat_bool lcd_bl = (eat_bool) duk_get_int(ctx, 5);
	void ret = eat_update_app(*app_code_addr, *app_code_new_addr, len, pin_wd, pin_led, lcd_bl);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_update_app_ok(duk_context *ctx) {
	void ret = eat_update_app_ok();
	duk_push_int(ctx, ret);
	return 1;
}
*/

duk_ret_t _eat_get_app_base_addr(duk_context *ctx) {
	unsigned long ret = eat_get_app_base_addr();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_get_app_space(duk_context *ctx) {
	unsigned long ret = eat_get_app_space();
	duk_push_int(ctx, ret);
	return 1;
}
/*
duk_ret_t _eat_audio_play_tone_id(duk_context *ctx) {
	EatAudioToneId_enum id = (EatAudioToneId_enum) duk_get_int(ctx, 0);
	EatAudioStyle_enum style = (EatAudioStyle_enum) duk_get_int(ctx, 1);
	unsigned char vol = (unsigned char) duk_get_int(ctx, 2);
	EatAudioPath_enum path = (EatAudioPath_enum) duk_get_int(ctx, 3);
	eat_bool ret = eat_audio_play_tone_id(id, style, vol, path);
	duk_push_int(ctx, ret);
	return 1;
}
*/

duk_ret_t _eat_audio_stop_tone_id(duk_context *ctx) {
	EatAudioToneId_enum id = (EatAudioToneId_enum) duk_get_int(ctx, 0);
	eat_bool ret = eat_audio_stop_tone_id(id);
	duk_push_int(ctx, ret);
	return 1;
}
/*
duk_ret_t _eat_audio_play_data(duk_context *ctx) {
	const void *audio_data = (const void) duk_get_int(ctx, 0);
	unsigned int len = (unsigned int) duk_get_int(ctx, 1);
	EatAudioFormat_enum format = (EatAudioFormat_enum) duk_get_int(ctx, 2);
	EatAudioStyle_enum style = (EatAudioStyle_enum) duk_get_int(ctx, 3);
	unsigned char vol = (unsigned char) duk_get_int(ctx, 4);
	EatAudioPath_enum path = (EatAudioPath_enum) duk_get_int(ctx, 5);
	eat_bool ret = eat_audio_play_data(*audio_data, len, format, style, vol, path);
	duk_push_int(ctx, ret);
	return 1;
}
*/

duk_ret_t _eat_audio_stop_data(duk_context *ctx) {
	eat_bool ret = eat_audio_stop_data();
	duk_push_int(ctx, ret);
	return 1;
}
/*
duk_ret_t _eat_audio_set_custom_tone(duk_context *ctx) {
	const EatAudioToneTable_st *tone = (const EatAudioToneTable_st) duk_get_int(ctx, 0);
	eat_bool ret = eat_audio_set_custom_tone(*tone);
	duk_push_int(ctx, ret);
	return 1;
}
*/

duk_ret_t _eat_audio_play_file(duk_context *ctx) {
	unsigned short *filename = (unsigned short*) duk_get_int(ctx, 0);
	eat_bool repeat = (eat_bool) duk_get_int(ctx, 1);
	eat_med_callback handler = (eat_med_callback) duk_get_int(ctx, 2);
	unsigned char volume = (unsigned char) duk_get_int(ctx, 3);
	EatAudioPath_enum outputpath = (EatAudioPath_enum) duk_get_int(ctx, 4);
	eat_med_result ret = eat_audio_play_file(filename, repeat, handler, volume, outputpath);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_audio_stop_file(duk_context *ctx) {
	eat_bool ret = eat_audio_stop_file();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_audio_set_play_mode_in_call(duk_context *ctx) {
	eat_audio_play_mode mode = (eat_audio_play_mode) duk_get_int(ctx, 0);
	eat_bool ret = eat_audio_set_play_mode_in_call(mode);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_audio_get_play_mode_in_call(duk_context *ctx) {
	eat_audio_play_mode ret = eat_audio_get_play_mode_in_call();
	duk_push_int(ctx, ret);
	return 1;
}
/*
duk_ret_t _eat_get_event(duk_context *ctx) {
	EatEvent_st *event = (EatEvent_st) duk_get_int(ctx, 0);
	unsigned char ret = eat_get_event(*event);
	duk_push_int(ctx, ret);
	return 1;
}
*/

duk_ret_t _eat_get_event_num(duk_context *ctx) {
	unsigned int ret = eat_get_event_num();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_reset_module(duk_context *ctx) {
	eat_bool ret = eat_reset_module();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_get_version(duk_context *ctx) {
	const char * ret = eat_get_version();
	duk_push_string(ctx, ret);
	return 1;
}

duk_ret_t _eat_get_buildtime(duk_context *ctx) {
	const char * ret = eat_get_buildtime();
	duk_push_string(ctx, ret);
	return 1;
}

duk_ret_t _eat_get_buildno(duk_context *ctx) {
	const char * ret = eat_get_buildno();
	duk_push_string(ctx, ret);
	return 1;
}
/*
duk_ret_t _eat_get_event_for_user(duk_context *ctx) {
	EatUser_enum user = (EatUser_enum) duk_get_int(ctx, 0);
	EatEvent_st *event = (EatEvent_st) duk_get_int(ctx, 1);
	eat_bool ret = eat_get_event_for_user(user, *event);
	duk_push_int(ctx, ret);
	return 1;
}
*/

duk_ret_t _eat_get_event_num_for_user(duk_context *ctx) {
	EatUser_enum user = (EatUser_enum) duk_get_int(ctx, 0);
	unsigned int ret = eat_get_event_num_for_user(user);
	duk_push_int(ctx, ret);
	return 1;
}
/*
duk_ret_t _eat_send_msg_to_user(duk_context *ctx) {
	EatUser_enum user_src = (EatUser_enum) duk_get_int(ctx, 0);
	EatUser_enum user_dst = (EatUser_enum) duk_get_int(ctx, 1);
	eat_bool use_point = (eat_bool) duk_get_int(ctx, 2);
	unsigned char len = (unsigned char) duk_get_int(ctx, 3);
	const unsigned char *data = (const unsigned char) duk_get_int(ctx, 4);
	const unsigned char **data_p = (const unsigned char) duk_get_int(ctx, 5);
	eat_bool ret = eat_send_msg_to_user(user_src, user_dst, use_point, len, *data, **data_p);
	duk_push_int(ctx, ret);
	return 1;
}
*/

duk_ret_t _eat_sleep_enable(duk_context *ctx) {
	eat_bool en = (eat_bool) duk_get_int(ctx, 0);
	eat_bool ret = eat_sleep_enable(en);
	duk_push_int(ctx, ret);
	return 1;
}
/*
duk_ret_t _eat_create_sem(duk_context *ctx) {
	unsigned char *sem_name_ptr = (unsigned char) duk_get_int(ctx, 0);
	int initial_count = (int) duk_get_int(ctx, 1);
	EatSemId_st ret = eat_create_sem(*sem_name_ptr, initial_count);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_sem_get(duk_context *ctx) {
	EatSemId_st sem_id = (EatSemId_st) duk_get_int(ctx, 0);
	EatWaitMode_enum wait_mode = (EatWaitMode_enum) duk_get_int(ctx, 1);
	eat_bool ret = eat_sem_get(sem_id, wait_mode);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_sem_put(duk_context *ctx) {
	EatSemId_st sem_id = (EatSemId_st) duk_get_int(ctx, 0);
	void ret = eat_sem_put(sem_id);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_sem_query(duk_context *ctx) {
	EatSemId_st sem_id = (EatSemId_st) duk_get_int(ctx, 0);
	int ret = eat_sem_query(sem_id);
	duk_push_int(ctx, ret);
	return 1;
}
*/

duk_ret_t _eat_power_down(duk_context *ctx) {
	eat_power_down();
	return 1;
}

duk_ret_t _eat_timer_start(duk_context *ctx) {
	EatTimer_enum id = (EatTimer_enum) duk_get_int(ctx, 0);
	unsigned int period = (unsigned int) duk_get_int(ctx, 1);
	eat_bool ret = eat_timer_start(id, period);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_timer_stop(duk_context *ctx) {
	EatTimer_enum id = (EatTimer_enum) duk_get_int(ctx, 0);
	eat_bool ret = eat_timer_stop(id);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_get_current_time(duk_context *ctx) {
	unsigned int ret = eat_get_current_time();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_get_duration_ms(duk_context *ctx) {
	unsigned int previous_time = (unsigned int) duk_get_int(ctx, 0);
	unsigned int ret = eat_get_duration_ms(previous_time);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_get_duration_us(duk_context *ctx) {
	unsigned int previous_time = (unsigned int) duk_get_int(ctx, 0);
	unsigned int ret = eat_get_duration_us(previous_time);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_sleep(duk_context *ctx) {
	unsigned int delay = (unsigned int) duk_get_int(ctx, 0);
	eat_sleep(delay);
	return 1;
}
/*
duk_ret_t _eat_get_rtc(duk_context *ctx) {
	EatRtc_st *rtc = (EatRtc_st) duk_get_int(ctx, 0);
	eat_bool ret = eat_get_rtc(*rtc);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_set_rtc(duk_context *ctx) {
	const EatRtc_st *rtc = (const EatRtc_st) duk_get_int(ctx, 0);
	eat_bool ret = eat_set_rtc(*rtc);
	duk_push_int(ctx, ret);
	return 1;
}
*/

duk_ret_t _eat_gpt_start(duk_context *ctx) {
	unsigned int period = (unsigned int) duk_get_int(ctx, 0);
	eat_bool loop = (eat_bool) duk_get_int(ctx, 1);
	eat_gpt_callback_func callback = (eat_gpt_callback_func) duk_get_int(ctx, 2);
	eat_bool ret = eat_gpt_start(period, loop, callback);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_gpt_stop(duk_context *ctx) {
	eat_bool ret = eat_gpt_stop();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_watchdog_start(duk_context *ctx) {
	u32 exp_time = (u32) duk_get_int(ctx, 0);
	u8 type = (u8) duk_get_int(ctx, 1);
	eat_bool ret = eat_watchdog_start(exp_time, type);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_watchdog_feed(duk_context *ctx) {
	eat_bool ret = eat_watchdog_feed();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_watchdog_stop(duk_context *ctx) {
	eat_bool ret = eat_watchdog_stop();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_uart_open(duk_context *ctx) {
	EatUart_enum uart = (EatUart_enum) duk_get_int(ctx, 0);
	eat_bool ret = eat_uart_open(uart);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_uart_close(duk_context *ctx) {
	EatUart_enum uart = (EatUart_enum) duk_get_int(ctx, 0);
	eat_bool ret = eat_uart_close(uart);
	duk_push_int(ctx, ret);
	return 1;
}
/*
duk_ret_t _eat_uart_set_config(duk_context *ctx) {
	EatUart_enum uart = (EatUart_enum) duk_get_int(ctx, 0);
	const EatUartConfig_st *config = (const EatUartConfig_st) duk_get_int(ctx, 1);
	eat_bool ret = eat_uart_set_config(uart, *config);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_uart_get_config(duk_context *ctx) {
	EatUart_enum uart = (EatUart_enum) duk_get_int(ctx, 0);
	EatUartConfig_st *config = (EatUartConfig_st) duk_get_int(ctx, 1);
	eat_bool ret = eat_uart_get_config(uart, *config);
	duk_push_int(ctx, ret);
	return 1;
}
*/

duk_ret_t _eat_uart_set_baudrate(duk_context *ctx) {
	EatUart_enum uart = (EatUart_enum) duk_get_int(ctx, 0);
	EatUartBaudrate baudrate = (EatUartBaudrate) duk_get_int(ctx, 1);
	eat_bool ret = eat_uart_set_baudrate(uart, baudrate);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_uart_get_baudrate(duk_context *ctx) {
	EatUart_enum uart = (EatUart_enum) duk_get_int(ctx, 0);
	EatUartBaudrate ret = eat_uart_get_baudrate(uart);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_uart_write(duk_context *ctx) {
	EatUart_enum uart = (EatUart_enum) duk_get_int(ctx, 0);
	const unsigned char *data = (const unsigned char *) duk_get_string(ctx, 1);
	unsigned short len = (unsigned short) duk_get_int(ctx, 2);
	unsigned short ret = eat_uart_write(uart, data, len);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_uart_read(duk_context *ctx) {
	EatUart_enum uart = (EatUart_enum) duk_get_int(ctx, 0);
	unsigned char data[2048]={0};
	unsigned short len = (unsigned short) duk_get_int(ctx, 1);
	unsigned short ret = eat_uart_read(uart, (unsigned char *)&data, len);
	duk_push_string(ctx, data);
	return 1;
}

duk_ret_t _eat_uart_set_debug(duk_context *ctx) {
	EatUart_enum uart = (EatUart_enum) duk_get_int(ctx, 0);
	eat_bool ret = eat_uart_set_debug(uart);
	duk_push_int(ctx, ret);
	return 1;
}
/*
duk_ret_t _eat_uart_set_debug_config(duk_context *ctx) {
	EatUartDebugMode_enum mode = (EatUartDebugMode_enum) duk_get_int(ctx, 0);
	EatUartConfig_st *cfg = (EatUartConfig_st) duk_get_int(ctx, 1);
	eat_bool ret = eat_uart_set_debug_config(mode, *cfg);
	duk_push_int(ctx, ret);
	return 1;
}
*/

duk_ret_t _eat_uart_set_at_port(duk_context *ctx) {
	EatUart_enum uart = (EatUart_enum) duk_get_int(ctx, 0);
	eat_bool ret = eat_uart_set_at_port(uart);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_uart_set_send_complete_event(duk_context *ctx) {
	EatUart_enum uart = (EatUart_enum) duk_get_int(ctx, 0);
	eat_bool send = (eat_bool) duk_get_int(ctx, 1);
	eat_bool ret = eat_uart_set_send_complete_event(uart, send);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_uart_get_send_complete_status(duk_context *ctx) {
	EatUart_enum uart = (EatUart_enum) duk_get_int(ctx, 0);
	eat_bool ret = eat_uart_get_send_complete_status(uart);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_uart_get_free_space(duk_context *ctx) {
	EatUart_enum uart = (EatUart_enum) duk_get_int(ctx, 0);
	unsigned short ret = eat_uart_get_free_space(uart);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_uart_control_by_app(duk_context *ctx) {
	eat_bool enable = (eat_bool) duk_get_int(ctx, 0);
	eat_uart_control_by_app(enable);
	return 0;
}

duk_ret_t _eat_uart_power_off(duk_context *ctx) {
	EatUart_enum uart = (EatUart_enum) duk_get_int(ctx, 0);
	eat_bool ret = eat_uart_power_off(uart);
	duk_push_int(ctx, ret);
	return 1;
}

static duk_ret_t _eat_trace(duk_context *ctx) {
  int nargs = duk_get_top(ctx);
  eat_trace((char *)duk_to_string(ctx, 0));
  return 0;  // no return value (= undefined) 
}

static void bear_notify_cb(cbm_bearer_state_enum state, u8 ip_addr[4]){
  char strbuf[1024];
  sprintf(strbuf, "if(typeof bear_notify_cb !== 'undefined'){bear_notify_cb(%d, '%d.%d.%d.%d');}", state, ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
  duk_eval_string_noresult(_ctx, strbuf);
}

duk_ret_t  _eat_gprs_bearer_open(duk_context *ctx) {
	u8 *apn = (u8 *) duk_get_string(ctx, 0);
	u8 *user = (u8 *) duk_get_string(ctx, 1);
	u8 *password = (u8 *) duk_get_string(ctx, 2);
	eat_bear_notify call_back = bear_notify_cb;//(eat_bear_notify) duk_get_int(ctx, 3);
	s8  ret = eat_gprs_bearer_open(apn, user, password, call_back);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t  _eat_gprs_bearer_hold(duk_context *ctx) {
	s8  ret = eat_gprs_bearer_hold();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t  _eat_gprs_bearer_release(duk_context *ctx) {
	s8  ret = eat_gprs_bearer_release();
	duk_push_int(ctx, ret);
	return 1;
}


duk_ret_t _eat_soc_create(duk_context *ctx) {
	socket_type_enum type = (socket_type_enum) duk_get_int(ctx, 0);
	u8 protocol = (u8) duk_get_int(ctx, 1);
	s8 ret = eat_soc_create(type, protocol);
	duk_push_int(ctx, ret);
	return 1;
}


duk_ret_t _eat_soc_connect(duk_context *ctx) {
  sockaddr_struct address={0};
  s8 ret=0;
	s8 s = (s8) duk_get_int(ctx, 0);
	const char *addr = duk_get_string(ctx, 1);
  int ip1,ip2,ip3,ip4,port;
  sscanf((const char *)addr, "%d.%d.%d.%d:%d", &ip1, &ip2, &ip3, &ip4, &port);
  address.sock_type = SOC_SOCK_STREAM;
  address.addr_len = 4;
  address.port = port;                /* TCP server port */
  address.addr[0]=ip1;                /* TCP server ip address */
  address.addr[1]=ip2;
  address.addr[2]=ip3;
  address.addr[3]=ip4;
  ret = eat_soc_connect(s, &address);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_soc_setsockopt(duk_context *ctx) {
	s8 s = (s8) duk_get_int(ctx, 0);
	u32 option = (u32) duk_get_int(ctx, 1);
	int val = duk_get_int(ctx, 2);
	s8 ret = eat_soc_setsockopt(s, option, &val, sizeof(val));
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_soc_getsockopt(duk_context *ctx) {
	s8 s = (s8) duk_get_int(ctx, 0);
	u32 option = (u32) duk_get_int(ctx, 1);
	int val = 0;
	s8 ret = eat_soc_getsockopt(s, option, &val, sizeof(val));
	duk_push_int(ctx, val);
	return 1;
}

duk_ret_t _eat_soc_bind(duk_context *ctx) {
	sockaddr_struct address={0};
	s8 s = (s8) duk_get_int(ctx, 0);
  s8 ret =0;
	const char *addr = duk_get_string(ctx, 1);
  int ip1,ip2,ip3,ip4,port;
  sscanf((const char *)addr, "%d.%d.%d.%d:%d", &ip1, &ip2, &ip3, &ip4, &port);
  address.sock_type = SOC_SOCK_STREAM;
  address.addr_len = 4;
  address.port = port;                /* TCP server port */
  address.addr[0]=ip1;                /* TCP server ip address */
  address.addr[1]=ip2;
  address.addr[2]=ip3;
  address.addr[3]=ip4;
	ret = eat_soc_bind(s, &address);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_soc_listen(duk_context *ctx) {
	s8 s = (s8) duk_get_int(ctx, 0);
	u8 backlog = (u8) duk_get_int(ctx, 1);
	s8 ret = eat_soc_listen(s, backlog);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_soc_accept(duk_context *ctx) {
	sockaddr_struct address={0};
  s8 ret=0;
	s8 s = (s8) duk_get_int(ctx, 0);
	const char *addr = duk_get_string(ctx, 1);
  int ip1,ip2,ip3,ip4,port;
  sscanf((const char *)addr, "%d.%d.%d.%d:%d", &ip1, &ip2, &ip3, &ip4, &port);
  address.sock_type = SOC_SOCK_STREAM;
  address.addr_len = 4;
  address.port = port;                /* TCP server port */
  address.addr[0]=ip1;                /* TCP server ip address */
  address.addr[1]=ip2;
  address.addr[2]=ip3;
  address.addr[3]=ip4;
	ret = eat_soc_accept(s, &address);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_soc_send(duk_context *ctx) {
	s8 s = (s8) duk_get_int(ctx, 0);
	const char *buf = duk_get_string(ctx, 1);
	s32 len = (s32) duk_get_int(ctx, 2);
	s32 ret = eat_soc_send(s, buf, len);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_soc_recv(duk_context *ctx) {
	s8 s = (s8) duk_get_int(ctx, 0);
	char buf[2048]={0};
	s32 len = (s32) duk_get_int(ctx, 1);
	s32 ret = eat_soc_recv(s, &buf, len);
	duk_push_string(ctx, buf);
	return 1;
}

duk_ret_t _eat_soc_sendto(duk_context *ctx) {
  sockaddr_struct address={0};
  s32 ret=0;
	s8 s = (s8) duk_get_int(ctx, 0);
	const char *buf = duk_get_string(ctx, 1);
  int len = duk_get_int(ctx,2);
	const char *addr = duk_get_string(ctx, 3);
  int ip1,ip2,ip3,ip4,port;
  sscanf((const char *)addr, "%d.%d.%d.%d:%d", &ip1, &ip2, &ip3, &ip4, &port);
  address.sock_type = SOC_SOCK_STREAM;
  address.addr_len = 4;
  address.port = port;                /* TCP server port */
  address.addr[0]=ip1;                /* TCP server ip address */
  address.addr[1]=ip2;
  address.addr[2]=ip3;
  address.addr[3]=ip4;
	ret = eat_soc_sendto(s, buf, len, &address);
	duk_push_int(ctx, ret);
	return 1;
}
/*
duk_ret_t _eat_soc_recvfrom(duk_context *ctx) {
	s8 s = (s8) duk_get_int(ctx, 0);
	void *buf = (void) duk_get_int(ctx, 1);
	s32 len = (s32) duk_get_int(ctx, 2);
	sockaddr_struct *fromaddr = (sockaddr_struct) duk_get_int(ctx, 3);
	s32 ret = eat_soc_recvfrom(s, *buf, len, *fromaddr);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_soc_select(duk_context *ctx) {
	u8 ndesc = (u8) duk_get_int(ctx, 0);
	soc_fd_set *in = (soc_fd_set) duk_get_int(ctx, 1);
	soc_fd_set *out = (soc_fd_set) duk_get_int(ctx, 2);
	soc_fd_set *ex = (soc_fd_set) duk_get_int(ctx, 3);
	soc_timeval_struct *tv = (soc_timeval_struct) duk_get_int(ctx, 4);
	s8 ret = eat_soc_select(ndesc, *in, *out, *ex, *tv);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_soc_fd_set(duk_context *ctx) {
	u8 s = (u8) duk_get_int(ctx, 0);
	soc_fd_set *p = (soc_fd_set) duk_get_int(ctx, 1);
	void ret = eat_soc_fd_set(s, *p);
	duk_push_int(ctx, ret);
	return 1;
}
*/
/*
duk_ret_t _eat_soc_gethostbyname(duk_context *ctx) {
	const char *domain_name = (const char) duk_get_int(ctx, 0);
	u8 *addr = (u8) duk_get_int(ctx, 1);
	u8 *addr_len = (u8) duk_get_int(ctx, 2);
	u32 request_id = (u32) duk_get_int(ctx, 3);
	s8 ret = eat_soc_gethostbyname(*domain_name, *addr, *addr_len, request_id);
	duk_push_int(ctx, ret);
	return 1;
}
*/
/*
duk_ret_t _eat_soc_gethost_notify_register(duk_context *ctx) {
	eat_hostname_notify call_back = (eat_hostname_notify) duk_get_int(ctx, 0);
	eat_soc_gethost_notify_register(call_back);
	return 1;
}
*/
/*
duk_ret_t _eat_soc_getsockaddr(duk_context *ctx) {
	s8 s = (s8) duk_get_int(ctx, 0);
	eat_bool is_local = (eat_bool) duk_get_int(ctx, 1);
	sockaddr_struct *addr = (sockaddr_struct) duk_get_int(ctx, 2);
	s8 ret = eat_soc_getsockaddr(s, is_local, *addr);
	duk_push_int(ctx, ret);
	return 1;
}
*/

duk_ret_t _eat_soc_close(duk_context *ctx) {
	s8 s = (s8) duk_get_int(ctx, 0);
	s8 ret = eat_soc_close(s);
	duk_push_int(ctx, ret);
	return 1;
}


static void soc_notify_cb(s8 s,soc_event_enum event,eat_bool result, u16 ack_size){
  char strbuf[1024];
  sprintf(strbuf, "if(typeof soc_notify_cb !== 'undefined'){soc_notify_cb(%d, %d, %d, %d);}", s, event, result, ack_size);
  duk_eval_string_noresult(_ctx, strbuf);
}

static void eat_sms_read_cb(EatSmsReadCnf_st smsReadCnfContent)
{
  char * strbuf=eat_mem_alloc(1200);
  eat_trace("eat_sms_read_cb, msg=%s",smsReadCnfContent.data);
  eat_trace("eat_sms_read_cb, datetime=%s",smsReadCnfContent.datetime);
  eat_trace("eat_sms_read_cb, name=%s",smsReadCnfContent.name);
  eat_trace("eat_sms_read_cb, status=%d",smsReadCnfContent.status);
  eat_trace("eat_sms_read_cb, len=%d",smsReadCnfContent.len);
  eat_trace("eat_sms_read_cb, number=%s",smsReadCnfContent.number);
//log_hex((const char *)&smsReadCnfContent,1024);
 
  sprintf(strbuf, "if(typeof eat_sms_read_cb !== 'undefined'){eat_sms_read_cb({name: '%s', datetime: '%s', message:'%s', number:'%s', status:%d, len:%d});}", 
          smsReadCnfContent.name, smsReadCnfContent.datetime, smsReadCnfContent.data, smsReadCnfContent.number, smsReadCnfContent.status,smsReadCnfContent.len);
  duk_eval_string_noresult(sms_ctx, strbuf);
  eat_mem_free(strbuf);
  
}

duk_ret_t _eat_sms_read(duk_context *ctx) {
	int index = (int) duk_get_int(ctx, 0);
	s8 ret;
  sms_ctx=ctx;
  ret = eat_read_sms(index, eat_sms_read_cb);
	duk_push_int(ctx, ret);
	return 1;
}


duk_ret_t _eat_send_text_sms(duk_context *ctx) {
	const char * number = duk_get_string(ctx, 0);
  const char * message = duk_get_string(ctx, 1);
	s8 ret = eat_send_text_sms((u8*)number, (u8*)message);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_set_sms_sc(duk_context *ctx) {
	const char * number = duk_get_string(ctx, 0);
	s8 ret = eat_set_sms_sc((u8*)number);
	duk_push_int(ctx, ret);
	return 1;
}





static void eat_ftp_close(){
  eat_modem_write("AT+SAPBR=0,1\r\n", strlen("AT+SAPBR=0,1\r\n"));
}

static void eat_ftp_init(const char *apn, const char *username, const char *password){
  static char buff[500]={0};
  eat_modem_write("AT+SAPBR=3,1,CONTYPE,GPRS\r\n", strlen("AT+SAPBR=3,1,CONTYPE,GPRS\r\n"));
  eat_sleep(500);
  sprintf(buff,"AT+SAPBR=3,1,APN,\"%s\"\r\n", apn);
  eat_modem_write(buff, strlen(buff));
  eat_sleep(500);
  sprintf(buff,"AT+SAPBR=3,1,USER,\"%s\"\r\n", username);
  eat_modem_write(buff, strlen(buff));
  eat_sleep(500);
  sprintf(buff,"AT+SAPBR=3,1,PWD,\"%s\"\r\n", password);
  eat_modem_write(buff, strlen(buff));
  eat_sleep(500);
  eat_modem_write("AT+SAPBR=1,1\r\n", strlen("AT+SAPBR=1,1\r\n"));
  eat_sleep(500);
  eat_modem_write("AT+SAPBR=2,1\r\n", strlen("AT+SAPBR=2,1\r\n"));
}

static void eat_ftp_get(const char *server, const char *port, const char *username, const char *password, const char *filename,const char *localpath, u8 sd){
  static char buff[500]={0};
  eat_modem_write("AT+FTPCID=1\r\n", strlen("AT+FTPCID=1\r\n"));
  eat_sleep(500);
  sprintf(buff,"AT+FTPUN=\"%s\"\r\n", username);
  eat_modem_write(buff, strlen(buff));
  eat_sleep(500);
  sprintf(buff,"AT+FTPPW=\"%s\"\r\n", password);
  eat_modem_write(buff, strlen(buff));
  eat_sleep(500);
  sprintf(buff,"AT+FTPGETNAME=\"%s\"\r\n", filename);
  eat_modem_write(buff, strlen(buff));
  eat_sleep(500);
  sprintf(buff,"AT+FTPGETPATH=\"%s\"\r\n", localpath);
  eat_modem_write(buff, strlen(buff));
  eat_sleep(500);
  sprintf(buff,"AT+FTPSERV=\"%s\"\r\n", server);
  eat_modem_write(buff, strlen(buff));
  eat_sleep(500);
  sprintf(buff,"AT+FTPPORT=%s\r\n", port);
  eat_modem_write(buff, strlen(buff));
  eat_sleep(500);
  eat_modem_write("AT+FTPTIMEOUT=3\r\n", strlen("AT+FTPTIMEOUT=3\r\n"));
  eat_sleep(500);
  sprintf(buff,"AT+FTPGETTOFS=%d,\"%s\"\r\n", sd, filename);
  eat_modem_write(buff, strlen(buff));
  eat_sleep(500);
}

duk_ret_t _eat_ftp_init(duk_context *ctx) {
  const char *apn  = duk_get_string(ctx, 0);
  const char *username = duk_get_string(ctx, 1); 
  const char *password = duk_get_string(ctx, 2); 
	eat_ftp_init(apn, username, password);
	return 0;
}

duk_ret_t _eat_ftp_get(duk_context *ctx) {
  const char *server  = duk_get_string(ctx, 0);
  const char *port = duk_get_string(ctx, 1); 
  const char *username = duk_get_string(ctx, 2); 
  const char *password = duk_get_string(ctx, 3); 
  const char *filename = duk_get_string(ctx, 4);
  const char *localpath = duk_get_string(ctx, 5);
  const u8 sd = duk_get_int(ctx, 6);
	eat_ftp_get(server, port, username, password, filename, localpath, sd);
	return 0;
}

duk_ret_t _eat_ftp_close(duk_context *ctx) {
	eat_ftp_close();
	return 0;
}



static u8 eat_upgrade_app(const char *fileName){
  u8 *data;
  u32 APP_DATA_RUN_BASE;  //app run addr
  u32 APP_DATA_STORAGE_BASE;  //app data storage addr
  u32 app_space_value,size, fileHandle;
  eat_bool rc;
  u32 ret;
  wchar_t wfname[2048];
	const unsigned short *FileName = (const unsigned short *) ctow(fileName, (wchar_t *)&wfname);

  fileHandle = eat_fs_Open(FileName, FS_READ_ONLY);
  if(fileHandle<EAT_FS_NO_ERROR){
    eat_trace("unable to open file");
    return -1;
  }
  eat_fs_GetFileSize(fileHandle, &size);
  data = eat_mem_alloc(size);
  eat_fs_Read(fileHandle, data, size, &ret);
  eat_fs_Close(fileHandle);

  APP_DATA_RUN_BASE = eat_get_app_base_addr(); //get app addr
  eat_trace("APP_DATA_RUN_BASE : %#x",APP_DATA_RUN_BASE);

  app_space_value = eat_get_app_space();  //get app space size
  eat_trace("app_space_value : %#x", app_space_value);

  APP_DATA_STORAGE_BASE = APP_DATA_RUN_BASE + (app_space_value>>1);//second half is space use to storage app_upgrade_data

  rc = eat_flash_erase((void*)APP_DATA_STORAGE_BASE , size);//erase the flash to write new app_data_storage
  if(EAT_FALSE == rc)
  {
      eat_trace("Erase flash failed [0x%08x, %dKByte]", APP_DATA_STORAGE_BASE,  size / 1024);
      return rc;
  }

  rc = eat_flash_write((void*)APP_DATA_STORAGE_BASE , data , size);//write the new app_data_storage
  if(EAT_FALSE == rc)
  {
      eat_trace("Write Flash Failed.");
      return rc;
  }

  //upgrade app
  eat_update_app((void*)(APP_DATA_RUN_BASE),(void*)(APP_DATA_STORAGE_BASE), size, EAT_PIN_NUM, EAT_PIN_NUM,EAT_FALSE);
  return 1;
}

duk_ret_t _eat_upgrade_app(duk_context *ctx) {
	const char * filename = duk_get_string(ctx, 0);
	s8 ret = eat_upgrade_app(filename);
	duk_push_int(ctx, ret);
	return 1;
}






void duk_sms_init(Sms_New_Message_Ind eat_sms_new_message_cb, Sms_Ready_Ind eat_sms_ready_cb, 
                  Sms_Flash_Message_Ind eat_sms_flash_message_cb, Sms_Send_Completed eat_sms_send_cb ) {
	eat_set_sms_operation_mode(EAT_TRUE); //set sms operation as API mode
  eat_set_sms_format(EAT_TRUE); //set sms format as TEXT mode
  eat_set_sms_cnmi(2,1,0,0,0); //set sms cnmi parameter
  eat_set_sms_sc("+447797704000"); //set center number
  eat_set_sms_storage(EAT_ME, EAT_ME, EAT_ME); //set sms storage type
  eat_sms_register_new_message_callback(eat_sms_new_message_cb);
  eat_sms_register_sms_ready_callback(eat_sms_ready_cb);
  eat_sms_register_flash_message_callback(eat_sms_flash_message_cb);
  eat_sms_register_send_completed_callback(eat_sms_send_cb);
  eat_trace("sms init done");
}


duk_ret_t _eat_bt_power(duk_context *ctx) {
  static char buff[15]={0};
  s8 ret;
	const u8 pwr = duk_get_int(ctx, 0);
  sprintf(buff,"AT+BTPOWER=%d\r\n", pwr);
  ret = eat_modem_write(buff, strlen(buff));
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_bt_spp_write(duk_context *ctx) {
	const char * buff = duk_get_string(ctx, 0);
  s8 ret;
  eat_modem_write("AT+BTSPPSEND\r\n", strlen("AT+BTSPPSEND\r\n"));
  eat_modem_write(buff, strlen(buff));
  ret = eat_modem_write("\x1b", 1); //ESC char
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_mma_init(duk_context *ctx) {
	s8 ret = mma8452_init();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_mma_config(duk_context *ctx) {
  const u8 scale = duk_get_int(ctx, 0);
  const u8 datarate = duk_get_int(ctx, 1);
	u8 ret = mma8452_config(scale, datarate);
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_mma_read(duk_context *ctx) {
  duk_idx_t obj_idx;
  float accel[3]={0};  // Stores the 12-bit signed value
  //int rotaccel[3]={0};  // Stores the 12-bit signed value
  int accelCount[3];  // Stores the 12-bit signed value
  float accelG[3];
  int i,j;
  const u8 scale = duk_get_int(ctx, 0);

  //facciamo la media di 6 samples presi a 50ms
  for(i=0;i<6;i++){
      mma8452_read_accel(&accelCount[0]);
      for ( j=0; j<3; j++)
          accel[j]+=accelCount[j]/6;
      eat_sleep(50);
  }
  //rotate(&accel[0],&rotaccel[0]);
  
  for ( i=0; i<3; i++)
    accelG[i] = (float) accel[i]/((1<<12)/(2 * scale));  // get actual g value, this depends on scale being set

  eat_trace("eat_mma_read X:%f Y:%f Z:%f",accelG[0],accelG[1],accelG[2]);

  obj_idx = duk_push_object(ctx);
  duk_push_number(ctx, accelG[0]);
  duk_put_prop_string(ctx, obj_idx, "x");
  duk_push_number(ctx, accelG[1]);
  duk_put_prop_string(ctx, obj_idx, "y");
  duk_push_number(ctx, accelG[2]);
  duk_put_prop_string(ctx, obj_idx, "z");
	return 1;
}

duk_ret_t _eat_mma_read_byte(duk_context *ctx) {
  const u8 address = duk_get_int(ctx, 0);
  uint8_t ret = 0;
	mma8452_i2c_register_read(address, &ret, 1);
	duk_push_int(ctx, ret);
	return 1;
}



register_bindings(duk_context *ctx){
  _ctx=ctx;

  eat_soc_notify_register(soc_notify_cb);

  duk_push_c_function(ctx, _eat_mma_config, 2);
  duk_put_global_string(ctx, "eat_mma_config");

  duk_push_c_function(ctx, _eat_mma_init, 1);
  duk_put_global_string(ctx, "eat_mma_init");

  duk_push_c_function(ctx, _eat_mma_read_byte, 1);
  duk_put_global_string(ctx, "eat_mma_read_byte");

  duk_push_c_function(ctx, _eat_mma_read, 1);
  duk_put_global_string(ctx, "eat_mma_read");

  duk_push_c_function(ctx, _eat_trace, DUK_VARARGS);
  duk_put_global_string(ctx, "eat_trace");

  duk_push_c_function(ctx, _eat_set_sms_sc, 1);
  duk_put_global_string(ctx, "eat_set_sms_sc");

  duk_push_c_function(ctx, _eat_bt_power, 1);
  duk_put_global_string(ctx, "eat_bt_power");

  duk_push_c_function(ctx, _eat_bt_spp_write, 1);
  duk_put_global_string(ctx, "eat_bt_spp_write");

  duk_push_c_function(ctx, _eat_ftp_init, 3);
  duk_put_global_string(ctx, "eat_ftp_init");

  duk_push_c_function(ctx, _eat_ftp_get, 7);
  duk_put_global_string(ctx, "eat_ftp_get");

  duk_push_c_function(ctx, _eat_ftp_close, 0);
  duk_put_global_string(ctx, "eat_ftp_close");

  duk_push_c_function(ctx, _eat_sms_read, 1);
  duk_put_global_string(ctx, "eat_sms_read");

  duk_push_c_function(ctx, _eat_upgrade_app, 1);
  duk_put_global_string(ctx, "eat_upgrade_app");

  duk_push_c_function(ctx, _eat_send_text_sms, 2);
	duk_put_global_string(ctx, "eat_send_text_sms");
  
  duk_push_c_function(ctx, _eat_gprs_bearer_open, 4);
	duk_put_global_string(ctx, "eat_gprs_bearer_open");

	duk_push_c_function(ctx, _eat_gprs_bearer_hold, 0);
	duk_put_global_string(ctx, "eat_gprs_bearer_hold");

	duk_push_c_function(ctx, _eat_gprs_bearer_release, 0);
	duk_put_global_string(ctx, "eat_gprs_bearer_release");

	duk_push_c_function(ctx, _eat_soc_create, 2);
	duk_put_global_string(ctx, "eat_soc_create");

	duk_push_c_function(ctx, _eat_soc_connect, 2);
	duk_put_global_string(ctx, "eat_soc_connect");

	duk_push_c_function(ctx, _eat_soc_setsockopt, 4);
	duk_put_global_string(ctx, "eat_soc_setsockopt");

	duk_push_c_function(ctx, _eat_soc_getsockopt, 4);
	duk_put_global_string(ctx, "eat_soc_getsockopt");

	duk_push_c_function(ctx, _eat_soc_bind, 2);
	duk_put_global_string(ctx, "eat_soc_bind");

	duk_push_c_function(ctx, _eat_soc_listen, 2);
	duk_put_global_string(ctx, "eat_soc_listen");

	duk_push_c_function(ctx, _eat_soc_accept, 2);
	duk_put_global_string(ctx, "eat_soc_accept");

	duk_push_c_function(ctx, _eat_soc_send, 3);
	duk_put_global_string(ctx, "eat_soc_send");

	duk_push_c_function(ctx, _eat_soc_recv, 3);
	duk_put_global_string(ctx, "eat_soc_recv");

	duk_push_c_function(ctx, _eat_soc_sendto, 4);
	duk_put_global_string(ctx, "eat_soc_sendto");

	//duk_push_c_function(ctx, _eat_soc_recvfrom, 4);
	//duk_put_global_string(ctx, "eat_soc_recvfrom");

	//duk_push_c_function(ctx, _eat_soc_select, 5);
	//duk_put_global_string(ctx, "eat_soc_select");

	//duk_push_c_function(ctx, _eat_soc_fd_set, 2);
	//duk_put_global_string(ctx, "eat_soc_fd_set");

	//duk_push_c_function(ctx, _eat_soc_gethostbyname, 4);
	//duk_put_global_string(ctx, "eat_soc_gethostbyname");

	//duk_push_c_function(ctx, _eat_soc_gethost_notify_register, 1);
	//duk_put_global_string(ctx, "eat_soc_gethost_notify_register");

	//duk_push_c_function(ctx, _eat_soc_getsockaddr, 3);
	//duk_put_global_string(ctx, "eat_soc_getsockaddr");

	duk_push_c_function(ctx, _eat_soc_close, 1);
	duk_put_global_string(ctx, "eat_soc_close");

	duk_push_c_function(ctx, _eat_gpio_setup, 3);
	duk_put_global_string(ctx, "eat_gpio_setup");

	duk_push_c_function(ctx, _eat_gpio_write, 2);
	duk_put_global_string(ctx, "eat_gpio_write");

	duk_push_c_function(ctx, _eat_gpio_read, 1);
	duk_put_global_string(ctx, "eat_gpio_read");

	duk_push_c_function(ctx, _eat_gpio_set_pull, 3);
	duk_put_global_string(ctx, "eat_gpio_set_pull");

	duk_push_c_function(ctx, _eat_int_setup, 4);
	duk_put_global_string(ctx, "eat_int_setup");

	duk_push_c_function(ctx, _eat_int_release, 1);
	duk_put_global_string(ctx, "eat_int_release");

	duk_push_c_function(ctx, _eat_int_set_trigger, 2);
	duk_put_global_string(ctx, "eat_int_set_trigger");

	duk_push_c_function(ctx, _eat_spi_init, 5);
	duk_put_global_string(ctx, "eat_spi_init");

	duk_push_c_function(ctx, _eat_spi_write, 3);
	duk_put_global_string(ctx, "eat_spi_write");

	duk_push_c_function(ctx, _eat_spi_read, 2);
	duk_put_global_string(ctx, "eat_spi_read");

	duk_push_c_function(ctx, _eat_spi_write_read, 4);
	duk_put_global_string(ctx, "eat_spi_write_read");

	duk_push_c_function(ctx, _eat_lcd_light_sw, 2);
	duk_put_global_string(ctx, "eat_lcd_light_sw");

	duk_push_c_function(ctx, _eat_kpled_sw, 1);
	duk_put_global_string(ctx, "eat_kpled_sw");

	duk_push_c_function(ctx, _eat_adc_get, 3);
	duk_put_global_string(ctx, "eat_adc_get");

	duk_push_c_function(ctx, _eat_adc_get_ms, 3);
	duk_put_global_string(ctx, "eat_adc_get_ms");

	duk_push_c_function(ctx, _eat_get_adc_sync, 2);
	duk_put_global_string(ctx, "eat_get_adc_sync");

	duk_push_c_function(ctx, _eat_pwm_start, 2);
	duk_put_global_string(ctx, "eat_pwm_start");

	duk_push_c_function(ctx, _eat_pwm_stop, 0);
	duk_put_global_string(ctx, "eat_pwm_stop");

	duk_push_c_function(ctx, _eat_pin_set_mode, 2);
	duk_put_global_string(ctx, "eat_pin_set_mode");

	duk_push_c_function(ctx, _eat_poweroff_key_sw, 1);
	duk_put_global_string(ctx, "eat_poweroff_key_sw");

	duk_push_c_function(ctx, _eat_sim_detect_en, 1);
	duk_put_global_string(ctx, "eat_sim_detect_en");

	duk_push_c_function(ctx, _eat_i2c_open, 3);
	duk_put_global_string(ctx, "eat_i2c_open");

	duk_push_c_function(ctx, _eat_i2c_write, 3);
	duk_put_global_string(ctx, "eat_i2c_write");

	duk_push_c_function(ctx, _eat_i2c_read, 5);
	duk_put_global_string(ctx, "eat_i2c_read");

	duk_push_c_function(ctx, _eat_i2c_close, 1);
	duk_put_global_string(ctx, "eat_i2c_close");

	duk_push_c_function(ctx, _eat_usb_eint_register, 1);
	duk_put_global_string(ctx, "eat_usb_eint_register");

	duk_push_c_function(ctx, _eat_get_usb_status, 0);
	duk_put_global_string(ctx, "eat_get_usb_status");

	duk_push_c_function(ctx, _eat_get_module_temp_sync, 1);
	duk_put_global_string(ctx, "eat_get_module_temp_sync");

	//duk_push_c_function(ctx, _eat_get_module_temp_asyn, 1);
	//duk_put_global_string(ctx, "eat_get_module_temp_asyn");

	//duk_push_c_function(ctx, _eat_get_cbc, 1);
	//duk_put_global_string(ctx, "eat_get_cbc");

	duk_push_c_function(ctx, _eat_get_imei, 2);
	duk_put_global_string(ctx, "eat_get_imei");

	duk_push_c_function(ctx, _eat_get_imsi, 2);
	duk_put_global_string(ctx, "eat_get_imsi");

	duk_push_c_function(ctx, _eat_get_task_id, 0);
	duk_put_global_string(ctx, "eat_get_task_id");

	duk_push_c_function(ctx, _eat_network_get_creg, 0);
	duk_put_global_string(ctx, "eat_network_get_creg");

	duk_push_c_function(ctx, _eat_network_get_ber, 0);
	duk_put_global_string(ctx, "eat_network_get_ber");

	duk_push_c_function(ctx, _eat_network_get_csq, 0);
	duk_put_global_string(ctx, "eat_network_get_csq");

	duk_push_c_function(ctx, _eat_network_get_cgatt, 0);
	duk_put_global_string(ctx, "eat_network_get_cgatt");

	duk_push_c_function(ctx, _eat_modem_write, 1);
	duk_put_global_string(ctx, "eat_modem_write");

	duk_push_c_function(ctx, _eat_modem_read, 2);
	duk_put_global_string(ctx, "eat_modem_read");

	duk_push_c_function(ctx, _eat_modem_set_poweron_urc_dir, 1);
	duk_put_global_string(ctx, "eat_modem_set_poweron_urc_dir");

	duk_push_c_function(ctx, _eat_gps_power_req, 1);
	duk_put_global_string(ctx, "eat_gps_power_req");

	duk_push_c_function(ctx, _eat_gps_power_status, 0);
	duk_put_global_string(ctx, "eat_gps_power_status");

	duk_push_c_function(ctx, _eat_gps_start_mode_get, 0);
	duk_put_global_string(ctx, "eat_gps_start_mode_get");

	duk_push_c_function(ctx, _eat_gps_start_mode_set, 1);
	duk_put_global_string(ctx, "eat_gps_start_mode_set");

	duk_push_c_function(ctx, _eat_gps_nmea_info_output, 3);
	duk_put_global_string(ctx, "eat_gps_nmea_info_output");

	duk_push_c_function(ctx, _eat_gps_sleep_set, 1);
	duk_put_global_string(ctx, "eat_gps_sleep_set");

	duk_push_c_function(ctx, _eat_gps_sleep_read, 0);
	duk_put_global_string(ctx, "eat_gps_sleep_read");

	duk_push_c_function(ctx, _eat_gps_status_get, 0);
	duk_put_global_string(ctx, "eat_gps_status_get");

	duk_push_c_function(ctx, _eat_gps_geo_fence_set, 11);
	duk_put_global_string(ctx, "eat_gps_geo_fence_set");

	duk_push_c_function(ctx, _eat_gps_geo_fence_read, 3);
	duk_put_global_string(ctx, "eat_gps_geo_fence_read");

	duk_push_c_function(ctx, _eat_gps_register_msg_proc_callback, 1);
	duk_put_global_string(ctx, "eat_gps_register_msg_proc_callback");

	duk_push_c_function(ctx, _eat_fs_Open, 2);
	duk_put_global_string(ctx, "eat_fs_Open");

	duk_push_c_function(ctx, _eat_fs_Close, 1);
	duk_put_global_string(ctx, "eat_fs_Close");

	duk_push_c_function(ctx, _eat_fs_Read, 4);
	duk_put_global_string(ctx, "eat_fs_Read");

	duk_push_c_function(ctx, _eat_fs_Write, 4);
	duk_put_global_string(ctx, "eat_fs_Write");

	duk_push_c_function(ctx, _eat_fs_Seek, 3);
	duk_put_global_string(ctx, "eat_fs_Seek");

	duk_push_c_function(ctx, _eat_fs_Commit, 1);
	duk_put_global_string(ctx, "eat_fs_Commit");

	duk_push_c_function(ctx, _eat_fs_GetFileSize, 2);
	duk_put_global_string(ctx, "eat_fs_GetFileSize");

	duk_push_c_function(ctx, _eat_fs_GetFilePosition, 2);
	duk_put_global_string(ctx, "eat_fs_GetFilePosition");

	duk_push_c_function(ctx, _eat_fs_GetAttributes, 1);
	duk_put_global_string(ctx, "eat_fs_GetAttributes");

	duk_push_c_function(ctx, _eat_fs_SetAttributes, 2);
	duk_put_global_string(ctx, "eat_fs_SetAttributes");

	duk_push_c_function(ctx, _eat_fs_Delete, 1);
	duk_put_global_string(ctx, "eat_fs_Delete");

	duk_push_c_function(ctx, _eat_fs_CreateDir, 1);
	duk_put_global_string(ctx, "eat_fs_CreateDir");

	duk_push_c_function(ctx, _eat_fs_Truncate, 1);
	duk_put_global_string(ctx, "eat_fs_Truncate");

	duk_push_c_function(ctx, _eat_fs_RemoveDir, 1);
	duk_put_global_string(ctx, "eat_fs_RemoveDir");

	duk_push_c_function(ctx, _eat_fs_GetFolderSize, 1);
	duk_put_global_string(ctx, "eat_fs_GetFolderSize");

	duk_push_c_function(ctx, _eat_fs_GetDiskFreeSize, 2);
	duk_put_global_string(ctx, "eat_fs_GetDiskFreeSize");

	duk_push_c_function(ctx, _eat_fs_GetDiskSize, 2);
	duk_put_global_string(ctx, "eat_fs_GetDiskSize");

	duk_push_c_function(ctx, _eat_fs_Rename, 2);
	duk_put_global_string(ctx, "eat_fs_Rename");

	//duk_push_c_function(ctx, _eat_fs_FindFirst, 6);
	//duk_put_global_string(ctx, "eat_fs_FindFirst");

	//duk_push_c_function(ctx, _eat_fs_FindNext, 4);
	//duk_put_global_string(ctx, "eat_fs_FindNext");

	duk_push_c_function(ctx, _eat_fs_FindClose, 1);
	duk_put_global_string(ctx, "eat_fs_FindClose");

	duk_push_c_function(ctx, _eat_fs_XDelete, 2);
	duk_put_global_string(ctx, "eat_fs_XDelete");

	duk_push_c_function(ctx, _eat_fs_Abort, 1);
	duk_put_global_string(ctx, "eat_fs_Abort");

	//duk_push_c_function(ctx, _eat_flash_erase, 2);
	//duk_put_global_string(ctx, "eat_flash_erase");

	//duk_push_c_function(ctx, _eat_flash_write, 3);
	//duk_put_global_string(ctx, "eat_flash_write");

	//duk_push_c_function(ctx, _eat_flash_read, 3);
	//duk_put_global_string(ctx, "eat_flash_read");

	duk_push_c_function(ctx, _eat_get_flash_block_size, 0);
	duk_put_global_string(ctx, "eat_get_flash_block_size");

	//duk_push_c_function(ctx, _eat_update_app, 6);
	//duk_put_global_string(ctx, "eat_update_app");

	//duk_push_c_function(ctx, _eat_update_app_ok, 0);
	//duk_put_global_string(ctx, "eat_update_app_ok");

	duk_push_c_function(ctx, _eat_get_app_base_addr, 0);
	duk_put_global_string(ctx, "eat_get_app_base_addr");

	duk_push_c_function(ctx, _eat_get_app_space, 0);
	duk_put_global_string(ctx, "eat_get_app_space");

	//duk_push_c_function(ctx, _eat_audio_play_tone_id, 4);
	//duk_put_global_string(ctx, "eat_audio_play_tone_id");

	duk_push_c_function(ctx, _eat_audio_stop_tone_id, 1);
	duk_put_global_string(ctx, "eat_audio_stop_tone_id");

	//duk_push_c_function(ctx, _eat_audio_play_data, 6);
	//duk_put_global_string(ctx, "eat_audio_play_data");

	duk_push_c_function(ctx, _eat_audio_stop_data, 0);
	duk_put_global_string(ctx, "eat_audio_stop_data");

	//duk_push_c_function(ctx, _eat_audio_set_custom_tone, 1);
	//duk_put_global_string(ctx, "eat_audio_set_custom_tone");

	duk_push_c_function(ctx, _eat_audio_play_file, 5);
	duk_put_global_string(ctx, "eat_audio_play_file");

	duk_push_c_function(ctx, _eat_audio_stop_file, 0);
	duk_put_global_string(ctx, "eat_audio_stop_file");

	duk_push_c_function(ctx, _eat_audio_set_play_mode_in_call, 1);
	duk_put_global_string(ctx, "eat_audio_set_play_mode_in_call");

	duk_push_c_function(ctx, _eat_audio_get_play_mode_in_call, 0);
	duk_put_global_string(ctx, "eat_audio_get_play_mode_in_call");

	//duk_push_c_function(ctx, _eat_get_event, 1);
	//duk_put_global_string(ctx, "eat_get_event");

	duk_push_c_function(ctx, _eat_get_event_num, 0);
	duk_put_global_string(ctx, "eat_get_event_num");

	duk_push_c_function(ctx, _eat_reset_module, 0);
	duk_put_global_string(ctx, "eat_reset_module");

	duk_push_c_function(ctx, _eat_get_version, 0);
	duk_put_global_string(ctx, "eat_get_version");

	duk_push_c_function(ctx, _eat_get_buildtime, 0);
	duk_put_global_string(ctx, "eat_get_buildtime");

	duk_push_c_function(ctx, _eat_get_buildno, 0);
	duk_put_global_string(ctx, "eat_get_buildno");

	//duk_push_c_function(ctx, _eat_get_event_for_user, 2);
	//duk_put_global_string(ctx, "eat_get_event_for_user");

	duk_push_c_function(ctx, _eat_get_event_num_for_user, 1);
	duk_put_global_string(ctx, "eat_get_event_num_for_user");

	//duk_push_c_function(ctx, _eat_send_msg_to_user, 6);
	//duk_put_global_string(ctx, "eat_send_msg_to_user");

	duk_push_c_function(ctx, _eat_sleep_enable, 1);
	duk_put_global_string(ctx, "eat_sleep_enable");

	//duk_push_c_function(ctx, _eat_create_sem, 2);
	//duk_put_global_string(ctx, "eat_create_sem");

	//duk_push_c_function(ctx, _eat_sem_get, 2);
	//duk_put_global_string(ctx, "eat_sem_get");

	//duk_push_c_function(ctx, _eat_sem_put, 1);
	//duk_put_global_string(ctx, "eat_sem_put");

	//duk_push_c_function(ctx, _eat_sem_query, 1);
	//duk_put_global_string(ctx, "eat_sem_query");

	duk_push_c_function(ctx, _eat_power_down, 0);
	duk_put_global_string(ctx, "eat_power_down");

	duk_push_c_function(ctx, _eat_timer_start, 2);
	duk_put_global_string(ctx, "eat_timer_start");

	duk_push_c_function(ctx, _eat_timer_stop, 1);
	duk_put_global_string(ctx, "eat_timer_stop");

	duk_push_c_function(ctx, _eat_get_current_time, 0);
	duk_put_global_string(ctx, "eat_get_current_time");

	duk_push_c_function(ctx, _eat_get_duration_ms, 1);
	duk_put_global_string(ctx, "eat_get_duration_ms");

	duk_push_c_function(ctx, _eat_get_duration_us, 1);
	duk_put_global_string(ctx, "eat_get_duration_us");

	duk_push_c_function(ctx, _eat_sleep, 1);
	duk_put_global_string(ctx, "eat_sleep");

	//duk_push_c_function(ctx, _eat_get_rtc, 1);
	//duk_put_global_string(ctx, "eat_get_rtc");

	//duk_push_c_function(ctx, _eat_set_rtc, 1);
	//duk_put_global_string(ctx, "eat_set_rtc");

	duk_push_c_function(ctx, _eat_gpt_start, 3);
	duk_put_global_string(ctx, "eat_gpt_start");

	duk_push_c_function(ctx, _eat_gpt_stop, 0);
	duk_put_global_string(ctx, "eat_gpt_stop");

	duk_push_c_function(ctx, _eat_watchdog_start, 2);
	duk_put_global_string(ctx, "eat_watchdog_start");

	duk_push_c_function(ctx, _eat_watchdog_feed, 0);
	duk_put_global_string(ctx, "eat_watchdog_feed");

	duk_push_c_function(ctx, _eat_watchdog_stop, 0);
	duk_put_global_string(ctx, "eat_watchdog_stop");

	duk_push_c_function(ctx, _eat_uart_open, 1);
	duk_put_global_string(ctx, "eat_uart_open");

	duk_push_c_function(ctx, _eat_uart_close, 1);
	duk_put_global_string(ctx, "eat_uart_close");

	//duk_push_c_function(ctx, _eat_uart_set_config, 2);
	//duk_put_global_string(ctx, "eat_uart_set_config");

	//duk_push_c_function(ctx, _eat_uart_get_config, 2);
	//duk_put_global_string(ctx, "eat_uart_get_config");

	duk_push_c_function(ctx, _eat_uart_set_baudrate, 2);
	duk_put_global_string(ctx, "eat_uart_set_baudrate");

	duk_push_c_function(ctx, _eat_uart_get_baudrate, 1);
	duk_put_global_string(ctx, "eat_uart_get_baudrate");

	duk_push_c_function(ctx, _eat_uart_write, 3);
	duk_put_global_string(ctx, "eat_uart_write");

	duk_push_c_function(ctx, _eat_uart_read, 3);
	duk_put_global_string(ctx, "eat_uart_read");

	duk_push_c_function(ctx, _eat_uart_set_debug, 1);
	duk_put_global_string(ctx, "eat_uart_set_debug");

	//duk_push_c_function(ctx, _eat_uart_set_debug_config, 2);
	//duk_put_global_string(ctx, "eat_uart_set_debug_config");

	duk_push_c_function(ctx, _eat_uart_set_at_port, 1);
	duk_put_global_string(ctx, "eat_uart_set_at_port");

	duk_push_c_function(ctx, _eat_uart_set_send_complete_event, 2);
	duk_put_global_string(ctx, "eat_uart_set_send_complete_event");

	duk_push_c_function(ctx, _eat_uart_get_send_complete_status, 1);
	duk_put_global_string(ctx, "eat_uart_get_send_complete_status");

	duk_push_c_function(ctx, _eat_uart_get_free_space, 1);
	duk_put_global_string(ctx, "eat_uart_get_free_space");

	duk_push_c_function(ctx, _eat_uart_control_by_app, 1);
	duk_put_global_string(ctx, "eat_uart_control_by_app");

	duk_push_c_function(ctx, _eat_uart_power_off, 1);
	duk_put_global_string(ctx, "eat_uart_power_off");
}