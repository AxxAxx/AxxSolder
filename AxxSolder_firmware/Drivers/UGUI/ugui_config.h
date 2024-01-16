#ifndef __UGUI_CONFIG_H
#define __UGUI_CONFIG_H

/* -------------------------------------------------------------------------------- */
/* -- CONFIG SECTION                                                             -- */
/* -------------------------------------------------------------------------------- */

#include <stdint.h>

/* Enable color mode */
// #define UGUI_USE_COLOR_RGB888   // RGB = 0xFF,0xFF,0xFF
 #define UGUI_USE_COLOR_RGB565   // RGB = 0bRRRRRGGGGGGBBBBB
// #define UGUI_USE_COLOR_BW   // BW = 0x00|0xFF

/* Enable Touch Support */
 #define UGUI_USE_TOUCH

/* Enable Console Support */
 #define UGUI_USE_CONSOLE

/* If you only use standard ASCII, disabling this will save some resources */
#define UGUI_USE_UTF8

/* Enable needed fonts */
#define UGUI_USE_FONT_4X16
#define UGUI_USE_FONT_5X8
#define UGUI_USE_FONT_5X12
#define UGUI_USE_FONT_6X8
#define UGUI_USE_FONT_6X10
#define UGUI_USE_FONT_7X12
#define UGUI_USE_FONT_8X8
#define UGUI_USE_FONT_8X12
#define UGUI_USE_FONT_8X12_CYRILLIC
#define UGUI_USE_FONT_10X16
#define UGUI_USE_FONT_12X16
#define UGUI_USE_FONT_12X20
#define UGUI_USE_FONT_16X26
#define UGUI_USE_FONT_22X36
#define UGUI_USE_FONT_24X40
#define UGUI_USE_FONT_32X53

#define UGUI_USE_FONT_arial_6X6
#define UGUI_USE_FONT_arial_6X6_CYRILLIC
#define UGUI_USE_FONT_arial_9X10
#define UGUI_USE_FONT_arial_9X10_CYRILLIC
#define UGUI_USE_FONT_arial_10X13
#define UGUI_USE_FONT_arial_10X13_CYRILLIC
#define UGUI_USE_FONT_arial_12X15
#define UGUI_USE_FONT_arial_12X15_CYRILLIC
#define UGUI_USE_FONT_arial_16X18
#define UGUI_USE_FONT_arial_16X18_CYRILLIC
#define UGUI_USE_FONT_arial_20X23
#define UGUI_USE_FONT_arial_20X23_CYRILLIC
#define UGUI_USE_FONT_arial_25X28
#define UGUI_USE_FONT_arial_25X28_CYRILLIC
#define UGUI_USE_FONT_arial_29X35
#define UGUI_USE_FONT_arial_29X35_CYRILLIC
#define UGUI_USE_FONT_arial_35X40
#define UGUI_USE_FONT_arial_35X40_CYRILLIC
#define UGUI_USE_FONT_arial_39X45
#define UGUI_USE_FONT_arial_39X45_CYRILLIC
#define UGUI_USE_FONT_arial_45X52
#define UGUI_USE_FONT_arial_45X52_CYRILLIC
#define UGUI_USE_FONT_arial_49X57
#define UGUI_USE_FONT_arial_49X57_CYRILLIC

/* Feature enablers */
// #define UGUI_USE_PRERENDER_EVENT
// #define UGUI_USE_POSTRENDER_EVENT
// #define UGUI_USE_MULTITASKING

/* Specify platform-dependent types here */

typedef uint8_t      UG_U8;
typedef int8_t       UG_S8;
typedef size_t       UG_SIZE;
typedef uint16_t     UG_U16;
typedef int16_t      UG_S16;
typedef uint32_t     UG_U32;
typedef int32_t      UG_S32;
typedef const UG_U8  UG_FONT;

#endif //__UGUI_CONFIG_H
