/* lcd_st7789.h
 * ST7789 controller: display geometry, initialization sequence, rotation table.
 * Include ONLY from lcd.c after including lcd_config.h.
 */
#ifndef LCD_ST7789_H
#define LCD_ST7789_H

/* ── Display geometry ──────────────────────────────────── */
#if !defined(LCD_135X240) && !defined(LCD_240X240) && \
    !defined(LCD_240X280) && !defined(LCD_240X320)
#  error "No valid ST7789 size selected. Pass -DLCD_SIZE=135X240|240X240|240X280|240X320 to CMake."
#endif

#ifdef LCD_135X240
#  if (LCD_ROTATION == 0) || (LCD_ROTATION == 2)
#    define LCD_WIDTH   135
#    define LCD_HEIGHT  240
#    define LCD_X_SHIFT 53
#    define LCD_Y_SHIFT 40
#  else
#    define LCD_WIDTH   240
#    define LCD_HEIGHT  135
#    define LCD_X_SHIFT 40
#    define LCD_Y_SHIFT 52
#  endif
#elif defined LCD_240X240
#  define LCD_WIDTH   240
#  define LCD_HEIGHT  240
#  define LCD_X_SHIFT 0
#  define LCD_Y_SHIFT 0
#elif defined LCD_240X280
#  if (LCD_ROTATION == 0) || (LCD_ROTATION == 2)
#    define LCD_WIDTH   240
#    define LCD_HEIGHT  280
#    define LCD_X_SHIFT 0
#    define LCD_Y_SHIFT 0
#  else
#    define LCD_WIDTH   280
#    define LCD_HEIGHT  240
#    define LCD_X_SHIFT 20
#    define LCD_Y_SHIFT 0
#  endif
#elif defined LCD_240X320
#  if (LCD_ROTATION == 0) || (LCD_ROTATION == 2)
#    define LCD_WIDTH   240
#    define LCD_HEIGHT  320
#    define LCD_X_SHIFT 0
#    define LCD_Y_SHIFT 0
#  else
#    define LCD_WIDTH   320
#    define LCD_HEIGHT  240
#    define LCD_X_SHIFT 0
#    define LCD_Y_SHIFT 0
#  endif
#endif

/* Initial rotation command — used once in lcd_init_cmd[] */
#if LCD_ROTATION == 0
#  define LCD_ROTATION_CMD (CMD_MADCTL_MX | CMD_MADCTL_MY | CMD_MADCTL_RGB)
#elif LCD_ROTATION == 1
#  define LCD_ROTATION_CMD (CMD_MADCTL_MY | CMD_MADCTL_MV | CMD_MADCTL_RGB)
#elif LCD_ROTATION == 2
#  define LCD_ROTATION_CMD (CMD_MADCTL_RGB)
#elif LCD_ROTATION == 3
#  define LCD_ROTATION_CMD (CMD_MADCTL_MX | CMD_MADCTL_MV | CMD_MADCTL_RGB)
#endif

/* Runtime rotation: MADCTL byte for each rotation index 0..3 */
static const uint8_t lcd_rotation_cmds[4] = {
    CMD_MADCTL_RGB,                                   /* 0 */
    CMD_MADCTL_MX | CMD_MADCTL_MV | CMD_MADCTL_RGB,  /* 1 */
    CMD_MADCTL_MX | CMD_MADCTL_MY | CMD_MADCTL_RGB,  /* 2 */
    CMD_MADCTL_MY | CMD_MADCTL_MV | CMD_MADCTL_RGB,  /* 3 */
};

/* Initialization command sequence */
static const uint8_t lcd_init_cmd[] = {
    0,  CMD_SLPOUT,
    1,  CMD_COLMOD,  CMD_COLOR_MODE_16bit,
    5,  CMD_PORCTRL, 0x0C, 0x0C, 0x00, 0x33, 0x33,
    1,  CMD_GCTRL,   0x35,
    1,  CMD_VCOMS,   0x19,
    1,  CMD_LCMCTRL, 0X2C,
    1,  CMD_VDVVRHEN,0x01,
    1,  CMD_VRHS,    0x12,
    1,  CMD_VDVS,    0x20,
    1,  CMD_FRCTRL2, 0x0F,
    2,  CMD_PWCTRL1, 0xA4, 0xA1,
    1,  CMD_MADCTL,  LCD_ROTATION_CMD,
    14, CMD_GMCTRP1, 0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23,
    14, CMD_GMCTRN1, 0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23,
    0,  CMD_INVON,
    0,  CMD_NORON,
};

#endif /* LCD_ST7789_H */
