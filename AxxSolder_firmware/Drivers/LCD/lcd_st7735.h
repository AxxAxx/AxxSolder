/* lcd_st7735.h
 * ST7735 controller: display geometry, initialization sequence, rotation table.
 * Include ONLY from lcd.c after including lcd_config.h.
 */
#ifndef LCD_ST7735_H
#define LCD_ST7735_H

/* ── Display geometry ──────────────────────────────────── */
#if !defined(LCD_160X128) && !defined(LCD_128X128) && !defined(LCD_160X80)
#  error "No valid ST7735 size selected. Pass -DLCD_SIZE=160X128|128X128|160X80 to CMake."
#endif

#ifdef LCD_160X128
#  define LCD_X_SHIFT 0
#  define LCD_Y_SHIFT 0
#  if (LCD_ROTATION == 0) || (LCD_ROTATION == 2)
#    define LCD_WIDTH  128
#    define LCD_HEIGHT 160
#  else
#    define LCD_WIDTH  160
#    define LCD_HEIGHT 128
#  endif
#elif defined LCD_128X128
#  define LCD_X_SHIFT 0
#  define LCD_Y_SHIFT 0
#  define LCD_WIDTH  128
#  define LCD_HEIGHT 128
#elif defined LCD_160X80
#  define LCD_X_SHIFT 0
#  define LCD_Y_SHIFT 0
#  if (LCD_ROTATION == 0) || (LCD_ROTATION == 2)
#    define LCD_WIDTH  80
#    define LCD_HEIGHT 160
#  else
#    define LCD_WIDTH  160
#    define LCD_HEIGHT 80
#  endif
#endif

/* ST7735 160x80 uses BGR order, other variants use RGB */
#ifdef LCD_160X80
#  define ST7735_COLOR_ORDER CMD_MADCTL_BGR
#else
#  define ST7735_COLOR_ORDER CMD_MADCTL_RGB
#endif

/* Initial rotation command — used once in lcd_init_cmd[] */
#if LCD_ROTATION == 0
#  define LCD_ROTATION_CMD (CMD_MADCTL_MX | CMD_MADCTL_MY | ST7735_COLOR_ORDER)
#elif LCD_ROTATION == 1
#  define LCD_ROTATION_CMD (CMD_MADCTL_MY | CMD_MADCTL_MV | ST7735_COLOR_ORDER)
#elif LCD_ROTATION == 2
#  define LCD_ROTATION_CMD (ST7735_COLOR_ORDER)
#elif LCD_ROTATION == 3
#  define LCD_ROTATION_CMD (CMD_MADCTL_MX | CMD_MADCTL_MV | ST7735_COLOR_ORDER)
#endif

/* Runtime rotation: MADCTL byte for each rotation index 0..3 */
static const uint8_t lcd_rotation_cmds[4] = {
    CMD_MADCTL_MX | CMD_MADCTL_MY | ST7735_COLOR_ORDER,  /* 0 */
    CMD_MADCTL_MY | CMD_MADCTL_MV | ST7735_COLOR_ORDER,  /* 1 */
    ST7735_COLOR_ORDER,                                   /* 2 */
    CMD_MADCTL_MX | CMD_MADCTL_MV | ST7735_COLOR_ORDER,  /* 3 */
};

/* Initialization command sequence */
static const uint8_t lcd_init_cmd[] = {
    0,  CMD_SLPOUT,
    3,  CMD_FRMCTR1, 0x01, 0x01, 0x01,
    3,  CMD_FRMCTR2, 0x01, 0x01, 0x01,
    6,  CMD_FRMCTR3, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    1,  CMD_INVCTR,  0x07,
    3,  CMD_PWCTR1,  0xA2, 0x02, 0x84,
    1,  CMD_PWCTR2,  0xC5,
    2,  CMD_PWCTR3,  0x0A, 0x00,
    2,  CMD_PWCTR4,  0x8A, 0x2A,
    2,  CMD_PWCTR5,  0x8A, 0xEE,
    1,  CMD_VMCTR1,  0x0E,
    1,  CMD_INVOFF,  0x00,
    1,  CMD_COLMOD,  0x05,
    2,  CMD_CASET,   0x00, LCD_WIDTH-1,
    2,  CMD_RASET,   0x00, LCD_HEIGHT-1,
    1,  CMD_MADCTL,  LCD_ROTATION_CMD,
    16, CMD_GMCTRP1, 0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d,
                     0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10,
    16, CMD_GMCTRN1, 0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D,
                     0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10,
    0,  CMD_NORON,
};

#endif /* LCD_ST7735_H */
