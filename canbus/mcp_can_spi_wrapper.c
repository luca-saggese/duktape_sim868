#include "mcp_can.h"
#include "eat_periphery.h"
#include "log.h"
#include "eat_interface.h"


/*********************************************************************************************************
** Function name:           mcp2515_spiInit
** Descriptions:            init sim868 spi
*********************************************************************************************************/
u8 mcp2515_spiInit(void){
  eat_bool ret;
  u32 rxId;

  //eat_gpio_setup(SPI_CS_PIN,EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
  //MCP2515_UNSELECT();
  //eat_spi_init_int(EAT_SPI_CLK_13M, EAT_SPI_3WIRE,EAT_SPI_BIT8, EAT_TRUE, EAT_TRUE);  
  ret = eat_spi_init_int(EAT_SPI_CLK_13M, EAT_SPI_3WIRE, EAT_SPI_BIT8, EAT_TRUE, EAT_TRUE);
  if (ret==EAT_FALSE)
      eat_trace("Error spi init.");
  return ret;
}

/*********************************************************************************************************
** Function name:           mcp2515_readRegister
** Descriptions:            read register
*********************************************************************************************************/
u8 mcp2515_readRegister(const u8 address)                                                                     
{
  u8 ret;
  u8 cmd[2];
  //unsigned char buff[3];
  cmd[0]=MCP_READ;
  cmd[1]=address;
  eat_spi_write_read_int(&cmd[0], 2, &ret, 1);
  LOG_DEBUG("eat_spi_write_read_int ret:%d", ret);

/*
u8 val;
    u8 b;
	eat_gpio_write(_chipSelectPin, EAT_GPIO_LEVEL_LOW);
    b=((addr<<1)&0x7E) | 0x80;


    //eat_spi_write_int(&b, 1, EAT_TRUE);
	//eat_spi_read_int(&val, 1);
    //LOG_DEBUG("read byte %u %u",b,val);
    eat_spi_write_read_int(&b, 1, &val, 1);
	eat_gpio_write(_chipSelectPin, EAT_GPIO_LEVEL_HIGH);
*/

/*
  MCP2515_SELECT();
  cmd=MCP_READ;
  eat_spi_write_int(&cmd, 1, EAT_TRUE);
  cmd=address;
  eat_spi_write_int(&cmd, 1, EAT_FALSE);
  eat_spi_read_int(&ret, 1);
  MCP2515_UNSELECT();
  LOG_DEBUG("eat_spi_read_int ret:%d", ret);

  eat_sleep(50);

  MCP2515_SELECT();
  buff[0]=MCP_READ;
  buff[1]=address;
  eat_spi_write_read_int(&buff[0], 2, &ret, 1);
  LOG_DEBUG("eat_spi_write_read_int ret:%d", ret);
  MCP2515_UNSELECT();
  */

  return ret;
}

/*********************************************************************************************************
** Function name:           mcp2515_reset
** Descriptions:            reset the device
*********************************************************************************************************/
void mcp2515_reset(void)                                      
{
    unsigned char cmd[1];
    cmd[0]=MCP_RESET;
    eat_spi_write_int(&cmd[0], 1, EAT_TRUE);
    eat_sleep(100);
}

/*********************************************************************************************************
** Function name:           mcp2515_readRegisterS
** Descriptions:            read registerS
*********************************************************************************************************/
void mcp2515_readRegisterS(const u8 address, u8 values[], const u8 n)
{
  unsigned char cmd[2];
  //MCP2515_SELECT();
  cmd[0]=MCP_READ;
  cmd[1]=address;
  eat_spi_write_read_int(&cmd[0], 2, &values[0], 1);
  //MCP2515_UNSELECT();
}

/*********************************************************************************************************
** Function name:           mcp2515_setRegister
** Descriptions:            set register
*********************************************************************************************************/
void mcp2515_setRegister(const u8 address, const u8 value)
{
  unsigned char cmd[3];
  u8 b;
  //MCP2515_SELECT();
  b=(address<<1)& MCP_WRITE;
  cmd[0]=MCP_WRITE;
  cmd[1]=address;
  cmd[2]=value;
  eat_spi_write_int(&cmd[0], 3, EAT_FALSE);
  //MCP2515_UNSELECT();
}

/*********************************************************************************************************
** Function name:           mcp2515_setRegisterS
** Descriptions:            set registerS
*********************************************************************************************************/
void mcp2515_setRegisterS(const u8 address, const u8 values[], const u8 n)
{
  u8 i;
  unsigned char cmd[20];
  //MCP2515_SELECT();
  cmd[0]=MCP_WRITE;
  eat_spi_write_int(&cmd[0], 1, EAT_TRUE);
  
  cmd[0]=address;
  for (i=0; i<n; i++) 
  {
      cmd[i+1]= values[i];
  }
  eat_spi_write_int(&cmd[0], 1 + n, EAT_FALSE);
  //MCP2515_UNSELECT();
}

/*********************************************************************************************************
** Function name:           mcp2515_modifyRegister
** Descriptions:            set bit of one register
*********************************************************************************************************/
void mcp2515_modifyRegister(const u8 address, const u8 mask, const u8 data)
{
  unsigned char cmd[4];
  cmd[0]=MCP_BITMOD;
  cmd[1]=address;
  cmd[2]=mask;
  cmd[3]=data;
  eat_spi_write_int(&cmd[0], 4, EAT_FALSE);
  //MCP2515_UNSELECT();
}



/*********************************************************************************************************
** Function name:           mcp2515_readStatus
** Descriptions:            read mcp2515's Status
*********************************************************************************************************/
u8 mcp2515_readStatus(void)                             
{
  u8 ret=0;
  unsigned char cmd;
  cmd=MCP_READ_STATUS;
  eat_spi_write_read_int(&cmd, 1, &ret, 1);
  return ret;
}

void mcp2515_test(u8 id){
  u8 ret[4];
  u8 cmd[4];

  if(id==0){
    MCP2515_SELECT();
  }else if(id==1){
    MCP2515_UNSELECT();
  }else if(id==2){
    mcp2515_readStatus();
  }else if(id==3){
    while(1){
      mcp2515_readStatus();
      eat_sleep(500);
    }
  }else if(id==4){
    cmd[0]=1;
    cmd[1]=2;
    cmd[2]=3;
    cmd[3]=4;

   // eat_spi_write_read_int(&cmd[0], 4, &ret[0], 4);
    eat_spi_write_int(&cmd[0], 4, 0);
    //LOG_HEX(&ret[0], 4);

  }
}