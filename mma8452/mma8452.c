/* MMA8452 C library - small 3-axis digital accelerometer from Freescale
 * 
 * This file contains the hardware-independent functions
 *
 * 2014 - Tom Magnier : tom@tmagnier.fr
 */

#include "eat_interface.h"
#include "mma8452.h"
#include "log.h"

/*

void MMA8452Reset() 
{
writeRegister(CTRL_REG2, 0x40); // set reset bit to 1 to assert software reset to zero at end of boot process
}

// Allow user compensation of acceleration errors
void MMA8452Offsets()
{
   MMA8452Standby();  // Must be in standby to change registers
   
   // Factory settings are pretty good; the settings below produce 1 mg error or less at 2 g full scale! For the device at rest on my table 
   // these values partially compensate for the slope of the table and the slope of the sensor in my breadboard. It is a pretty stable setup!
   // For negative values use 2's complement, i.e., -2 mg = 0xFF, etc.
   writeRegister(OFF_X, 0xF9); // X-axis compensation; this is -14 mg
   writeRegister(OFF_Y, 0x01); // Y-axis compensation; this is +2 mg
   writeRegister(OFF_Z, 0x00); // z-axis compensation; this is  0 mg adjustment
   
   MMA8452Active();  // Set to active to start reading
}

*/

bool mma8452_init(void)
{
	mma8452_i2c_init();
	
	return mma8452_verify_device_id();
}

bool mma8452_verify_device_id(void)
{
	u8 device_id = 0;
	
	//Read device ID from WHO_AM_I register
	mma8452_i2c_register_read(MMA8452_REG_WHO_AM_I, &device_id, 1);
	
	return (device_id == MMA8452_DEVICE_ID);
}

bool mma8452_reset(void)
{
	return mma8452_i2c_register_write(MMA8452_REG_CTRL_REG2, 0x40);
}

bool mma8452_standby(void)
{
	//Read current value of CTRL_REG1 register
	u8 value = 0;
	
	if(!mma8452_i2c_register_read(MMA8452_REG_CTRL_REG1, &value, sizeof(value)))
		return false;
	
	return mma8452_i2c_register_write(MMA8452_REG_CTRL_REG1, 
		value & ~(MMA8452_CTRL_REG1_ACTIVE));
}

bool mma8452_active(void)
{
	//Read current value of CTRL_REG1 register
	u8 value = 0;
	
	if(!mma8452_i2c_register_read(MMA8452_REG_CTRL_REG1, &value, sizeof(value)))
		return false;
	
	return mma8452_i2c_register_write(MMA8452_REG_CTRL_REG1, 
		value | MMA8452_CTRL_REG1_ACTIVE);
}

//TODO: return bool instead of void
u8 mma8452_config(u8 scale, u8 dataRate)
{
  u8 value = 0;
  u8 cfg=0;
  u8 odr=0;//800

  int sensibility = MMA8452_TRANSIENT_THS_MG_LSB(1000); //1g era 63mg
  u8 debounche = 1; //era 0x40

  mma8452_standby();
  
  /*
  byte cfg = readRegister(XYZ_DATA_CFG);
  cfg &= 0xFC; // Mask out scale bits
  cfg |= (fsr >> 2);  // Neat trick, see page 22. 00 = 2G, 01 = 4A, 10 = 8G
  writeRegister(XYZ_DATA_CFG, cfg);
    */
  mma8452_i2c_register_read(MMA8452_REG_XYZ_DATA_CFG, &cfg, 1);
  cfg &= 0xFC; // Mask out scale bits
  cfg |= (scale >> 2);  // Neat trick, see page 22. 00 = 2G, 01 = 4A, 10 = 8G
  mma8452_i2c_register_write(MMA8452_REG_XYZ_DATA_CFG, cfg);  

  /*
  // Set up the full scale range to 2, 4, or 8g.
  if ((scale==2)||(scale==4)||(scale==8))
      mma8452_i2c_register_write(MMA8452_REG_XYZ_DATA_CFG, scale >> 2);  
  else
      mma8452_i2c_register_write(MMA8452_REG_XYZ_DATA_CFG, 0);
  */

  // SET THE OUTPUT DATA RATE

	mma8452_i2c_register_read(MMA8452_REG_CTRL_REG1, &cfg, 1);
	cfg &= 0xC7; // Mask out data rate bits
	cfg |= (odr << 3);
	mma8452_i2c_register_write(MMA8452_REG_CTRL_REG1, cfg);

    /*

    // Setup the 3 data rate bits, from 0 to 7
    mma8452_i2c_register_read(MMA8452_REG_CTRL_REG1, &value, sizeof(value));
    mma8452_i2c_register_write(MMA8452_REG_CTRL_REG1, value & ~(0x38));
    if (dataRate <= 7)
        mma8452_i2c_register_write(MMA8452_REG_CTRL_REG1, value | (dataRate << 3));  
        

    mma8452_i2c_register_write(MMA8452_REG_CTRL_REG4, MMA8452_CTRL_REG4_INT_EN_TRANS);
    mma8452_i2c_register_write(MMA8452_REG_CTRL_REG5, MMA8452_CTRL_REG5_INT_CFG_TRANS);
    mma8452_i2c_register_write(MMA8452_REG_TRANSIENT_CFG, MMA8452_TRANSIENT_CFG_XTEFE | MMA8452_TRANSIENT_CFG_YTEFE | MMA8452_TRANSIENT_CFG_ZTEFE | MMA8452_TRANSIENT_CFG_ELE);
    
    //set sensibility
    mma8452_i2c_register_write(MMA8452_REG_TRANSIENT_THS, sensibility & MMA8452_TRANSIENT_THS_MSK);
    
    mma8452_i2c_register_write(MMA8452_REG_HP_FILTER_CUTOFF, MMA8452_HP_FILTER_SEL_MSK);
    //debounce value
    mma8452_i2c_register_write(MMA8452_REG_TRANSIENT_COUNT, debounche);

    mma8452_i2c_register_read(MMA8452_REG_CTRL_REG1, &value, sizeof(value));

    mma8452_i2c_register_write(MMA8452_REG_CTRL_REG1, value | MMA8452_CTRL_REG1_F_READ);
    */

  mma8452_active();
  return 1;
}

void mma8452_read_accel(int *destination)
{
  u8 rawData[6]={0};  // x/y/z accel register data stored here
  u8 i=0;

  mma8452_i2c_register_read(MMA8452_REG_OUT_X_MSB, &rawData[0], 6);  // Read the six raw data registers into data array

  //log_hex((const char *)&rawData[0],6);

  destination[0] = ((short)(rawData[0]<<8 | rawData[1])) >> 4;
	destination[1] = ((short)(rawData[2]<<8 | rawData[3])) >> 4;
	destination[2] = ((short)(rawData[4]<<8 | rawData[5])) >> 4;
  //eat_trace("%d %d %d", destination[0], destination[1], destination[2]);
}



