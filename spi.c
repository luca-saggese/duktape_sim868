



#include "spi.h"
#include "eat_periphery.h"
#include "eat_interface.h"

static u8 SPI_PIN_CS = EAT_PIN5_UART1_DCD;
static u8 SPI_PIN_CLK = EAT_PIN7_UART1_RI;
static u8 SPI_PIN_MOSI = EAT_PIN42_STATUS;
static u8 SPI_PIN_MISO = EAT_PIN41_NETLIGHT;

static u8  SPI_PIN_CD = EAT_PIN58_GPIO2;
/*
  static u8  SPI_PIN_CS = EAT_PIN42_STATUS;
  static u8  SPI_PIN_CLK = EAT_PIN5_UART1_DCD;
  static u8  SPI_PIN_MOSI = EAT_PIN7_UART1_RI;
  static u8  SPI_PIN_MISO = EAT_PIN41_NETLIGHT;
*/

static int SPI_CLK=EAT_SPI_CLK_13M;
static u8  SPI_WIRE = EAT_SPI_3WIRE;
static u8  SPI_BIT =EAT_SPI_BIT8;
static u8  SPI_USE_MISO = EAT_TRUE;
static u8  SPI_USE_CS = EAT_TRUE;
static u8  SPI_MSBF = EAT_TRUE;

int delay_for_spi() {
  //int r1=SPI_CLK;
  int r1=1024;
  while (r1 > 0) {
    r1--;
  }
  return 0;
}

u8 eat_spi_config(EatPinName_enum miso, EatPinName_enum mosi, EatPinName_enum cs, EatPinName_enum clk){
  SPI_PIN_CS = cs;
  SPI_PIN_CLK = clk;
  SPI_PIN_MOSI = mosi;
  SPI_PIN_MISO = miso;
  return 1;
}

eat_bool eat_spi_init_int(EatSpiClk_enum clk, EatSpiWire_enum wire, EatSpiBit_enum bit, eat_bool enable_SDI, eat_bool enable_cs) {
  SPI_CLK= clk;
  SPI_WIRE = wire;
  SPI_BIT= bit;
  SPI_USE_MISO = enable_SDI;
  SPI_USE_CS= enable_cs;

  if (SPI_USE_CS) { //CS
    eat_pin_set_mode(SPI_PIN_CS, 0x0);
    eat_gpio_setup(SPI_PIN_CS, 0x1, 0x1);
  }
  eat_pin_set_mode(SPI_PIN_CLK, 0x0);
  eat_gpio_setup(SPI_PIN_CLK, 0x1, 0x1);
  eat_pin_set_mode(SPI_PIN_MOSI, 0x0);
  eat_gpio_setup(SPI_PIN_MOSI, 0x1, 0x1);
  if (SPI_WIRE == EAT_SPI_4WIRE) {
    eat_pin_set_mode(SPI_PIN_CD, 0x0);
    eat_gpio_setup(SPI_PIN_CD, 0x1, 0x1);
  }
  if (SPI_USE_MISO) { //MISO
    eat_pin_set_mode(SPI_PIN_MISO, 0x0);
    eat_gpio_setup(SPI_PIN_MISO, 0x0, 0x1); //INPUT, HIGH
  }
  return 0x1;
}

unsigned char eat_spi_read_int(unsigned char *data, unsigned char len) {
  unsigned char  r5, r0, r4;
  //eat_trace("spi read len=%d", len);
  eat_gpio_write(SPI_PIN_CLK, 0x1);
  delay_for_spi();
  
  if (SPI_USE_CS) {//CS
    eat_gpio_write(SPI_PIN_CS, 0x0);
    delay_for_spi();
  }
  for (r5 = 0x0; r5 < len; r5++) {
    r0 = 0x0;
    r4 = 0;
    do {
      if(eat_gpio_read(SPI_PIN_MISO)) {//MISO
        if(SPI_MSBF)
          r0|=(1<<(7-r4));
        else
          r0|=(1<<r4);
      }
      delay_for_spi();
      eat_gpio_write(SPI_PIN_CLK, 0x0);//CLK
      delay_for_spi();
      eat_gpio_write(SPI_PIN_CLK, 0x1);
      delay_for_spi();
      r4++;
    } while (r4 < 0x8);// bits
    *(data + r5) = r0;
  }
  if (SPI_USE_CS) {
    eat_gpio_write(SPI_PIN_CS, 0x1);
    delay_for_spi();
  }
  r0 = len;
  return r0;
}

eat_bool eat_spi_write_int(const unsigned char *data, unsigned char len, eat_bool is_command) {
  u8 r5=0,r4=0;
  
  eat_gpio_write(SPI_PIN_CLK, 0x1);
  eat_gpio_write(SPI_PIN_MOSI, 0x0);
  delay_for_spi();

  if (SPI_USE_CS) {
    eat_gpio_write(SPI_PIN_CS, 0x0);
    delay_for_spi();
  }
  
  if (SPI_WIRE == EAT_SPI_4WIRE) {
    eat_gpio_write(SPI_PIN_CD, (is_command?0:1));
    delay_for_spi();
  }
  eat_trace("spi write len=%d", len);
  for (r5 = 0; r5 < len; r5++) {
    r4 = 0;
    do {
      if(SPI_MSBF){
        if((*(data + r5)&(1<<(7-r4)))>0){
          eat_gpio_write(SPI_PIN_MOSI, 0x1);
        }else{
          eat_gpio_write(SPI_PIN_MOSI, 0x0);
        }
      }else{
        if((*(data + r5)&(1<<r4))>0){
          eat_gpio_write(SPI_PIN_MOSI, 0x1);
        }else{
          eat_gpio_write(SPI_PIN_MOSI, 0x0);
        }
      }
      delay_for_spi();
      eat_gpio_write(SPI_PIN_CLK, 0x0);
      delay_for_spi();
      eat_gpio_write(SPI_PIN_CLK, 0x1);
      delay_for_spi();
      r4++;
    } while (r4 < 8);//num bits
  }
  if (SPI_WIRE == EAT_SPI_4WIRE) {
    eat_gpio_write(SPI_PIN_CD, 0x1);
    delay_for_spi();
  }
  if (SPI_USE_CS) {
    eat_gpio_write(SPI_PIN_CS, 0x1);
    delay_for_spi();
  }
  return 0x1;
}



unsigned char eat_spi_write_read_int(const unsigned char *wdata, unsigned char wlen, unsigned char* rdata, unsigned char rlen){
  u8 r5,r4,r0;
  eat_gpio_write(SPI_PIN_CLK, 0x1);
  eat_gpio_write(SPI_PIN_MOSI, 0x0);
  delay_for_spi();

  if (SPI_USE_CS) {
    eat_gpio_write(SPI_PIN_CS, 0x0);
    delay_for_spi();
  }
  //eat_trace("spi write len=%d", wlen);
  for (r5 = 0; r5 < wlen; r5++) {
    r4 = 0;
    do {
      if(SPI_MSBF){
        if((*(wdata + r5)&(1<<(7-r4)))>0){
          eat_gpio_write(SPI_PIN_MOSI, 0x1);
        }else{
          eat_gpio_write(SPI_PIN_MOSI, 0x0);
        }
      }else{
        if((*(wdata + r5)&(1<<r4))>0){
          eat_gpio_write(SPI_PIN_MOSI, 0x1);
        }else{
          eat_gpio_write(SPI_PIN_MOSI, 0x0);
        }
      }
      delay_for_spi();
      eat_gpio_write(SPI_PIN_CLK, 0x0);
      delay_for_spi();
      eat_gpio_write(SPI_PIN_CLK, 0x1);
      delay_for_spi();
      r4++;
    } while (r4 < 8);//num bits
  }
  for (r5 = 0x0; r5 < rlen; r5++) {
    r0 = 0x0;
    r4 = 0;
    do {
      if(eat_gpio_read(SPI_PIN_MISO)) {//MISO
        if(SPI_MSBF)
          r0|=(1<<(7-r4));
        else
          r0|=(1<<r4);
      }
      delay_for_spi();
      eat_gpio_write(SPI_PIN_CLK, 0x0);//CLK
      delay_for_spi();
      eat_gpio_write(SPI_PIN_CLK, 0x1);
      delay_for_spi();
      r4++;
    } while (r4 < 0x8);// bits
    *(rdata + r5) = r0;
  }

  if (SPI_USE_CS) {
    eat_gpio_write(SPI_PIN_CS, 0x1);
    delay_for_spi();
  }
  return rlen;
}

