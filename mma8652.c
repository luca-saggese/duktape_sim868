/* MMA8652 C library - small 3-axis digital accelerometer from Freescale
 * 
 * This file contains the hardware-independent functions
 *
 * 2014 - Tom Magnier : tom@tmagnier.fr
 */
 
#include "mma8652.h"

bool mma8652_init(void)
{
	mma8652_i2c_init();
	
	return mma8652_verify_device_id();
}

bool mma8652_verify_device_id(void)
{
	uint8_t device_id = 0;
	
	//Read device ID from WHO_AM_I register
	mma8652_i2c_register_read(MMA8652_REG_WHO_AM_I, &device_id, 1);
	
	return (device_id == MMA8452_DEVICE_ID);
}

bool mma8652_standby(void)
{
	//Read current value of CTRL_REG1 register
	uint8_t value = 0;
	
	if(!mma8652_i2c_register_read(MMA8652_REG_CTRL_REG1, &value, sizeof(value)))
		return false;
	
	return mma8652_i2c_register_write(MMA8652_REG_CTRL_REG1, 
		value & ~(MMA8652_CTRL_REG1_ACTIVE));
}

bool mma8652_active(void)
{
	//Read current value of CTRL_REG1 register
	uint8_t value = 0;
	
	if(!mma8652_i2c_register_read(MMA8652_REG_CTRL_REG1, &value, sizeof(value)))
		return false;
	
	return mma8652_i2c_register_write(MMA8652_REG_CTRL_REG1, 
		value | MMA8652_CTRL_REG1_ACTIVE);
}

//TODO: return bool instead of void
void mma8652_config(u8 scale, u8 dataRate)
{
    uint8_t value = 0;
    

    int sensibility = MMA8652_TRANSIENT_THS_MG_LSB(1000); //1g era 63mg
    u8 debounche = 1; //era 0x40

    mma8652_standby();
    
    // Set up the full scale range to 2, 4, or 8g.
    if ((scale==2)||(scale==4)||(scale==8))
        mma8652_i2c_register_write(MMA8652_REG_XYZ_DATA_CFG, scale >> 2);  
    else
        mma8652_i2c_register_write(MMA8652_REG_XYZ_DATA_CFG, 0);

    // Setup the 3 data rate bits, from 0 to 7
    mma8652_i2c_register_read(MMA8652_REG_CTRL_REG1, &value, sizeof(value));
    mma8652_i2c_register_write(MMA8652_REG_CTRL_REG1, value & ~(0x38));
    if (dataRate <= 7)
        mma8652_i2c_register_write(MMA8652_REG_CTRL_REG1, value | (dataRate << 3));  
        

    mma8652_i2c_register_write(MMA8652_REG_CTRL_REG4, MMA8652_CTRL_REG4_INT_EN_TRANS);
    mma8652_i2c_register_write(MMA8652_REG_CTRL_REG5, MMA8652_CTRL_REG5_INT_CFG_TRANS);
    mma8652_i2c_register_write(MMA8652_REG_TRANSIENT_CFG, MMA8652_TRANSIENT_CFG_XTEFE | MMA8652_TRANSIENT_CFG_YTEFE | MMA8652_TRANSIENT_CFG_ZTEFE | MMA8652_TRANSIENT_CFG_ELE);
    
    //set sensibility
    mma8652_i2c_register_write(MMA8652_REG_TRANSIENT_THS, sensibility & MMA8652_TRANSIENT_THS_MSK);
    
    mma8652_i2c_register_write(MMA8652_REG_HP_FILTER_CUTOFF, MMA8652_HP_FILTER_SEL_MSK);
    //debounce value
    mma8652_i2c_register_write(MMA8652_REG_TRANSIENT_COUNT, debounche);

    mma8652_i2c_register_read(MMA8652_REG_CTRL_REG1, &value, sizeof(value));

    mma8652_i2c_register_write(MMA8652_REG_CTRL_REG1, value | MMA8652_CTRL_REG1_F_READ);

    mma8652_active();
}

void mma8652_read_accel(int * destination)
{
  uint8_t rawData[6];  // x/y/z accel register data stored here
  u8 i=0;

  mma8652_i2c_register_read(MMA8652_REG_OUT_X_MSB, &rawData[0], sizeof(rawData));  // Read the six raw data registers into data array

  // Loop to calculate 12-bit ADC and g value for each axis
  for (i=0; i<6; i+=2)
  {
    destination[i/2] = ((rawData[i] << 8) | rawData[i+1]) >> 4;  // Turn the MSB and LSB into a 12-bit value
    if (rawData[i] > 0x7F)
    {  
      // If the number is negative, we have to make it so manually (no 12-bit data type)
      destination[i/2] -= 0x1000;
    }
  }
}
