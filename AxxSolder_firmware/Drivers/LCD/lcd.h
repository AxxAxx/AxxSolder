#ifndef __LCD_H__
#define __LCD_H__

#include "images.h"
#include "ugui.h"
#include "main.h"

/* For demo only. Minimum is 32, 128 and higher will enable all tests */
#define DEMO_FLASH_KB 128

/* LCD Commands */
typedef enum{
  CMD_MADCTL_MY  = 0x80,
  CMD_MADCTL_MX  = 0x40,
  CMD_MADCTL_MV  = 0x20,
  CMD_MADCTL_ML  = 0x10,
  CMD_MADCTL_RGB = 0x00,
  CMD_MADCTL_BGR = 0x08,
  CMD_MADCTL_MH  = 0x04,
  CMD_NOP        = 0x00,
  CMD_SWRESET    = 0x01,
  CMD_RDDID      = 0x04,
  CMD_RDDST      = 0x09,
  CMD_SLPIN      = 0x10,
  CMD_SLPOUT     = 0x11,
  CMD_PTLON      = 0x12,
  CMD_NORON      = 0x13,
  CMD_INVOFF     = 0x20,
  CMD_INVON      = 0x21,
  CMD_GAMSET     = 0x26,
  CMD_DISPOFF    = 0x28,
  CMD_DISPON     = 0x29,
  CMD_CASET      = 0x2A,
  CMD_RASET      = 0x2B,
  CMD_RAMWR      = 0x2C,
  CMD_RAMRD      = 0x2E,
  CMD_PTLAR      = 0x30,
  CMD_MADCTL     = 0x36,
  CMD_IDMOFF     = 0x38,
  CMD_IDMON      = 0x39,
  CMD_COLMOD     = 0x3A,
  CMD_RAMCTRL    = 0xB0,
  CMD_FRMCTR1    = 0xB1,
  CMD_RGBCTRL    = 0xB1,
  CMD_FRMCTR2    = 0xB2,
  CMD_PORCTRL    = 0xB2,
  CMD_FRMCTR3    = 0xB3,
  CMD_FRCTRL1    = 0xB3,
  CMD_INVCTR     = 0xB4,
  CMD_PARCTRL    = 0xB5,
  CMD_DISSET5    = 0xB6,
  CMD_GCTRL      = 0xB7,
  CMD_VCOMS      = 0xBB,
  CMD_PWCTR1     = 0xC0,
  CMD_LCMCTRL    = 0xC0,
  CMD_PWCTR2     = 0xC1,
  CMD_IDSET      = 0xC1,
  CMD_PWCTR3     = 0xC2,
  CMD_VDVVRHEN   = 0xC2,
  CMD_PWCTR4     = 0xC3,
  CMD_VRHS       = 0xC3,
  CMD_PWCTR5     = 0xC4,
  CMD_VDVS       = 0xC4,
  CMD_VMCTR1     = 0xC5,
  CMD_VCMOFSET   = 0xC5,
  CMD_FRCTRL2    = 0xC6,
  CMD_PWCTRL1    = 0xD0,
  CMD_RDID1      = 0xDA,
  CMD_RDID2      = 0xDB,
  CMD_RDID3      = 0xDC,
  CMD_RDID4      = 0xDD,
  CMD_PWCTR6     = 0xFC,
  CMD_GMCTRP1    = 0xE0,
  CMD_GMCTRN1    = 0xE1,
  CMD_COLOR_MODE_16bit = 0x55,
  CMD_COLOR_MODE_18bit = 0x66,
}lcd_cmds;

#define color565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))
#define ABS(x) ((x) > 0 ? (x) : -(x))

/* ── Hardware ──────────────────────────────────────────── */
#define LCD_HANDLE            hspi2
extern SPI_HandleTypeDef      LCD_HANDLE;

#define LCD_DC                SPI2_DC_Pin
#define LCD_RST               SPI2_RST_Pin
#define LCD_CS                SPI2_CS_Pin

#define USE_DMA
/* #define LCD_LOCAL_FB */   /* Uncomment to use local frame buffer (needs ~150 KB RAM) */
#define DMA_Min_Pixels        32

/* ── Build-time validation ─────────────────────────────── */
#if !defined(LCD_ROTATION)
#  define LCD_ROTATION 2   /* default: 180° */
#endif

/* Defaults for IDE builds that don't pass CMake defines */
#if !defined(USE_ST7789) && !defined(USE_ST7735)
#  define USE_ST7789
#endif
#if !defined(LCD_240X320) && !defined(LCD_240X280) && !defined(LCD_240X240) && !defined(LCD_135X240) && \
    !defined(LCD_160X128) && !defined(LCD_128X128) && !defined(LCD_160X80)
#  define LCD_240X320
#endif


/* ── Controller header (geometry + rotation table + init_cmd) ── */
#if defined USE_ST7789
#  include "lcd_st7789.h"
#elif defined USE_ST7735
#  include "lcd_st7735.h"
#endif

void LCD_init(void);
void LCD_SetRotation(uint8_t m);
void LCD_DrawPixel(int16_t x, int16_t y, uint16_t color);
void LCD_DrawPixelFB(int16_t x, int16_t y, uint16_t color);
int8_t LCD_Fill(uint16_t xSta, uint16_t ySta, uint16_t xEnd, uint16_t yEnd, uint16_t color);

int8_t LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_DrawImage(uint16_t x, uint16_t y, UG_BMP* bmp);
void LCD_InvertColors(uint8_t invert);

void LCD_PutChar(uint16_t x, uint16_t y, char ch, UG_FONT* font, uint16_t color, uint16_t bgcolor);
void LCD_PutStr(uint16_t x, uint16_t y, char *str, UG_FONT* font, uint16_t color, uint16_t bgcolor);

void LCD_TearEffect(uint8_t tear);

/* Simple test function. */
#ifdef LCD_ENABLE_TEST
void LCD_Test(void);
#endif

#endif /* __LCD_H__ */
