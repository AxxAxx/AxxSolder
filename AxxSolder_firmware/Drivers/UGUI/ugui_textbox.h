#ifndef __UGUI_TEXTBOX_H
#define __UGUI_TEXTBOX_H

#include "ugui.h"

/* -------------------------------------------------------------------------------- */
/* -- TEXTBOX OBJECT                                                             -- */
/* -------------------------------------------------------------------------------- */
/* Textbox structure */
typedef struct
{
   char* str;
   UG_FONT* font;
   UG_U8 style;
   UG_COLOR fc;
   UG_COLOR bc;
   UG_U8 align;
   UG_S8 h_space;
   UG_S8 v_space;
} UG_TEXTBOX;

/* Object type */
#define OBJ_TYPE_TEXTBOX                              2

/* Default textbox IDs */
#define TXB_ID_0                                      OBJ_ID_0
#define TXB_ID_1                                      OBJ_ID_1
#define TXB_ID_2                                      OBJ_ID_2
#define TXB_ID_3                                      OBJ_ID_3
#define TXB_ID_4                                      OBJ_ID_4
#define TXB_ID_5                                      OBJ_ID_5
#define TXB_ID_6                                      OBJ_ID_6
#define TXB_ID_7                                      OBJ_ID_7
#define TXB_ID_8                                      OBJ_ID_8
#define TXB_ID_9                                      OBJ_ID_9
#define TXB_ID_10                                     OBJ_ID_10
#define TXB_ID_11                                     OBJ_ID_11
#define TXB_ID_12                                     OBJ_ID_12
#define TXB_ID_13                                     OBJ_ID_13
#define TXB_ID_14                                     OBJ_ID_14
#define TXB_ID_15                                     OBJ_ID_15
#define TXB_ID_16                                     OBJ_ID_16
#define TXB_ID_17                                     OBJ_ID_17
#define TXB_ID_18                                     OBJ_ID_18
#define TXB_ID_19                                     OBJ_ID_19

/* Textbox functions */
UG_RESULT UG_TextboxCreate( UG_WINDOW* wnd, UG_TEXTBOX* txb, UG_U8 id, UG_S16 xs, UG_S16 ys, UG_S16 xe, UG_S16 ye );
UG_RESULT UG_TextboxDelete( UG_WINDOW* wnd, UG_U8 id );
UG_RESULT UG_TextboxShow( UG_WINDOW* wnd, UG_U8 id );
UG_RESULT UG_TextboxHide( UG_WINDOW* wnd, UG_U8 id );
UG_RESULT UG_TextboxSetForeColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR fc );
UG_RESULT UG_TextboxSetBackColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR bc );
UG_RESULT UG_TextboxSetText( UG_WINDOW* wnd, UG_U8 id, char* str );
UG_RESULT UG_TextboxSetFont( UG_WINDOW* wnd, UG_U8 id, UG_FONT* font );
UG_RESULT UG_TextboxSetHSpace( UG_WINDOW* wnd, UG_U8 id, UG_S8 hs );
UG_RESULT UG_TextboxSetVSpace( UG_WINDOW* wnd, UG_U8 id, UG_S8 vs );
UG_RESULT UG_TextboxSetAlignment( UG_WINDOW* wnd, UG_U8 id, UG_U8 align );
UG_COLOR UG_TextboxGetForeColor( UG_WINDOW* wnd, UG_U8 id );
UG_COLOR UG_TextboxGetBackColor( UG_WINDOW* wnd, UG_U8 id );
char* UG_TextboxGetText( UG_WINDOW* wnd, UG_U8 id );
UG_FONT* UG_TextboxGetFont( UG_WINDOW* wnd, UG_U8 id );
UG_S8 UG_TextboxGetHSpace( UG_WINDOW* wnd, UG_U8 id );
UG_S8 UG_TextboxGetVSpace( UG_WINDOW* wnd, UG_U8 id );
UG_U8 UG_TextboxGetAlignment( UG_WINDOW* wnd, UG_U8 id );

#endif // __UGUI_TEXTBOX_H