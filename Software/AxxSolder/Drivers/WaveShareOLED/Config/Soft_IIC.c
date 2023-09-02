#include "Soft_IIC.h"
#include "stdio.h"
void iic_init(void)
{
	__IIC_SCL_SET();
	__IIC_SDA_SET();
	Driver_Delay_ms(1);
}

void iic_start(void)
{
	__IIC_SDA_OUT();
	__IIC_SDA_SET();
	__IIC_SCL_SET();//
	Driver_Delay_us(10);
	__IIC_SDA_CLR();
	Driver_Delay_us(10);
	__IIC_SCL_CLR();
	Driver_Delay_us(10);
}

void iic_stop(void)
{
	__IIC_SCL_CLR();
	__IIC_SDA_CLR();
	Driver_Delay_us(10);
	__IIC_SCL_SET();
	Driver_Delay_us(10);
	__IIC_SDA_SET();
	Driver_Delay_us(10);
}

int iic_wait_for_ack(void)
{
	uint8_t chTimeOut = 0;
	
	__IIC_SCL_CLR();
	Driver_Delay_us(10);
	__IIC_SCL_SET();
	
	while ((!(__IIC_SDA_READ())) && (chTimeOut ++)) {
		chTimeOut ++;
		if (chTimeOut > 200) {
			chTimeOut = 0;
			iic_stop();
			printf("timeout");
			return -1;
		}
	}
	__IIC_SCL_CLR();
	Driver_Delay_us(10);
	
	return 0;
}

void iic_write_byte(uint8_t chData)
{
	uint8_t i;
	
	__IIC_SDA_OUT();
	for(i = 0; i < 8; i ++) {
		__IIC_SCL_CLR();
		Driver_Delay_us(10);
		if(chData & 0x80) {
			__IIC_SDA_SET();
		} else {
			__IIC_SDA_CLR();
		}
		chData <<= 1;
		Driver_Delay_us(10);
		__IIC_SCL_SET();
		Driver_Delay_us(10);
	}
}




