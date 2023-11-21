/*****************************************************************************
* | File      	:   OLED_1in5_rgb_test.c
* | Author      :   Waveshare team
* | Function    :   1.5inch OLED Module test demo
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
#include "test.h"
#include "OLED_1in5_rgb.h"

int OLED_1in5_rgb_test(void)
{
	printf("1.5inch RGB OLED test demo\n");
	if(System_Init() != 0) {
		return -1;
	}

	if(USE_IIC) {
		printf("Only USE_SPI_4W, Please revise DEV_Config.h !!!\r\n");
		return -1;
	}
	
	printf("OLED Init...\r\n");
	OLED_1in5_rgb_Init();
	Driver_Delay_ms(500);	
	OLED_1in5_rgb_Clear();
	
	// 0.Create a new image cache
	UBYTE *BlackImage;
	UWORD Imagesize = (OLED_1in5_RGB_WIDTH*2) * OLED_1in5_RGB_HEIGHT;
	if((BlackImage = (UBYTE *)malloc(Imagesize/4)) == NULL) {
			printf("Failed to apply for black memory...\r\n");
			return -1;
	}
	printf("Paint_NewImage\r\n");
	Paint_NewImage(BlackImage, OLED_1in5_RGB_WIDTH/2, OLED_1in5_RGB_HEIGHT/2, 270, BLACK);	
	Paint_SetScale(65);
	printf("Drawing\r\n");
	//1.Select Image
	Paint_SelectImage(BlackImage);
	Driver_Delay_ms(500);
	Paint_Clear(BLACK);
	while(1) {
		
		// 2.Drawing on the image		
		printf("Drawing:page 1\r\n");
		Paint_DrawPoint(5, 5, BLUE, DOT_PIXEL_1X1, DOT_STYLE_DFT);
		Paint_DrawPoint(15, 5, BRED, DOT_PIXEL_2X2, DOT_STYLE_DFT);
		Paint_DrawPoint(25, 5, GRED, DOT_PIXEL_3X3, DOT_STYLE_DFT);
		Paint_DrawLine(5, 10, 5, 20, GBLUE, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
		Paint_DrawLine(10, 20, 10, 30, RED, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
		Paint_DrawLine(15, 30, 15, 40, MAGENTA, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
		Paint_DrawLine(20, 40, 20, 50, GREEN, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
		Paint_DrawCircle(40, 20, 15, CYAN, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
		Paint_DrawCircle(50, 50, 12, YELLOW, DOT_PIXEL_1X1, DRAW_FILL_FULL);			
		Paint_DrawRectangle(35, 15, 45, 25, BROWN, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
		Paint_DrawRectangle(45, 45, 55, 55, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);		
		// 3.Show image on page1
		OLED_1in5_rgb_Display_Part(BlackImage, 0, 0, 64, 64);
		Driver_Delay_ms(2000);			
		Paint_Clear(BLACK);

		// Drawing on the image
		printf("Drawing:page 2\r\n");
		for(UBYTE i=0; i<16; i++){
			Paint_DrawRectangle(0, 4*i, 63, 4*(i+1), i*4095, DOT_PIXEL_1X1, DRAW_FILL_FULL);
		}			
		// Show image on page2
		OLED_1in5_rgb_Display_Part(BlackImage, 0, 0, 64, 64);
		Driver_Delay_ms(2000);	
		Paint_Clear(BLACK);	
		
		// Drawing on the image
		printf("Drawing:page 3\r\n");			
		Paint_DrawString_EN(0, 0, "waveshare", &Font12, BLACK, BLUE);
		Paint_DrawString_EN(0, 14, "hello world", &Font8, BLACK, MAGENTA);
		Paint_DrawNum(10, 24, 123.459, &Font8, 2, RED, BLACK);
		Paint_DrawNum(10, 34, 9854, &Font12, 1, YELLOW, BLACK);
		// Show image on page2
		OLED_1in5_rgb_Display_Part(BlackImage, 0, 0, 64, 64);
		Driver_Delay_ms(2000);	
		Paint_Clear(BLACK);		
		
		// Drawing on the image
		printf("Drawing:page 4\r\n");
		Paint_DrawString_CN(0, 0,"ÄãºÃAb", &Font12CN, BROWN, BLACK);
		// Show image on page4
		OLED_1in5_rgb_Display_Part(BlackImage, 0, 0, 64, 64);
		Driver_Delay_ms(2000);		
		Paint_Clear(BLACK);		
		
		// show the array image
		printf("Drawing:page 5\r\n");
		OLED_1in5_rgb_Display(gImage_1in5_rgb);
		Driver_Delay_ms(2000);		
		Paint_Clear(BLACK);		

		OLED_1in5_rgb_Clear();
	}
}

