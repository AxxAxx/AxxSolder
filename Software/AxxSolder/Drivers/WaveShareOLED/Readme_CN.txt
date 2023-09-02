/*****************************************************************************
* | File      	:   Readme_CN.txt
* | Author      :   Waveshare team
* | Function    :   Help with use
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2020-08-20
* | Info        :   在这里提供一个中文版本的使用文档，以便你的快速使用
******************************************************************************/
这个文件是帮助您使用本例程。
由于我们的OLED屏越来越多，不便于我们的维护，因此把所有的OLED程序做成一个工程。
在这里简略的描述本工程的使用：

1.基本信息：
本例程是基于HAL库进行开发的，对应的HAL固件版本为：STM32Cube_FW_F1_V1.8.0;
本例程是基于STM32F103RBT6进行开发的，例程均在我们的XNUCLEO-F103RB开发板进行了验证;
本例程可以在工程的Examples\中查看对应的测试例程;

2.管脚连接：
管脚连接你可以使用STM32CubeMX打开项目文件oled_demo.ioc查看，这里也再重述一次：
SPI:
	OLED   =>    STM32f103RB
	VCC    ->    3.3
	GND    ->    GND
	DIN    ->    PA7(MOSI)
	CLK    ->    PA5(SCLK)
	CS     ->    PB6
	DC     ->    PA8
	RST    ->    PA9

IIC:
	OLED   =>    STM32f103RB
	VCC    ->    3.3
	GND    ->    GND
	DIN    ->    PB9(SDA)
	CLK    ->    PB8(SCL)
	CS     ->    PB6
	DC     ->    PA8
	RST    ->    PA9

IIC_SOFT
	OLED   =>    STM32f103RB
	VCC    ->    3.3
	GND    ->    GND
	DIN    ->    PC8 (IIC_SDA_SOFT)
	CLK    ->    PC6 (IIC_SCL_SOFT)
	CS     ->    PB6
	DC     ->    PA8
	RST    ->    PA9

3.基本使用：
由于本工程是一个综合工程，对于使用而言，你可能需要阅读以下内容：
你可以在main.c中的97行到109行看到已经进行了注释的7个函数(后续有新产品添加肯定有变动，以实际为准)
请注意你购买的是哪一款的OLED。
栗子1：
    如果你购买的1.3inch OLED Module (C)，那么你应该把对应的105行代码的注释去掉，即：
        //	OLED_1in3_c_test();
    修改成：
        OLED_1in3_c_test();
栗子2：
    如果你购买的0.91inch OLED Module，注意由于该模块只有IIC接口，而例程默认是SPI，需要去Config.h中修改，即：
		#define USE_SPI_4W 	1
		#define USE_IIC 	0
	修改成：
		#define USE_SPI_4W 	0
		#define USE_IIC 	1
    并把对应的97行代码的注释去掉，即：
        //	OLED_0in91_test();	//	Only IIC !!!
    修改成：
       OLED_0in91_test();	//	Only IIC !!!

4.目录结构（选读）：
如果你经常使用我们的产品，对我们的程序目录结构会十分熟悉，关于具体的函数的我们有一份
函数的API手册，你可以在我们的WIKI上下载或像售后客服索取，这里简单介绍一次：
Config\:此目录为硬件接口层文件，在DEV_Config.c(.h)可以看到很多定义，包括：
    数据类型：
        #define UBYTE   uint8_t
        #define UWORD   uint16_t
        #define UDOUBLE uint32_t
	SPI和IIC的选择：
		#define USE_SPI_4W 	1
		#define USE_IIC 	0
	IIC地址：
		#define IIC_CMD        0X00
		#define IIC_RAM        0X40
    GPIO读写：
		#define OLED_CS_0		HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET)
		#define OLED_CS_1		HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET)
		#define OLED_DC_0		HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET)
		#define OLED_DC_1		HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET)
		#define OLED_RST_0		HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET)
		#define OLED_RST_1		HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET)
    SPI传输数据：
        void SPI4W_Write_Byte(UBYTE value);
    IIC传输数据：
        void I2C_Write_Byte(UBYTE value, UBYTE Cmd);
    延时：
        #define DEV_Delay_ms(__xms) HAL_Delay(__xms);
        注意：此延时函数并未使用示波器测量具体数值
    模块初始化与退出的处理：
        UBYTE	System_Init(void);
        void	System_Exit(void);
        注意：1.这里是处理使用OLED前与使用完之后一些GPIO的处理。
              
GUI\:此目录为一些基本的图像处理函数，在GUI_Paint.c(.h)中：
    常用图像处理：创建图形、翻转图形、镜像图形、设置像素点、清屏等;
    常用画图处理：画点、线、框、圆、中文字符、英文字符、数字等;
    常用时间显示：提供一个常用的显示时间函数;
    常用显示图片：提供一个显示位图的函数;
    
Fonts\:为一些常用的字体：
    Ascii:
        font8: 5*8 
        font12: 7*12
        font16: 11*16 
        font20: 14*20 
        font24: 17*24
    中文：
        font12CN: 16*21 
        font24CN: 32*41
        
OLED\:此目录下为OLED驱动函数;
Examples\:此目录下为OLED的测试程序，你可在其中看到具体的使用方法;