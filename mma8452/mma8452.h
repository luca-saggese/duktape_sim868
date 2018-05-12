/* MMA8452 C library - small 3-axis digital accelerometer from Freescale
 * 
 * 2014 - Tom Magnier : magnier.tom@gmail.com
 */

#ifndef MMA8452_H
#define MMA8452_H

#include <stdint.h>
#include <stdbool.h>
#include "eat_type.h"


#define MMA8452_I2C_ADDR				(0x1D) //7-bit I2C address
#define MMA8452_DEVICE_ID				(0x4A) //Device ID, to be compared with WHO_AM_I register
#define MMA8452_DEVICE_ID				(0x2A) //Device ID, to be compared with WHO_AM_I register

#define MMA8452_SPEED_FAST_MODE         (400)
#define MMA8452_SPEED_NORMAL_MODE       (100)

/* MMA8452 register addresses definitions */
#define MMA8452_REG_STATUS_F_STATUS		(0x00)	//Data Status / FIFO status (depends on F_MODE)
#define MMA8452_REG_OUT_X_MSB			(0x01)	//12-bit X data
#define MMA8452_REG_OUT_X_LSB			(0x02)	//12-bit X data
#define MMA8452_REG_OUT_Y_MSB			(0x03)	//12-bit Y data
#define MMA8452_REG_OUT_Y_LSB			(0x04)	//12-bit Y data
#define MMA8452_REG_OUT_Z_MSB			(0x05)	//12-bit Z data
#define MMA8452_REG_OUT_Z_LSB			(0x06)	//12-bit Z data
#define MMA8452_REG_F_SETUP				(0x09)	//FIFO setup
#define MMA8452_REG_TRIG_CFG			(0x0A)	//FIFO triggers
#define MMA8452_REG_SYSMOD				(0x0B)	//System mode
#define MMA8452_REG_INT_SOURCE			(0x0C)	//Interrupt status
#define MMA8452_REG_WHO_AM_I			(0x0D)	//ID register
#define MMA8452_REG_XYZ_DATA_CFG		(0x0E)	//Data config
#define MMA8452_REG_HP_FILTER_CUTOFF	(0x0F)	//HP filter setting
#define MMA8452_REG_PL_STATUS			(0x10)	//PL status
#define MMA8452_REG_PL_CFG				(0x11)	//PL configuration
#define MMA8452_REG_PL_COUNT			(0x12)	//PL debounce
#define MMA8452_REG_PL_BF_ZCOMP			(0x13)	//PL Back/front Z comp
#define MMA8452_REG_PL_THS_REG			(0x14)	//PL threshold
#define MMA8452_REG_FF_MT_CFG			(0x15)	//Freefall/motion config
#define MMA8452_REG_FF_MT_SRC			(0x16)	//Freefall/motion source
#define MMA8452_REG_FF_MT_THS			(0x17)	//Freefall/motion threshold
#define MMA8452_REG_FF_MT_COUNT			(0x18)	//Freefall/motion debounce
#define MMA8452_REG_TRANSIENT_CFG		(0x1D)	//Transient config
#define MMA8452_REG_TRANSIENT_SRC		(0x1E)	//Transient source
#define MMA8452_REG_TRANSIENT_THS		(0x1F)	//Transient threshold
#define MMA8452_REG_TRANSIENT_COUNT		(0x20)	//Transient debounce
#define MMA8452_REG_PULSE_CFG			(0x21)	//Pulse config
#define MMA8452_REG_PULSE_SRC			(0x22)	//Pulse source
#define MMA8452_REG_PULSE_THSX			(0x23)	//Pulse X threshold
#define MMA8452_REG_PULSE_THSY			(0x24)	//Pulse Y threshold
#define MMA8452_REG_PULSE_THSZ			(0x25)	//Pulse Z threshold
#define MMA8452_REG_PULSE_TMLT			(0x26)	//Pulse first timer
#define MMA8452_REG_PULSE_LTCY			(0x27)	//Pulse latency
#define MMA8452_REG_PULSE_WIND			(0x28)	//Pulse second window
#define MMA8452_REG_ASLP_COUNT			(0x29)	//Auto-SLEEP counter
#define MMA8452_REG_CTRL_REG1			(0x2A)	//Control reg1
#define MMA8452_REG_CTRL_REG2			(0x2B)	//Control reg2
#define MMA8452_REG_CTRL_REG3			(0x2C)	//Control reg3 (wake int. from SLEEP)
#define MMA8452_REG_CTRL_REG4			(0x2D)	//Control reg4 (int. enable map)
#define MMA8452_REG_CTRL_REG5			(0x2E)	//Control reg5 (int. configuration)
#define MMA8452_REG_OFF_X				(0x2F)	//X-axis 0g offset
#define MMA8452_REG_OFF_Y				(0x30)	//Y-axis 0g offset
#define MMA8452_REG_OFF_Z				(0x31)	//Z-axis 0g offset


/* MMA8452 register bits definitions 
 * Use example for read-only registers : if(i2c_read(MMA8452_REG_STATUS_F_STATUS) & MMA8452_STATUS_XDR) ...
 */
//STATUS Data status register (0x00 -- with F_MODE = 00)
#define MMA8452_STATUS_XDR				(1<<0)	//X-axis new data available
#define MMA8452_STATUS_YDR				(1<<1)	//Y-axis new data available
#define MMA8452_STATUS_ZDR				(1<<2)	//Z-axis new data available
#define MMA8452_STATUS_ZYXDR			(1<<3)	//X,Y,Z-axis new data available
#define MMA8452_STATUS_XOW				(1<<4)	//X-axis data overwrite
#define MMA8452_STATUS_YOW				(1<<5)	//Y-axis data overwrite
#define MMA8452_STATUS_ZOW				(1<<6)	//Z-axis data overwrite
#define MMA8452_STATUS_ZYXOW			(1<<7)	//X,Y,Z-axis data overwrite

//F_STATUS FIFO status register (0x00 -- with F_MODE > 0)
#define	MMA8452_F_STATUS_F_CNT_MSK		(0x3F)	//FIFO sample counter
#define MMA8452_F_STATUS_F_WMRK_FLAG	(1<<6)	//FIFO watermark flag
#define MMA8452_F_STATUS_F_OVF			(1<<7)	//FIFO overflow flag

//F_SETUP FIFO Setup register (0x09)
//Bits 7-6 : F_MODE
#define MMA8452_F_MODE_MSK				(0x3<<6)	//FIFO mode bitmask
#define MMA8452_F_MODE_FIFO_DISABLED 	(0x0<<6)	//FIFO disabled (default)
#define MMA8452_F_MODE_FIFO_CIRCULAR 	(0x1<<6)	//Circular buffer mode
#define MMA8452_F_MODE_FIFO_STOP 		(0x2<<6)	//FIFO stops accepting samples on overflow
#define MMA8452_F_MODE_FIFO_TRIGGER		(0x3<<6)	//Trigger mode
//Bits 5-0 : F_WMRK
#define MMA8452_F_SETUP_F_WMRK_MSK		(0x3F)	//FIFO event sample count watermark

//TRIG_CFG Trigger configuration register (0x0A)
#define MMA8452_TRIG_CFG_Trig_FF_MT		(1<<2)	//Freefall/motion trigger
#define MMA8452_TRIG_CFG_Trig_PULSE		(1<<3)	//Pulse interrupt trigger
#define MMA8452_TRIG_CFG_Trig_LNDPRT	(1<<4)	//Landscape/portrait orientation trigger
#define MMA8452_TRIG_CFG_Trig_TRANS		(1<<5)	//Transient interrupt trigger

//SYSMOD System mode register (0x0B)
#define MMA8452_SYSMOD_MSK				(0x3)	//System mode bitmask
#define MMA8452_SYSMOD_STANDBY			(0x0)	//System mode : STANDBY (default)
#define MMA8452_SYSMOD_WAKE				(0x1)	//System mode : WAKE
#define MMA8452_SYSMOD_SLEEP			(0x2)	//System mode : SLEEP
#define MMA8452_SYSMOD_FGT_MSK			(0x1F<<2) //number of samples since gate error
#define MMA8452_SYSMOD_FGERR			(1<<7)	//FIFO gate error flag

//INT_SOURCE System interrupt status register (0x0C)
#define MMA8452_INT_SRC_DRDY			(1<<0)	//Data ready interrupt status
#define MMA8452_INT_SRC_FF_MT			(1<<2)	//Freefall/motion interrupt status
#define MMA8452_INT_SRC_PULSE			(1<<3)	//Pulse interrupt status
#define MMA8452_INT_SRC_LNDPRT			(1<<4)	//Landscape/portrait orientation interrupt status
#define MMA8452_INT_SRC_TRANS			(1<<5)	//Transient interrupt status
#define MMA8452_INT_SRC_FIFO			(1<<6)	//FIFO interrupt status
#define MMA8452_INT_SRC_ASLP			(1<<7)	//Auto-SLEEP/WAKE interrupt status

//XYZ_DATA_CFG register (0x0E)
#define MMA8452_FS_RANGE_MSK			(0x3)	//Full-scale range selection bitmask
#define MMA8452_FS_RANGE_2G				(0x0)	//Full-scale range +/- 2g (default)
#define MMA8452_FS_RANGE_4G				(0x1)	//Full-scale range +/- 4g
#define MMA8452_FS_RANGE_8G				(0x2)	//Full-scale range +/- 8g

#define MMA8452_XYZ_DATA_CFG_HPF_OUT	(1<<4)	//Enable high-pass filter on output data

//HP_FILTER_CUTOFF High-pass filter register (0x0F)
#define MMA8452_HP_FILTER_SEL_MSK		(0x03)	//HP cutoff freq. selection, see Table 33 p. 31
#define MMA8452_HP_FILTER_Pulse_LPF_EN	(1<<4)	//Enable LP filter for pulse function
#define MMA8452_HP_FILTER_Pulse_HPF_BYP	(1<<5)	//Bypass HP filter for pulse function

//PL_STATUS Portrait/Landscape status register(0x10)
#define MMA8452_PL_STATUS_BAFRO			(1<<0)	//Back or Front orientation

#define MMA8452_PL_STATUS_LAPO_MSK		(0x3<<1) //Land/portrait orientation bitmask
#define MMA8452_PL_STATUS_LAPO_PORTRAIT_UP 		(0x0<<1)
#define MMA8452_PL_STATUS_LAPO_PORTRAIT_DOWN 	(0x1<<1)
#define MMA8452_PL_STATUS_LAPO_LANDSCAPE_RIGHT 	(0x2<<1)
#define MMA8452_PL_STATUS_LAPO_LANDSCAPE_LEFT	(0x3<<1)

#define MMA8452_PL_STATUS_LO			(1<<6)	//Z-tilt angle lockout
#define MMA8452_PL_STATUS_NEWLP			(1<<7)	//L/P status change flag

//PL_CFG Portrait/Landscape config register (0x11)
#define MMA8452_PL_CFG_PL_EN			(1<<6) //P/L detection enable
#define MMA8452_PL_CFG_DBCNTM			(1<<7) //Debounce counter mode selection

//PL_BF_ZCOMP Back/front and Z compensation register (0x13)
#define MMA8452_PL_BF_ZCOMP_ZLOCK_MSK	(0x07)	//Z-lock angle threshold, see Table 43 p.34
#define MMA8452_PL_BF_ZCOMP_BKFR		(0x3<<6) //Back/front trip angle threshold, see Table 44 p.34

//PL_THS_REG P/L threshold and hysteresis register (0x14)
#define MMA8452_PL_HYS_MSK				(0x07)	//Hysteresis value, see Table 48 p.35
#define MMA8452_PL_THS_MSK				(0x1F<<3) //P/L trip threshold angle, see Table 47 p.35

//FF_MT_CFG Freefall/motion configuration register (0x15)
#define MMA8452_FF_MT_CFG_XEFE			(1<<3)	//Event flag enable on X event
#define MMA8452_FF_MT_CFG_YEFE			(1<<4)	//Event flag enable on Y event
#define MMA8452_FF_MT_CFG_ZEFE			(1<<5)	//Event flag enable on Z event
#define MMA8452_FF_MT_CFG_OAE			(1<<6)	//if 1 : motion detection, if 0 : freefall detection
#define MMA8452_FF_MT_CFG_ELE			(1<<7)	//Event Latch enable

//FF_MT_SRC FF/MT source register (0x16)
#define MMA8452_FF_MT_SRC_XHP			(1<<0)	//X-motion polarity flag (1: event was negative g)
#define MMA8452_FF_MT_SRC_XHE			(1<<1)	//X-motion event flag (1: X motion detected)
#define MMA8452_FF_MT_SRC_YHP			(1<<2)	//Y-motion polarity flag (1: event was negative g)
#define MMA8452_FF_MT_SRC_YHE			(1<<3)	//Y-motion event flag (1: Y motion detected)
#define MMA8452_FF_MT_SRC_ZHP			(1<<4)	//Z-motion polarity flag (1: event was negative g)
#define MMA8452_FF_MT_SRC_ZHE			(1<<5)	//Z-motion event flag (1: Z motion detected)
#define MMA8452_FF_MT_SRC_EA			(1<<7)	//Event active flag

//FF_MT_THS FF/MT threshold register (0x17)
#define MMA8452_FF_MT_THS_MSK			(0x7F)	//FF/MT threshold (resolution : 0.063g/LSB)
#define MMA8452_FF_MT_THS_DBCNTM		(1<<7)	//Debounce counter mode selection

//TRANSIENT_CFG Transient configuration register (0x1D)
#define MMA8452_TRANSIENT_CFG_HPF_BYP	(1<<0)	//bypass HP filter for transient detection
#define MMA8452_TRANSIENT_CFG_XTEFE		(1<<1)	//Event flag enable for X-transient detection
#define MMA8452_TRANSIENT_CFG_YTEFE		(1<<2)	//Event flag enable for Y-transient detection
#define MMA8452_TRANSIENT_CFG_ZTEFE		(1<<3)	//Event flag enable for Z-transient detection
#define MMA8452_TRANSIENT_CFG_ELE		(1<<4)	//Event Latch enable

//TRANSIENT_SRC Transient source register (0x1E)
#define MMA8452_TRANSIENT_SRC_XTP		(1<<0)	//X-transient event polarity (1: event was negative g)
#define MMA8452_TRANSIENT_SRC_XTE		(1<<1)	//X-transient event flag
#define MMA8452_TRANSIENT_SRC_YTP		(1<<2)	//Y-transient event polarity (1: event was negative g)
#define MMA8452_TRANSIENT_SRC_YTE		(1<<3)	//Y-transient event flag
#define MMA8452_TRANSIENT_SRC_ZTP		(1<<4)	//Z-transient event polarity (1: event was negative g)
#define MMA8452_TRANSIENT_SRC_ZTE		(1<<5)	//Z-transient event flag
#define MMA8452_TRANSIENT_SRC_EA		(1<<6)	//Event active flag

//TRANSIENT_THS Transient threshold register (0x1F)
#define MMA8452_TRANSIENT_THS_MG_LSB(x) ((x / 63))	//resolution : 63mg/LSB)
#define MMA8452_TRANSIENT_THS_MSK		(0x7F)	//Transient threshold (resolution : 0.063g/LSB)
#define MMA8452_TRANSIENT_THS_DBCNTM	(1<<7)	//Debounce counter mode selection

//PULSE_CFG Pulse configuration register (0x21)
#define MMA8452_PULSE_CFG_XSPEFE		(1<<0)	//Event flag enable for single pulse event on X axis
#define MMA8452_PULSE_CFG_XDPEFE		(1<<1)	//Event flag enable for double pulse event on X axis
#define MMA8452_PULSE_CFG_YSPEFE		(1<<2)	//Event flag enable for single pulse event on Y axis
#define MMA8452_PULSE_CFG_YDPEFE		(1<<3)	//Event flag enable for double pulse event on Y axis
#define MMA8452_PULSE_CFG_ZSPEFE		(1<<4)	//Event flag enable for single pulse event on Z axis
#define MMA8452_PULSE_CFG_ZDPEFE		(1<<5)	//Event flag enable for double pulse event on Z axis
#define MMA8452_PULSE_CFG_ELE			(1<<6)	//Event Latch enable
#define MMA8452_PULSE_CFG_DPA			(1<<7)	//Double pulse abort

//PULSE_SRC Pulse source register (0x22)
#define MMA8452_PULSE_SRC_XPP			(1<<0)	//X pulse event polarity (1: event was negative)
#define MMA8452_PULSE_SRC_XPE			(1<<1)	//X pulse event flag
#define MMA8452_PULSE_SRC_YPP			(1<<2)	//Y pulse event polarity (1: event was negative)
#define MMA8452_PULSE_SRC_YPE			(1<<3)	//Y pulse event flag
#define MMA8452_PULSE_SRC_ZPP			(1<<4)	//Z pulse event polarity (1: event was negative)
#define MMA8452_PULSE_SRC_ZPE			(1<<5)	//Z pulse event flag
#define MMA8452_PULSE_SRC_EA			(1<<6)	//Event Active flag

//PULSE_THSX,Y,Z Pulse threshold for X,Y and Z axis registers (0x23-0x25)
#define MMA8452_PULSE_THS_MSK			(0x7F)	//Pulse threshold (resolution : 0.063g/LSB)

//CTRL_REG1 System Control 1 register (0x2A)
//NOTE : device must be in STANDBY mode to change any of the fields in CTRL_REG1
#define MMA8452_CTRL_REG1_ACTIVE		(1<<0)	//STANDBY/ACTIVE mode selection
#define MMA8452_CTRL_REG1_F_READ		(1<<1)	//Fast read mode (single byte data format)

#define MMA8452_CTRL_REG1_DATARATE_MSK	(0x7<<3) //Data rate selection bitmask
#define MMA8452_CTRL_REG1_DATARATE_800_HZ	(0x0<<3)	//800Hz 	1.25ms (default)
#define MMA8452_CTRL_REG1_DATARATE_400_HZ	(0x1<<3)	//400Hz 	2.5ms 
#define MMA8452_CTRL_REG1_DATARATE_200_HZ	(0x2<<3)	//200Hz 	5ms
#define MMA8452_CTRL_REG1_DATARATE_100_HZ	(0x3<<3)	//100Hz 	10ms
#define MMA8452_CTRL_REG1_DATARATE_50_HZ	(0x4<<3)	//50Hz 		20ms
#define MMA8452_CTRL_REG1_DATARATE_12_5_HZ	(0x5<<3)	//12.5Hz 	80ms
#define MMA8452_CTRL_REG1_DATARATE_6_25_HZ	(0x6<<3)	//6.25Hz 	160ms
#define MMA8452_CTRL_REG1_DATARATE_1_56_HZ	(0x7<<3)	//1.56Hz 	640ms

#define MMA8452_CTRL_REG1_ASLP_RATE_MSK	(0x3<<6) //Data rate when in SLEEP mode bitmask
#define MMA8452_CTRL_REG1_ASLP_RATE_50_HZ	(0x0<<6)	//50Hz
#define MMA8452_CTRL_REG1_ASLP_RATE_12_5_HZ	(0x1<<6)	//12.5Hz
#define MMA8452_CTRL_REG1_ASLP_RATE_6_25_HZ	(0x2<<6)	//6.25Hz
#define MMA8452_CTRL_REG1_ASLP_RATE_1_56_HZ	(0x3<<6)	//1.56Hz

//CTRL_REG2 System Control 2 register (0x2B)
#define MMA8452_CTRL_REG2_MODS_MSK		(0x3<<0)	//ACTIVE mode power scheme selection bitmask (oversampling mode)
#define MMA8452_CTRL_REG2_MODS_NORMAL	(0x0<<0)	//Normal mode - see Table 101 p.54
#define MMA8452_CTRL_REG2_MODS_LNLP		(0x1<<0)	//Low noise low power mode
#define MMA8452_CTRL_REG2_MODS_HI_RES	(0x2<<0)	//High resolution mode
#define MMA8452_CTRL_REG2_MODS_LP		(0x3<<0)	//Low power mode

#define MMA8452_CTRL_REG2_SLPE			(1<<2)		//Auto-SLEEP enable

#define MMA8452_CTRL_REG2_SMODS_MSK		(0x3<<3)	//SLEEP mode power scheme selection bitmask (oversampling mode)
#define MMA8452_CTRL_REG2_SMODS_NORMAL	(0x0<<3)	//Normal mode
#define MMA8452_CTRL_REG2_SMODS_LNLP	(0x1<<3)	//Low noise low power mode
#define MMA8452_CTRL_REG2_SMODS_HI_RES	(0x2<<3)	//High resolution mode
#define MMA8452_CTRL_REG2_SMODS_LP		(0x3<<3)	//Low power mode

#define MMA8452_CTRL_REG2_RST			(1<<6)		//Software reset
#define MMA8452_CTRL_REG2_ST			(1<<7)		//Self-test enable

//CTRL_REG3 System control 3 register (0x2C)
#define MMA8452_CTRL_REG3_PP_OD			(1<<0)		//Push-pull/open-dran selection on interrupt pad (1: open-drain)
#define MMA8452_CTRL_REG3_IPOL			(1<<1)		//Interrupt polarity (1: ACTIVE high)
#define MMA8452_CTRL_REG3_WAKE_FF_MT	(1<<3)		//Wake from FF/MT interrupt
#define MMA8452_CTRL_REG3_WAKE_PULSE	(1<<4)		//Wake from Pulse interrupt
#define MMA8452_CTRL_REG3_WAKE_LNDPRT	(1<<5)		//Wake from Orientation interrupt
#define MMA8452_CTRL_REG3_WAKE_TRANS	(1<<6)		//Wake from Transient interrupt
#define MMA8452_CTRL_REG3_FIFO_GATE		(1<<7)		//FIFO Gate

//CTRL_REG4 System Control 4 register (0x2D)
#define MMA8452_CTRL_REG4_INT_EN_DRDY	(1<<0)		//Data Ready interrupt enable
#define MMA8452_CTRL_REG4_INT_EN_FF_MT	(1<<2)		//FF/MT interrupt enable
#define MMA8452_CTRL_REG4_INT_EN_PULSE	(1<<3)		//Pulse interrupt enable
#define MMA8452_CTRL_REG4_INT_EN_LNDPRT	(1<<4)		//Orientation interrupt enable
#define MMA8452_CTRL_REG4_INT_EN_TRANS	(1<<5)		//Transient interrupt enable
#define MMA8452_CTRL_REG4_INT_EN_FIFO	(1<<6)		//FIFO interrupt enable
#define MMA8452_CTRL_REG4_INT_EN_ASLP	(1<<7)		//Auto-SLEEP/WAKE interrupt enable

//CTRL_REG5 System Control 5 register (0x2E)
#define MMA8452_CTRL_REG5_INT_CFG_DRDY	(1<<0)		//Data Ready interrupt config (0: routed to INT1, 1: routed to INT2)
#define MMA8452_CTRL_REG5_INT_CFG_FF_MT	(1<<2)		//FF/MT interrupt config
#define MMA8452_CTRL_REG5_INT_CFG_PULSE	(1<<3)		//Pulse interrupt config
#define MMA8452_CTRL_REG5_INT_CFG_LNDPR	(1<<4)		//Orientation interrupt config
#define MMA8452_CTRL_REG5_INT_CFG_TRANS	(1<<5)		//Transient interrupt config
#define MMA8452_CTRL_REG5_INT_CFG_FIFO	(1<<6)		//FIFO interrupt config
#define MMA8452_CTRL_REG5_INT_CFG_ASLP	(1<<7)		//Auto-SLEEP/WAKE interrupt config


//Hardware-dependent functions that must be defined by the user
//--------------------------------------------------------------
bool mma8452_i2c_init(void);  //Init I2C hardware module - return the result of the operation (success/failure)
bool mma8452_i2c_register_write(u8 register_address, u8 value); //write a single register 

/**@brief Read multiple registers 
 * 
 * @param[in] first_reg_address address of the first register to read
 * @param[in] p_destination 	pointer to an array of bytes to store read values in
 * @param[in] num_bytes			the number of bytes to read
 *
 * @return true if the operation has succeeded
 */
bool mma8452_i2c_register_read(u8 first_reg_address, u8 * p_destination, u8 num_bytes);


//Hardware_independent functions
//-------------------------------
/**@brief Init MMA8452 operation
 *
 * @return true if the init has succeeded
 */
bool mma8452_init(void); 

/**@brief Check the device ID by comparing the content of the WHO_AM_I register and the stored ID
 *
 * @return true if the ID is correct
 */
bool mma8452_verify_device_id(void);

/**@brief Put the device in STANDBY mode
 *
 * @return the result of the operation
 */
bool mma8452_standby(void);

/**@brief Put the device in ACTIVE mode
 *
 * @return the result of the operation
 */
bool mma8452_active(void);


u8 mma8452_config(u8 scale, u8 dataRate);

void mma8452_read_accel(int * destination);

#endif //MMA8452_H
