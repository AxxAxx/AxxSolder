/*****************************************************************************
* | File      	:   OLED_0in95_rgb.c
* | Author      :   Waveshare team
* | Function    :   0.95inch RGB OLED Module Drive function
* | Info        :
*----------------
* |	This version:   V2.0
* | Date        :   2020-08-17
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
#include "OLED_0in95_rgb.h"
#include "stdio.h"

/*******************************************************************************
function:
            Hardware reset
*******************************************************************************/
static void OLED_Reset(void)
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
static void OLED_WriteReg(uint8_t Reg)
{
#if USE_SPI_4W
    //OLED_DC_0;
    //OLED_CS_0;
    SPI4W_Write_Byte(Reg);
    //OLED_CS_1;
#endif
}

static void OLED_WriteData(uint8_t Data)
{   
#if USE_SPI_4W
    //OLED_DC_1;
    //OLED_CS_0;
    SPI4W_Write_Byte(Data);
    //OLED_CS_1;
#endif
}

/*******************************************************************************
function:
        Common register initialization
*******************************************************************************/
static void OLED_InitReg(void)
{
    OLED_WriteReg(DISPLAY_OFF);          //Display Off
    OLED_WriteReg(SET_CONTRAST_A);       //Set contrast for color A
    OLED_WriteReg(0xFF);                     //145 0x91
    OLED_WriteReg(SET_CONTRAST_B);       //Set contrast for color B
    OLED_WriteReg(0xFF);                     //80 0x50
    OLED_WriteReg(SET_CONTRAST_C);       //Set contrast for color C
    OLED_WriteReg(0xFF);                     //125 0x7D
    OLED_WriteReg(MASTER_CURRENT_CONTROL);//master current control
    OLED_WriteReg(0x06);                     //6
    OLED_WriteReg(SET_PRECHARGE_SPEED_A);//Set Second Pre-change Speed For ColorA
    OLED_WriteReg(0x64);                     //100
    OLED_WriteReg(SET_PRECHARGE_SPEED_B);//Set Second Pre-change Speed For ColorB
    OLED_WriteReg(0x78);                     //120
    OLED_WriteReg(SET_PRECHARGE_SPEED_C);//Set Second Pre-change Speed For ColorC
    OLED_WriteReg(0x64);                     //100
    OLED_WriteReg(SET_REMAP);            //set remap & data format
    OLED_WriteReg(0x72);                      //normal 0x72   180 0x60 
    OLED_WriteReg(SET_DISPLAY_START_LINE);//Set display Start Line
    OLED_WriteReg(0x0);
    OLED_WriteReg(SET_DISPLAY_OFFSET);   //Set display offset
    OLED_WriteReg(0x0);
    OLED_WriteReg(NORMAL_DISPLAY);       //Set display mode
    OLED_WriteReg(SET_MULTIPLEX_RATIO);  //Set multiplex ratio
    OLED_WriteReg(0x3F);
    OLED_WriteReg(SET_MASTER_CONFIGURE); //Set master configuration
    OLED_WriteReg(0x8E);
    OLED_WriteReg(POWER_SAVE_MODE);      //Set Power Save Mode
    OLED_WriteReg(0x00);                     //0x00
    OLED_WriteReg(PHASE_PERIOD_ADJUSTMENT);//phase 1 and 2 period adjustment
    OLED_WriteReg(0x31);                     //0x31
    OLED_WriteReg(DISPLAY_CLOCK_DIV);    //display clock divider/oscillator frequency
    OLED_WriteReg(0xF0);
    OLED_WriteReg(SET_PRECHARGE_VOLTAGE);//Set Pre-Change Level
    OLED_WriteReg(0x3A);
    OLED_WriteReg(SET_V_VOLTAGE);        //Set vcomH
    OLED_WriteReg(0x3E);
    OLED_WriteReg(DEACTIVE_SCROLLING);   //disable scrolling
}

/********************************************************************************
function:
            initialization
********************************************************************************/
void OLED_0in95_rgb_Init(void)
{
    //Hardware reset
    OLED_Reset();

    //Set the initialization register
    OLED_InitReg();
    Driver_Delay_ms(200);

    //Turn on the OLED display
    OLED_WriteReg(0xAF);
}

/********************************************************************************
function:
            Clear screen
********************************************************************************/
void OLED_0in95_rgb_Clear(void)
{
    UWORD i;

    OLED_WriteReg(SET_COLUMN_ADDRESS);
    OLED_WriteReg(0);         //cloumn start address
    OLED_WriteReg(OLED_0in95_RGB_WIDTH - 1); //cloumn end address
    OLED_WriteReg(SET_ROW_ADDRESS);
    OLED_WriteReg(0);         //page atart address
    OLED_WriteReg(OLED_0in95_RGB_HEIGHT - 1); //page end address  

    for(i=0; i<OLED_0in95_RGB_WIDTH*OLED_0in95_RGB_HEIGHT*2; i++){
        OLED_WriteData(0x00);
    }
}

/********************************************************************************
function:   Update all memory to OLED
********************************************************************************/
void OLED_0in95_rgb_Display(const UBYTE *Image)
{
    UWORD i, j, temp;

    OLED_WriteReg(SET_COLUMN_ADDRESS);
    OLED_WriteReg(0);         //cloumn start address
    OLED_WriteReg(OLED_0in95_RGB_WIDTH - 1); //cloumn end address
    OLED_WriteReg(SET_ROW_ADDRESS);
    OLED_WriteReg(0);         //page atart address
    OLED_WriteReg(OLED_0in95_RGB_HEIGHT - 1); //page end address  
     
    for(i=0; i<OLED_0in95_RGB_HEIGHT; i++)
        for(j=0; j<OLED_0in95_RGB_WIDTH*2; j++)
        {
            temp = Image[j + i*OLED_0in95_RGB_WIDTH*2];
            OLED_WriteData(temp);
        }
}
