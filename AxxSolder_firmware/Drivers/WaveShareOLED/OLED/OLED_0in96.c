/*****************************************************************************
* | File      	:   OLED_0in96.c
* | Author      :   Waveshare team
* | Function    :   OLED_0in96 OLED Module Drive function
* | Info        :
*----------------
* |	This version:   V2.0
* | Date        :   2020-08-14
* | Info        :
* -----------------------------------------------------------------------------
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "OLED_0in96.h"
#include "stdio.h"

#define vccstate SSD1306_SWITCHCAPVCC

/*******************************************************************************
function:
			Hardware reset
*******************************************************************************/
static void OLED_0in96_Reset(void)
{
	//OLED_RST_1;
    Driver_Delay_ms(100);
    //OLED_RST_0;
    Driver_Delay_ms(100);
    //OLED_RST_1;
    Driver_Delay_ms(100);
}

/*******************************************************************************
function:
			Write register address and data
*******************************************************************************/
static void OLED_0in96_WriteReg(uint8_t Reg)
{
#if USE_SPI_4W
	//OLED_DC_0;
	//OLED_CS_0;
    SPI4W_Write_Byte(Reg);
    //OLED_CS_1;
#elif USE_IIC_SOFT
	iic_start();
	iic_write_byte(0x78);
	iic_wait_for_ack();
	iic_write_byte(0x00);
	iic_wait_for_ack();
	iic_write_byte(Reg);
	iic_wait_for_ack();
	iic_stop();
#endif
}

/*******************************************************************************
function:
			Common register initialization
*******************************************************************************/
static void OLED_0in96_WriteData(uint8_t Data)
{	
#if USE_SPI_4W
    //OLED_DC_1;
    //OLED_CS_0;
    SPI4W_Write_Byte(Data);
    //OLED_CS_1;
#elif USE_IIC_SOFT	
	iic_start();
	iic_write_byte(0x78);
	iic_wait_for_ack();
	iic_write_byte(0x40);
	iic_wait_for_ack();
	iic_write_byte(Data);
	iic_wait_for_ack();
	iic_stop();
#endif
}

static void OLED_0in96_InitReg()
{
	OLED_0in96_WriteReg(SSD1306_DISPLAYOFF);
    OLED_0in96_WriteReg(SSD1306_SETDISPLAYCLOCKDIV);
    OLED_0in96_WriteReg(0x80);                              // the suggested ratio 0x80

    OLED_0in96_WriteReg(SSD1306_SETMULTIPLEX);
    OLED_0in96_WriteReg(0x3F);
    OLED_0in96_WriteReg(SSD1306_SETDISPLAYOFFSET);
    OLED_0in96_WriteReg(0x0);                               // no offset
	OLED_0in96_WriteReg(SSD1306_SETSTARTLINE | 0x0);        // line #0
	OLED_0in96_WriteReg(SSD1306_CHARGEPUMP);
    OLED_0in96_WriteReg((vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0x14);

    OLED_0in96_WriteReg(SSD1306_MEMORYMODE);
    OLED_0in96_WriteReg(0x00);                              // 0x0 act like ks0108

    OLED_0in96_WriteReg(SSD1306_SEGREMAP | 0x1);
	OLED_0in96_WriteReg(SSD1306_COMSCANDEC);
	OLED_0in96_WriteReg(SSD1306_SETCOMPINS);
    OLED_0in96_WriteReg(0x12);           // TODO - calculate based on _rawHieght ?
	OLED_0in96_WriteReg(SSD1306_SETCONTRAST);
    OLED_0in96_WriteReg((vccstate == SSD1306_EXTERNALVCC) ? 0x9F : 0xCF);
    OLED_0in96_WriteReg(SSD1306_SETPRECHARGE);
    OLED_0in96_WriteReg((vccstate == SSD1306_EXTERNALVCC) ? 0x22 : 0xF1);
    OLED_0in96_WriteReg(SSD1306_SETVCOMDETECT);
    OLED_0in96_WriteReg(0x40);
    OLED_0in96_WriteReg(SSD1306_DISPLAYALLON_RESUME);
    OLED_0in96_WriteReg(SSD1306_NORMALDISPLAY);
}

/********************************************************************************
function:
			initialization
********************************************************************************/
void OLED_0in96_Init()
{
    //Hardware reset
    OLED_0in96_Reset();

    //Set the initialization register
    OLED_0in96_InitReg();
    Driver_Delay_ms(200);

    //Turn on the OLED display
    OLED_0in96_WriteReg(0xaf);
}

/********************************************************************************
function:
			Clear screen
********************************************************************************/
void OLED_0in96_clear()
{
    UWORD j;
	OLED_0in96_WriteReg(SSD1306_COLUMNADDR);
	OLED_0in96_WriteReg(0);         //cloumn start address
	OLED_0in96_WriteReg(OLED_0in96_HEIGHT -1); //cloumn end address
	OLED_0in96_WriteReg(SSD1306_PAGEADDR);
	OLED_0in96_WriteReg(0);         //page atart address
	OLED_0in96_WriteReg(OLED_0in96_WIDTH/8 -1); //page end address
    
    for (j = 0; j < 1024; j++) {
	        OLED_0in96_WriteData(0x00);
    }
}

/********************************************************************************
function:
			Update all memory to OLED
********************************************************************************/
void OLED_0in96_display(const UBYTE *Image)
{
    UWORD j, i, temp;
	OLED_0in96_WriteReg(SSD1306_COLUMNADDR);
	OLED_0in96_WriteReg(0);         //cloumn start address
	OLED_0in96_WriteReg(OLED_0in96_HEIGHT -1); //cloumn end address
	OLED_0in96_WriteReg(SSD1306_PAGEADDR);
	OLED_0in96_WriteReg(0);         //page atart address
	OLED_0in96_WriteReg(OLED_0in96_WIDTH/8 -1); //page end address
    
    for (j = 0; j < 8; j++) {
        for(i = 0; i < 128; i++) {
            temp = Image[7-j + i*8];
            OLED_0in96_WriteData(temp);
        }
    }
}
