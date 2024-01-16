#ifndef __UGUI_FONTS_DATA_H
#define __UGUI_FONTS_DATA_H

/* -------------------------------------------------------------------------------- */
/* -- µGUI FONTS                                                                 -- */
/* -- Source: http://www.mikrocontroller.net/user/show/benedikt                  -- */
/* -------------------------------------------------------------------------------- */
#include "ugui.h"
/*
  Old fonts converted to new structure. They use Code Page 850 encoding. UTF-8 can be disabled if only these fonts are being used.
  https://en.wikipedia.org/wiki/Code_page_850
*/

#ifdef UGUI_USE_FONT_4X16
extern UG_FONT FONT_4X6[];
#endif

#ifdef UGUI_USE_FONT_5X8
extern UG_FONT FONT_5X8[];
#endif

#ifdef UGUI_USE_FONT_5X12
extern UG_FONT FONT_5X12[];
#endif

#ifdef UGUI_USE_FONT_6X8
extern UG_FONT FONT_6X8[];
#endif

#ifdef UGUI_USE_FONT_6X10
extern UG_FONT FONT_6X10[];
#endif

#ifdef UGUI_USE_FONT_7X12
extern UG_FONT FONT_7X12[];
#endif

#ifdef UGUI_USE_FONT_8X8
extern UG_FONT FONT_8X8[];
#endif

#ifdef UGUI_USE_FONT_8X12
extern UG_FONT FONT_8X12[];
#endif

#ifdef UGUI_USE_FONT_8X12_CYRILLIC
extern UG_FONT FONT_8X12_CYRILLIC[];
#endif

#ifdef UGUI_USE_FONT_10X16
extern UG_FONT FONT_10X16[];
#endif

#ifdef UGUI_USE_FONT_12X16
extern UG_FONT FONT_12X16[];
#endif

#ifdef UGUI_USE_FONT_12X20
extern UG_FONT FONT_12X20[];
#endif

#ifdef UGUI_USE_FONT_16X26
extern UG_FONT FONT_16X26[];
#endif

#ifdef UGUI_USE_FONT_22X36
extern UG_FONT FONT_22X36[];
#endif

#ifdef UGUI_USE_FONT_24X40
extern UG_FONT FONT_24X40[];
#endif

#ifdef UGUI_USE_FONT_32X53
extern UG_FONT FONT_32X53[];
#endif
/*
  New fonts. They use Unicode encoding. Can be generated with the new version of ttf2ugui
  Accessing encodings higher than 127 () require UTF-8 enabled and set the editor settings to use that encoding.
  https://en.wikipedia.org/wiki/List_of_Unicode_characters
*/
#ifdef UGUI_USE_FONT_arial_6X6
extern UG_FONT FONT_arial_6X6[];
#endif
#ifdef UGUI_USE_FONT_arial_6X6_CYRILLIC
extern UG_FONT FONT_arial_6X6_CYRILLIC[];
#endif
#ifdef UGUI_USE_FONT_arial_9X10
extern UG_FONT FONT_arial_9X10[];
#endif
#ifdef UGUI_USE_FONT_arial_9X10_CYRILLIC
extern UG_FONT FONT_arial_9X10_CYRILLIC[];
#endif
#ifdef UGUI_USE_FONT_arial_10X13
extern UG_FONT FONT_arial_10X13[];
#endif
#ifdef UGUI_USE_FONT_arial_10X13_CYRILLIC
extern UG_FONT FONT_arial_10X13_CYRILLIC[];
#endif
#ifdef UGUI_USE_FONT_arial_12X15
extern UG_FONT FONT_arial_12X15[];
#endif
#ifdef UGUI_USE_FONT_arial_12X15_CYRILLIC
extern UG_FONT FONT_arial_12X15_CYRILLIC[];
#endif
#ifdef UGUI_USE_FONT_arial_16X18
extern UG_FONT FONT_arial_16X18[];
#endif
#ifdef UGUI_USE_FONT_arial_16X18_CYRILLIC
extern UG_FONT FONT_arial_16X18_CYRILLIC[];
#endif
#ifdef UGUI_USE_FONT_arial_20X23
extern UG_FONT FONT_arial_20X23[];
#endif
#ifdef UGUI_USE_FONT_arial_20X23_CYRILLIC
extern UG_FONT FONT_arial_20X23_CYRILLIC[];
#endif
#ifdef UGUI_USE_FONT_arial_25X28
extern UG_FONT FONT_arial_25X28[];
#endif
#ifdef UGUI_USE_FONT_arial_25X28_CYRILLIC
extern UG_FONT FONT_arial_25X28_CYRILLIC[];
#endif
#ifdef UGUI_USE_FONT_arial_29X35
extern UG_FONT FONT_arial_29X35[];
#endif
#ifdef UGUI_USE_FONT_arial_29X35_CYRILLIC
extern UG_FONT FONT_arial_29X35_CYRILLIC[];
#endif
#ifdef UGUI_USE_FONT_arial_35X40
extern UG_FONT FONT_arial_35X40[];
#endif
#ifdef UGUI_USE_FONT_arial_35X40_CYRILLIC
extern UG_FONT FONT_arial_35X40_CYRILLIC[];
#endif
#ifdef UGUI_USE_FONT_arial_39X45
extern UG_FONT FONT_arial_39X45[];
#endif
#ifdef UGUI_USE_FONT_arial_39X45_CYRILLIC
extern UG_FONT FONT_arial_39X45_CYRILLIC[];
#endif
#ifdef UGUI_USE_FONT_arial_45X52
extern UG_FONT FONT_arial_45X52[];
#endif
#ifdef UGUI_USE_FONT_arial_45X52_CYRILLIC
extern UG_FONT FONT_arial_45X52_CYRILLIC[];
#endif
#ifdef UGUI_USE_FONT_arial_49X57
extern UG_FONT FONT_arial_49X57[];
#endif
#ifdef UGUI_USE_FONT_arial_49X57_CYRILLIC
extern UG_FONT FONT_arial_49X57_CYRILLIC[];
#endif

#endif // __UGUI_FONTS_DATA_H
