#ifndef __UGUI_BUTTON_H
#define __UGUI_BUTTON_H

#include "ugui.h"

/* -------------------------------------------------------------------------------- */
/* -- BUTTON OBJECT                                                              -- */
/* -------------------------------------------------------------------------------- */

#if !defined(C_PAL_BUTTON_PRESSED) || !defined(C_PAL_BUTTON_RELEASED)
#error "You must define a theme!"
#endif

/* Button structure */
typedef struct
{
   UG_U8 state;
   UG_U8 style;
   UG_COLOR fc;
   UG_COLOR bc;
   UG_COLOR afc;
   UG_COLOR abc;
   UG_FONT* font;
   UG_U8 align;
   UG_S8 h_space;
   UG_S8 v_space;
   char* str;
}UG_BUTTON;

/* Depress text on touch, giving additional visual feedback, but requires full button redraw  */
#define BUTTON_TXT_DEPRESS

/* Object type */
#define OBJ_TYPE_BUTTON                               1

/* Default button IDs */
#define BTN_ID_0                                      OBJ_ID_0
#define BTN_ID_1                                      OBJ_ID_1
#define BTN_ID_2                                      OBJ_ID_2
#define BTN_ID_3                                      OBJ_ID_3
#define BTN_ID_4                                      OBJ_ID_4
#define BTN_ID_5                                      OBJ_ID_5
#define BTN_ID_6                                      OBJ_ID_6
#define BTN_ID_7                                      OBJ_ID_7
#define BTN_ID_8                                      OBJ_ID_8
#define BTN_ID_9                                      OBJ_ID_9
#define BTN_ID_10                                     OBJ_ID_10
#define BTN_ID_11                                     OBJ_ID_11
#define BTN_ID_12                                     OBJ_ID_12
#define BTN_ID_13                                     OBJ_ID_13
#define BTN_ID_14                                     OBJ_ID_14
#define BTN_ID_15                                     OBJ_ID_15
#define BTN_ID_16                                     OBJ_ID_16
#define BTN_ID_17                                     OBJ_ID_17
#define BTN_ID_18                                     OBJ_ID_18
#define BTN_ID_19                                     OBJ_ID_19

/* Button states */
#define BTN_STATE_RELEASED                            (0<<0)
#define BTN_STATE_PRESSED                             (1<<0)
#define BTN_STATE_ALWAYS_REDRAW                       (1<<1)

/* Button style */
#define BTN_STYLE_2D                                  (0<<0)
#define BTN_STYLE_3D                                  (1<<0)
#define BTN_STYLE_TOGGLE_COLORS                       (1<<1)
#define BTN_STYLE_USE_ALTERNATE_COLORS                (1<<2)
#define BTN_STYLE_NO_BORDERS                          (1<<3)
#define BTN_STYLE_NO_FILL                             (1<<4)

/* Button functions */
UG_RESULT UG_ButtonCreate( UG_WINDOW* wnd, UG_BUTTON* btn, UG_U8 id, UG_S16 xs, UG_S16 ys, UG_S16 xe, UG_S16 ye );
UG_RESULT UG_ButtonDelete( UG_WINDOW* wnd, UG_U8 id );
UG_RESULT UG_ButtonShow( UG_WINDOW* wnd, UG_U8 id );
UG_RESULT UG_ButtonHide( UG_WINDOW* wnd, UG_U8 id );
UG_RESULT UG_ButtonSetForeColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR fc );
UG_RESULT UG_ButtonSetBackColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR bc );
UG_RESULT UG_ButtonSetAlternateForeColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR afc );
UG_RESULT UG_ButtonSetAlternateBackColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR abc );
UG_RESULT UG_ButtonSetText( UG_WINDOW* wnd, UG_U8 id, char* str );
UG_RESULT UG_ButtonSetFont( UG_WINDOW* wnd, UG_U8 id, UG_FONT* font );
UG_RESULT UG_ButtonSetStyle( UG_WINDOW* wnd, UG_U8 id, UG_U8 style );
UG_RESULT UG_ButtonSetHSpace( UG_WINDOW* wnd, UG_U8 id, UG_S8 hs );
UG_RESULT UG_ButtonSetVSpace( UG_WINDOW* wnd, UG_U8 id, UG_S8 vs );
UG_RESULT UG_ButtonSetAlignment( UG_WINDOW* wnd, UG_U8 id, UG_U8 align );
UG_COLOR UG_ButtonGetForeColor( UG_WINDOW* wnd, UG_U8 id );
UG_COLOR UG_ButtonGetBackColor( UG_WINDOW* wnd, UG_U8 id );
UG_COLOR UG_ButtonGetAlternateForeColor( UG_WINDOW* wnd, UG_U8 id );
UG_COLOR UG_ButtonGetAlternateBackColor( UG_WINDOW* wnd, UG_U8 id );
char* UG_ButtonGetText( UG_WINDOW* wnd, UG_U8 id );
UG_FONT* UG_ButtonGetFont( UG_WINDOW* wnd, UG_U8 id );
UG_U8 UG_ButtonGetStyle( UG_WINDOW* wnd, UG_U8 id );
UG_S8 UG_ButtonGetHSpace( UG_WINDOW* wnd, UG_U8 id );
UG_S8 UG_ButtonGetVSpace( UG_WINDOW* wnd, UG_U8 id );
UG_U8 UG_ButtonGetAlignment( UG_WINDOW* wnd, UG_U8 id );

#endif // __UGUI_BUTTON_H
