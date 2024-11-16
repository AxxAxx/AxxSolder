#ifndef __UGUI_THEME_H
#define __UGUI_THEME_H

#include "ugui_colors.h"

#if defined(UGUI_USE_COLOR_RGB888)

#define C_DESKTOP_COLOR                0x5E8BEF
#define C_FORE_COLOR                   C_BLACK
#define C_BACK_COLOR                   0xF0F0F0
#define C_TITLE_FORE_COLOR             C_WHITE
#define C_TITLE_BACK_COLOR             C_BLUE
#define C_INACTIVE_TITLE_FORE_COLOR    C_WHITE
#define C_INACTIVE_TITLE_BACK_COLOR    C_GRAY

#define C_PAL_WINDOW                   /* Frame 0 */ \
                                       C_WHITE_39,   \
                                       C_WHITE_39,   \
                                       C_WHITE_39,   \
                                       C_WHITE_39,   \
                                       /* Frame 1 */ \
                                       C_WHITE_89,   \
                                       C_WHITE_89,   \
                                       C_WHITE_41,   \
                                       C_WHITE_41,   \
                                       /* Frame 2 */ \
                                       C_WHITE,      \
                                       C_WHITE,      \
                                       C_WHITE_63,   \
                                       C_WHITE_63

#define C_PAL_BUTTON_PRESSED           /* Frame 0 */ \
                                       C_WHITE_39,   \
                                       C_WHITE_39,   \
                                       C_WHITE_39,   \
                                       C_WHITE_39,   \
                                       /* Frame 1 */ \
                                       C_WHITE_63,   \
                                       C_WHITE_63,   \
                                       C_WHITE_63,   \
                                       C_WHITE_63,   \
                                       /* Frame 2 */ \
                                       C_WHITE_94,   \
                                       C_WHITE_94,   \
                                       C_WHITE_94,   \
                                       C_WHITE_94
#define C_PAL_BUTTON_RELEASED          C_PAL_WINDOW

#define C_PAL_CHECKBOX_PRESSED         C_PAL_BUTTON_PRESSED
#define C_PAL_CHECKBOX_RELEASED        C_PAL_BUTTON_RELEASED

#define C_PAL_PROGRESS                 C_PAL_BUTTON_RELEASED

#elif defined(UGUI_USE_COLOR_RGB565)

#define C_DESKTOP_COLOR                0x5C5D
#define C_FORE_COLOR                   C_BLACK
#define C_BACK_COLOR                   C_WHITE_94
#define C_TITLE_FORE_COLOR             C_WHITE
#define C_TITLE_BACK_COLOR             C_BLUE
#define C_INACTIVE_TITLE_FORE_COLOR    C_WHITE
#define C_INACTIVE_TITLE_BACK_COLOR    C_GRAY


#define C_PAL_WINDOW                   /* Frame 0 */ \
                                       C_WHITE_39,   \
                                       C_WHITE_39,   \
                                       C_WHITE_39,   \
                                       C_WHITE_39,   \
                                       /* Frame 1 */ \
                                       C_WHITE,      \
                                       C_WHITE,      \
                                       C_WHITE_41,   \
                                       C_WHITE_41,   \
                                       /* Frame 2 */ \
                                       C_WHITE_89,   \
                                       C_WHITE_89,   \
                                       C_WHITE_63,   \
                                       C_WHITE_63

#define C_PAL_BUTTON_PRESSED           /* Frame 0 */ \
                                       C_WHITE_39,   \
                                       C_WHITE_39,   \
                                       C_WHITE_39,   \
                                       C_WHITE_39,   \
                                       /* Frame 1 */ \
                                       C_WHITE_63,   \
                                       C_WHITE_63,   \
                                       C_WHITE_63,   \
                                       C_WHITE_63,   \
                                       /* Frame 2 */ \
                                       C_WHITE_94,   \
                                       C_WHITE_94,   \
                                       C_WHITE_94,   \
                                       C_WHITE_94
#define C_PAL_BUTTON_RELEASED          C_PAL_WINDOW

#define C_PAL_CHECKBOX_PRESSED         C_PAL_BUTTON_PRESSED
#define C_PAL_CHECKBOX_RELEASED        C_PAL_BUTTON_RELEASED

#define C_PAL_PROGRESS                 C_PAL_BUTTON_RELEASED

#elif defined(UGUI_USE_COLOR_BW)

#define C_DESKTOP_COLOR                C_BLACK
#define C_FORE_COLOR                   C_BLACK
#define C_BACK_COLOR                   C_WHITE
#define C_TITLE_FORE_COLOR             C_BLACK
#define C_TITLE_BACK_COLOR             C_WHITE
#define C_INACTIVE_TITLE_FORE_COLOR    C_WHITE
#define C_INACTIVE_TITLE_BACK_COLOR    C_BLACK

#define C_PAL_WINDOW                   /* Frame 0 */ \
                                       C_BLACK,      \
                                       C_BLACK,      \
                                       C_BLACK,      \
                                       C_BLACK,      \
                                       /* Frame 1 */ \
                                       C_WHITE,      \
                                       C_WHITE,      \
                                       C_BLACK,      \
                                       C_BLACK,      \
                                       /* Frame 2 */ \
                                       C_WHITE,      \
                                       C_WHITE,      \
                                       C_BLACK,      \
                                       C_BLACK

#define C_PAL_BUTTON_PRESSED           /* Frame 0 */ \
                                       C_BLACK,      \
                                       C_BLACK,      \
                                       C_BLACK,      \
                                       C_BLACK,      \
                                       /* Frame 1 */ \
                                       C_BLACK,      \
                                       C_BLACK,      \
                                       C_WHITE,      \
                                       C_WHITE,      \
                                       /* Frame 2 */ \
                                       C_WHITE,      \
                                       C_WHITE,      \
                                       C_WHITE,      \
                                       C_WHITE
#define C_PAL_BUTTON_RELEASED          /* Frame 0 */ \
                                       C_BLACK,      \
                                       C_BLACK,      \
                                       C_BLACK,      \
                                       C_BLACK,      \
                                       /* Frame 1 */ \
                                       C_WHITE,      \
                                       C_WHITE,      \
                                       C_BLACK,      \
                                       C_BLACK,      \
                                       /* Frame 2 */ \
                                       C_WHITE,      \
                                       C_WHITE,      \
                                       C_WHITE,      \
                                       C_WHITE

#define C_PAL_CHECKBOX_PRESSED         C_PAL_BUTTON_PRESSED
#define C_PAL_CHECKBOX_RELEASED        C_PAL_BUTTON_RELEASED

#define C_PAL_PROGRESS                 C_PAL_BUTTON_RELEASED

#endif

#endif // __UGUI_THEME_H