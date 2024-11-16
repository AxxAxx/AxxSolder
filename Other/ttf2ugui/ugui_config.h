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
// #define UGUI_USE_TOUCH

/* Enable Console Support */
 //#define UGUI_USE_CONSOLE

/* If you only use standard ASCII, disabling this will save some resources */
#define UGUI_USE_UTF8

/* Enable needed fonts */

/* Feature enablers */
// #define UGUI_USE_PRERENDER_EVENT
// #define UGUI_USE_POSTRENDER_EVENT
// #define UGUI_USE_MULTITASKING

/* Specify platform-dependent types here */

typedef uint8_t      UG_U8;
typedef int8_t       UG_S8;
typedef uint16_t     UG_U16;
typedef int16_t      UG_S16;
typedef uint32_t     UG_U32;
typedef int32_t      UG_S32;
typedef const UG_U8  UG_FONT;

#endif //__UGUI_CONFIG_H
