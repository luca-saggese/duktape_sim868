/* MMA8452 C library - small 3-axis digital accelerometer from Freescale
 * 
 * This file contains the hardware dependent functions for the I2C module
 * Hardware functions for the nRF51822 MCU from Nordic, for a specific application
 * Provided as reference only
 * 
 * 2014 - Tom Magnier : tom@tmagnier.fr
 */

#include <eat_interface.h>
#include <eat_periphery.h>


#include "mma8452.h"

bool mma8452_i2c_init(void)
{
  s32 ret = eat_i2c_open(EAT_I2C_OWNER_1, MMA8452_I2C_ADDR-1, MMA8452_SPEED_NORMAL_MODE);
  if (EAT_DEV_STATUS_OK != ret)
  {
    eat_trace("unable to open i2c bus");
    return false;
  }

  return true;
}

bool mma8452_i2c_register_write(u8 register_address, u8 value)
{
  unsigned char write_buffer[2] = {0};

  write_buffer[0] = register_address;
  write_buffer[1] = value;

  return eat_i2c_write(EAT_I2C_OWNER_1, write_buffer, sizeof(write_buffer)) == EAT_DEV_STATUS_OK;
}

bool mma8452_i2c_register_read(u8 first_reg_address, u8 * p_destination, u8 num_bytes)
{
  return eat_i2c_read(EAT_I2C_OWNER_1, &first_reg_address, 1, p_destination, num_bytes) == EAT_DEV_STATUS_OK;
}
