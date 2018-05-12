#ifndef _SPI_H_
#define _SPI_H_


#include <eat_type.h>
#include <eat_periphery.h>





u8 eat_spi_config(EatPinName_enum miso, EatPinName_enum mosi, EatPinName_enum cs, EatPinName_enum clk);
eat_bool eat_spi_init_int(EatSpiClk_enum clk, EatSpiWire_enum wire, EatSpiBit_enum bit, eat_bool enable_SDI, eat_bool enable_cs);
unsigned char eat_spi_read_int(unsigned char *data, unsigned char len);
eat_bool eat_spi_write_int(const unsigned char *data, unsigned char len, eat_bool is_command);
unsigned char eat_spi_write_read_int(const unsigned char *wdata, unsigned char wlen, unsigned char* rdata, unsigned char rlen);

#endif