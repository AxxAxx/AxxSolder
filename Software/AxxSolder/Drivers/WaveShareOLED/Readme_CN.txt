/*****************************************************************************
* | File      	:   Readme_CN.txt
* | Author      :   Waveshare team
* | Function    :   Help with use
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2020-08-20
* | Info        :   �������ṩһ�����İ汾��ʹ���ĵ����Ա���Ŀ���ʹ��
******************************************************************************/
����ļ��ǰ�����ʹ�ñ����̡�
�������ǵ�OLED��Խ��Խ�࣬���������ǵ�ά������˰����е�OLED��������һ�����̡�
��������Ե����������̵�ʹ�ã�

1.������Ϣ��
�������ǻ���HAL����п����ģ���Ӧ��HAL�̼��汾Ϊ��STM32Cube_FW_F1_V1.8.0;
�������ǻ���STM32F103RBT6���п����ģ����̾������ǵ�XNUCLEO-F103RB�������������֤;
�����̿����ڹ��̵�Examples\�в鿴��Ӧ�Ĳ�������;

2.�ܽ����ӣ�
�ܽ����������ʹ��STM32CubeMX����Ŀ�ļ�oled_demo.ioc�鿴������Ҳ������һ�Σ�
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

3.����ʹ�ã�
���ڱ�������һ���ۺϹ��̣�����ʹ�ö��ԣ��������Ҫ�Ķ��������ݣ�
�������main.c�е�97�е�109�п����Ѿ�������ע�͵�7������(�������²�Ʒ��ӿ϶��б䶯����ʵ��Ϊ׼)
��ע���㹺�������һ���OLED��
����1��
    ����㹺���1.3inch OLED Module (C)����ô��Ӧ�ðѶ�Ӧ��105�д����ע��ȥ��������
        //	OLED_1in3_c_test();
    �޸ĳɣ�
        OLED_1in3_c_test();
����2��
    ����㹺���0.91inch OLED Module��ע�����ڸ�ģ��ֻ��IIC�ӿڣ�������Ĭ����SPI����ҪȥConfig.h���޸ģ�����
		#define USE_SPI_4W 	1
		#define USE_IIC 	0
	�޸ĳɣ�
		#define USE_SPI_4W 	0
		#define USE_IIC 	1
    ���Ѷ�Ӧ��97�д����ע��ȥ��������
        //	OLED_0in91_test();	//	Only IIC !!!
    �޸ĳɣ�
       OLED_0in91_test();	//	Only IIC !!!

4.Ŀ¼�ṹ��ѡ������
����㾭��ʹ�����ǵĲ�Ʒ�������ǵĳ���Ŀ¼�ṹ��ʮ����Ϥ�����ھ���ĺ�����������һ��
������API�ֲᣬ����������ǵ�WIKI�����ػ����ۺ�ͷ���ȡ������򵥽���һ�Σ�
Config\:��Ŀ¼ΪӲ���ӿڲ��ļ�����DEV_Config.c(.h)���Կ����ܶඨ�壬������
    �������ͣ�
        #define UBYTE   uint8_t
        #define UWORD   uint16_t
        #define UDOUBLE uint32_t
	SPI��IIC��ѡ��
		#define USE_SPI_4W 	1
		#define USE_IIC 	0
	IIC��ַ��
		#define IIC_CMD        0X00
		#define IIC_RAM        0X40
    GPIO��д��
		#define OLED_CS_0		HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET)
		#define OLED_CS_1		HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET)
		#define OLED_DC_0		HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET)
		#define OLED_DC_1		HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET)
		#define OLED_RST_0		HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET)
		#define OLED_RST_1		HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET)
    SPI�������ݣ�
        void SPI4W_Write_Byte(UBYTE value);
    IIC�������ݣ�
        void I2C_Write_Byte(UBYTE value, UBYTE Cmd);
    ��ʱ��
        #define DEV_Delay_ms(__xms) HAL_Delay(__xms);
        ע�⣺����ʱ������δʹ��ʾ��������������ֵ
    ģ���ʼ�����˳��Ĵ���
        UBYTE	System_Init(void);
        void	System_Exit(void);
        ע�⣺1.�����Ǵ���ʹ��OLEDǰ��ʹ����֮��һЩGPIO�Ĵ���
              
GUI\:��Ŀ¼ΪһЩ������ͼ����������GUI_Paint.c(.h)�У�
    ����ͼ��������ͼ�Ρ���תͼ�Ρ�����ͼ�Ρ��������ص㡢������;
    ���û�ͼ�������㡢�ߡ���Բ�������ַ���Ӣ���ַ������ֵ�;
    ����ʱ����ʾ���ṩһ�����õ���ʾʱ�亯��;
    ������ʾͼƬ���ṩһ����ʾλͼ�ĺ���;
    
Fonts\:ΪһЩ���õ����壺
    Ascii:
        font8: 5*8 
        font12: 7*12
        font16: 11*16 
        font20: 14*20 
        font24: 17*24
    ���ģ�
        font12CN: 16*21 
        font24CN: 32*41
        
OLED\:��Ŀ¼��ΪOLED��������;
Examples\:��Ŀ¼��ΪOLED�Ĳ��Գ�����������п��������ʹ�÷���;