#ifndef __UGUI_PROGRESS_H
#define __UGUI_PROGRESS_H

#include "ugui.h"

/* -------------------------------------------------------------------------------- */
/* -- PROGRESS OBJECT                                                             -- */
/* -------------------------------------------------------------------------------- */
/* Progress Bar structure */
typedef struct
{
   UG_U8 style;
   UG_COLOR fc;
   UG_COLOR bc;
   UG_U8 progress;
} UG_PROGRESS;

/* Object type */
#define OBJ_TYPE_PROGRESS                             5

/* Default Progress Bar IDs */
#define PGB_ID_0                                      OBJ_ID_0
#define PGB_ID_1                                      OBJ_ID_1
#define PGB_ID_2                                      OBJ_ID_2
#define PGB_ID_3                                      OBJ_ID_3
#define PGB_ID_4                                      OBJ_ID_4
#define PGB_ID_5                                      OBJ_ID_5
#define PGB_ID_6                                      OBJ_ID_6
#define PGB_ID_7                                      OBJ_ID_7
#define PGB_ID_8                                      OBJ_ID_8
#define PGB_ID_9                                      OBJ_ID_9
#define PGB_ID_10                                     OBJ_ID_10
#define PGB_ID_11                                     OBJ_ID_11
#define PGB_ID_12                                     OBJ_ID_12
#define PGB_ID_13                                     OBJ_ID_13
#define PGB_ID_14                                     OBJ_ID_14
#define PGB_ID_15                                     OBJ_ID_15
#define PGB_ID_16                                     OBJ_ID_16
#define PGB_ID_17                                     OBJ_ID_17
#define PGB_ID_18                                     OBJ_ID_18
#define PGB_ID_19                                     OBJ_ID_19

/* Progress Bar style */
#define PGB_STYLE_2D                                  (0<<0)
#define PGB_STYLE_3D                                  (1<<0)
#define PGB_STYLE_NO_BORDERS                          (1<<1)
#define PGB_STYLE_FORE_COLOR_MESH                     (1<<2)
#define PGB_STYLE_NO_FILL                             (1<<3)

/* Progress Bar functions */
UG_RESULT UG_ProgressCreate( UG_WINDOW* wnd, UG_PROGRESS* pgb, UG_U8 id, UG_S16 xs, UG_S16 ys, UG_S16 xe, UG_S16 ye );
UG_RESULT UG_ProgressDelete( UG_WINDOW* wnd, UG_U8 id );
UG_RESULT UG_ProgressShow( UG_WINDOW* wnd, UG_U8 id );
UG_RESULT UG_ProgressHide( UG_WINDOW* wnd, UG_U8 id );
UG_RESULT UG_ProgressSetForeColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR fc );
UG_RESULT UG_ProgressSetBackColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR bc );
UG_RESULT UG_ProgressSetStyle( UG_WINDOW* wnd, UG_U8 id, UG_U8 style );
UG_RESULT UG_ProgressSetProgress( UG_WINDOW* wnd, UG_U8 id, UG_U8 progress );
UG_COLOR UG_ProgressGetForeColor( UG_WINDOW* wnd, UG_U8 id );
UG_COLOR UG_ProgressGetBackColor( UG_WINDOW* wnd, UG_U8 id );
UG_U8 UG_ProgressGetStyle( UG_WINDOW* wnd, UG_U8 id );
UG_U8 UG_ProgressGetProgress( UG_WINDOW* wnd, UG_U8 id );

#endif // __UGUI_PROGRESS_H