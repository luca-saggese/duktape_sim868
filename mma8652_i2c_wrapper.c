/* MMA8652 C library - small 3-axis digital accelerometer from Freescale
 * 
 * This file contains the hardware dependent functions for the I2C module
 * Hardware functions for the nRF51822 MCU from Nordic, for a specific application
 * Provided as reference only
 * 
 * 2014 - Tom Magnier : tom@tmagnier.fr
 */

#include <eat_interface.h>
#include <eat_periphery.h>


#include "mma8652.h"

bool mma8652_i2c_init(void)
{
    s32 ret = eat_i2c_open(EAT_I2C_OWNER_0, MMA8652_I2C_ADDR, MMA8652_SPEED_FAST_MODE);
    if (EAT_DEV_STATUS_OK != ret)
    {
        return false;
    }

    return true;
}

bool mma8652_i2c_register_write(uint8_t register_address, uint8_t value)
{
    unsigned char write_buffer[2] = {0};

    write_buffer[0] = register_address;
    write_buffer[1] = value;

    return eat_i2c_write(EAT_I2C_OWNER_0, write_buffer, sizeof(write_buffer)) == EAT_DEV_STATUS_OK;
	
}

bool mma8652_i2c_register_read(uint8_t first_reg_address, uint8_t * p_destination, uint8_t num_bytes)
{
    return eat_i2c_read(EAT_I2C_OWNER_0, &first_reg_address, 1, p_destination, num_bytes) == EAT_DEV_STATUS_OK;
}
