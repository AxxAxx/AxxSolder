#ifndef __UGUI_CHECKBOX_H
#define __UGUI_CHECKBOX_H

#include "ugui.h"

/* -------------------------------------------------------------------------------- */
/* -- CHECKBOX OBJECT                                                            -- */
/* -------------------------------------------------------------------------------- */

#if !defined(C_PAL_CHECKBOX_PRESSED) || !defined(C_PAL_CHECKBOX_RELEASED)
#error "You must define a theme!"
#endif

/* Checkbox structure */
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
   UG_U8 checked;
}UG_CHECKBOX;

/* Object type */
#define OBJ_TYPE_CHECKBOX                             4

/* Default checkbox IDs */
#define CHB_ID_0                                      OBJ_ID_0
#define CHB_ID_1                                      OBJ_ID_1
#define CHB_ID_2                                      OBJ_ID_2
#define CHB_ID_3                                      OBJ_ID_3
#define CHB_ID_4                                      OBJ_ID_4
#define CHB_ID_5                                      OBJ_ID_5
#define CHB_ID_6                                      OBJ_ID_6
#define CHB_ID_7                                      OBJ_ID_7
#define CHB_ID_8                                      OBJ_ID_8
#define CHB_ID_9                                      OBJ_ID_9
#define CHB_ID_10                                     OBJ_ID_10
#define CHB_ID_11                                     OBJ_ID_11
#define CHB_ID_12                                     OBJ_ID_12
#define CHB_ID_13                                     OBJ_ID_13
#define CHB_ID_14                                     OBJ_ID_14
#define CHB_ID_15                                     OBJ_ID_15
#define CHB_ID_16                                     OBJ_ID_16
#define CHB_ID_17                                     OBJ_ID_17
#define CHB_ID_18                                     OBJ_ID_18
#define CHB_ID_19                                     OBJ_ID_19

/* Checkbox states */
#define CHB_STATE_RELEASED                            (0<<0)
#define CHB_STATE_PRESSED                             (1<<0)
#define CHB_STATE_ALWAYS_REDRAW                       (1<<1)

/* Checkbox style */
#define CHB_STYLE_2D                                  (0<<0)
#define CHB_STYLE_3D                                  (1<<0)
#define CHB_STYLE_TOGGLE_COLORS                       (1<<1)
#define CHB_STYLE_USE_ALTERNATE_COLORS                (1<<2)
#define CHB_STYLE_NO_BORDERS                          (1<<3)
#define CHB_STYLE_NO_FILL                             (1<<4)

/* Checkbox functions */
UG_RESULT UG_CheckboxCreate( UG_WINDOW* wnd, UG_CHECKBOX* btn, UG_U8 id, UG_S16 xs, UG_S16 ys, UG_S16 xe, UG_S16 ye );
UG_RESULT UG_CheckboxDelete( UG_WINDOW* wnd, UG_U8 id );
UG_RESULT UG_CheckboxShow( UG_WINDOW* wnd, UG_U8 id );
UG_RESULT UG_CheckboxHide( UG_WINDOW* wnd, UG_U8 id );
UG_RESULT UG_CheckboxSetChecked( UG_WINDOW* wnd, UG_U8 id, UG_U8 ch );
UG_RESULT UG_CheckboxSetForeColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR fc );
UG_RESULT UG_CheckboxSetBackColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR bc );
UG_RESULT UG_CheckboxSetAlternateForeColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR afc );
UG_RESULT UG_CheckboxSetAlternateBackColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR abc );
UG_RESULT UG_CheckboxSetText( UG_WINDOW* wnd, UG_U8 id, char* str );
UG_RESULT UG_CheckboxSetFont( UG_WINDOW* wnd, UG_U8 id, UG_FONT* font );
UG_RESULT UG_CheckboxSetStyle( UG_WINDOW* wnd, UG_U8 id, UG_U8 style );
UG_RESULT UG_CheckboxSetHSpace( UG_WINDOW* wnd, UG_U8 id, UG_S8 hs );
UG_RESULT UG_CheckboxSetVSpace( UG_WINDOW* wnd, UG_U8 id, UG_S8 vs );
UG_RESULT UG_CheckboxSetAlignment( UG_WINDOW* wnd, UG_U8 id, UG_U8 align );
UG_U8 UG_CheckboxGetChecked( UG_WINDOW* wnd, UG_U8 id );
UG_COLOR UG_CheckboxGetForeColor( UG_WINDOW* wnd, UG_U8 id );
UG_COLOR UG_CheckboxGetBackColor( UG_WINDOW* wnd, UG_U8 id );
UG_COLOR UG_CheckboxGetAlternateForeColor( UG_WINDOW* wnd, UG_U8 id );
UG_COLOR UG_CheckboxGetAlternateBackColor( UG_WINDOW* wnd, UG_U8 id );
char* UG_CheckboxGetText( UG_WINDOW* wnd, UG_U8 id );
UG_FONT* UG_CheckboxGetFont( UG_WINDOW* wnd, UG_U8 id );
UG_U8 UG_CheckboxGetStyle( UG_WINDOW* wnd, UG_U8 id );
UG_S8 UG_CheckboxGetHSpace( UG_WINDOW* wnd, UG_U8 id );
UG_S8 UG_CheckboxGetVSpace( UG_WINDOW* wnd, UG_U8 id );
UG_U8 UG_CheckboxGetAlignment( UG_WINDOW* wnd, UG_U8 id );

#endif // __UGUI_CHECKBOX_H