#include "ugui_button.h"

/* -------------------------------------------------------------------------------- */
/* -- BUTTON FUNCTIONS                                                           -- */
/* -------------------------------------------------------------------------------- */

/* Static functions */
static void _UG_ButtonUpdate(UG_WINDOW* wnd, UG_OBJECT* obj);

const UG_COLOR pal_button_pressed[] = {
    C_PAL_BUTTON_PRESSED
};

const UG_COLOR pal_button_released[] = {
    C_PAL_BUTTON_RELEASED
};

UG_RESULT UG_ButtonCreate( UG_WINDOW* wnd, UG_BUTTON* btn, UG_U8 id, UG_S16 xs, UG_S16 ys, UG_S16 xe, UG_S16 ye )
{
   UG_OBJECT* obj;

   obj = _UG_GetFreeObject( wnd );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   /* Initialize object-specific parameters */
   btn->state = BTN_STATE_RELEASED;
   btn->bc = wnd->bc;
   btn->fc = wnd->fc;
   btn->abc = wnd->bc;
   btn->afc = wnd->fc;
   btn->style = BTN_STYLE_3D;
   btn->align = ALIGN_CENTER;
   btn->font = UG_GetGUI() != NULL ? (UG_GetGUI()->font) : NULL;
   btn->str = "-";

   /* Initialize standard object parameters */
   obj->update = _UG_ButtonUpdate;
   #ifdef UGUI_USE_TOUCH
   obj->touch_state = OBJ_TOUCH_STATE_INIT;
   #endif
   obj->type = OBJ_TYPE_BUTTON;
   obj->event = OBJ_EVENT_NONE;
   obj->a_rel.xs = xs;
   obj->a_rel.ys = ys;
   obj->a_rel.xe = xe;
   obj->a_rel.ye = ye;
   obj->a_abs.xs = -1;
   obj->a_abs.ys = -1;
   obj->a_abs.xe = -1;
   obj->a_abs.ye = -1;
   obj->id = id;
   obj->state |= OBJ_STATE_VISIBLE | OBJ_STATE_REDRAW | OBJ_STATE_VALID;
   #ifdef UGUI_USE_TOUCH
   obj->state |= OBJ_STATE_TOUCH_ENABLE;
   #endif
   obj->data = (void*)btn;

   /* Update function: Do your thing! */
   obj->state &= ~OBJ_STATE_FREE;

   return UG_RESULT_OK;
}

UG_RESULT UG_ButtonDelete( UG_WINDOW* wnd, UG_U8 id )
{
   return _UG_DeleteObject( wnd, OBJ_TYPE_BUTTON, id );
}

UG_RESULT UG_ButtonShow( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   obj->state |= OBJ_STATE_VISIBLE;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_ButtonHide( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   btn = (UG_BUTTON*)(obj->data);

   btn->state &= ~BTN_STATE_PRESSED;
   #ifdef UGUI_USE_TOUCH
   obj->touch_state = OBJ_TOUCH_STATE_INIT;
   #endif
   obj->event = OBJ_EVENT_NONE;
   obj->state &= ~OBJ_STATE_VISIBLE;
   obj->state |= OBJ_STATE_UPDATE;

   return UG_RESULT_OK;
}

UG_RESULT UG_ButtonSetForeColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR fc )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   btn = (UG_BUTTON*)(obj->data);
   btn->fc = fc;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_ButtonSetBackColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR bc )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   btn = (UG_BUTTON*)(obj->data);
   btn->bc = bc;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_ButtonSetAlternateForeColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR afc )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   btn = (UG_BUTTON*)(obj->data);
   btn->afc = afc;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_ButtonSetAlternateBackColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR abc )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   btn = (UG_BUTTON*)(obj->data);
   btn->abc = abc;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_ButtonSetText( UG_WINDOW* wnd, UG_U8 id, char* str )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   btn = (UG_BUTTON*)(obj->data);
   btn->str = str;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_ButtonSetFont( UG_WINDOW* wnd, UG_U8 id, UG_FONT* font )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   btn = (UG_BUTTON*)(obj->data);
   btn->font = font;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_ButtonSetStyle( UG_WINDOW* wnd, UG_U8 id, UG_U8 style )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   btn = (UG_BUTTON*)(obj->data);

   /* Select color scheme */
   btn->style &= ~(BTN_STYLE_USE_ALTERNATE_COLORS | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS | BTN_STYLE_NO_FILL);
   btn->state |= BTN_STATE_ALWAYS_REDRAW;
   if ( style & BTN_STYLE_NO_BORDERS )
   {
      btn->style |= BTN_STYLE_NO_BORDERS;
   }
   if ( style & BTN_STYLE_NO_FILL )
   {
      btn->style |= BTN_STYLE_NO_FILL;
   }
   if ( style & BTN_STYLE_TOGGLE_COLORS )
   {
      btn->style |= BTN_STYLE_TOGGLE_COLORS;
   }
   else if ( style & BTN_STYLE_USE_ALTERNATE_COLORS )
   {
      btn->style |= BTN_STYLE_USE_ALTERNATE_COLORS;
   }
   else
   {
      btn->state &= ~BTN_STATE_ALWAYS_REDRAW;
   }

   /* 3D or 2D */
   if ( style & BTN_STYLE_3D )
   {
      btn->style |= BTN_STYLE_3D;
   }
   else
   {
      btn->style &= ~BTN_STYLE_3D;
   }   
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_ButtonSetHSpace( UG_WINDOW* wnd, UG_U8 id, UG_S8 hs )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   btn = (UG_BUTTON*)(obj->data);
   btn->h_space = hs;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_ButtonSetVSpace( UG_WINDOW* wnd, UG_U8 id, UG_S8 vs )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   btn = (UG_BUTTON*)(obj->data);
   btn->v_space = vs;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_ButtonSetAlignment( UG_WINDOW* wnd, UG_U8 id, UG_U8 align )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   btn = (UG_BUTTON*)(obj->data);
   btn->align = align;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_COLOR UG_ButtonGetForeColor( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;
   UG_COLOR c = C_BLACK;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj != NULL )
   {
      btn = (UG_BUTTON*)(obj->data);
      c = btn->fc;
   }
   return c;
}

UG_COLOR UG_ButtonGetBackColor( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;
   UG_COLOR c = C_BLACK;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj != NULL )
   {
      btn = (UG_BUTTON*)(obj->data);
      c = btn->bc;
   }
   return c;
}

UG_COLOR UG_ButtonGetAlternateForeColor( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;
   UG_COLOR c = C_BLACK;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj != NULL )
   {
      btn = (UG_BUTTON*)(obj->data);
      c = btn->afc;
   }
   return c;
}

UG_COLOR UG_ButtonGetAlternateBackColor( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;
   UG_COLOR c = C_BLACK;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj != NULL )
   {
      btn = (UG_BUTTON*)(obj->data);
      c = btn->abc;
   }
   return c;
}

char* UG_ButtonGetText( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;
   char* str = NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj != NULL )
   {
      btn = (UG_BUTTON*)(obj->data);
      str = btn->str;
   }
   return str;
}

UG_FONT* UG_ButtonGetFont( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;
   UG_FONT* font = NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj != NULL )
   {
      btn = (UG_BUTTON*)(obj->data);
      font = (UG_FONT*)btn->font;
   }
   return font;
}

UG_U8 UG_ButtonGetStyle( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;
   UG_U8 style = 0;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj != NULL )
   {
      btn = (UG_BUTTON*)(obj->data);
      style = btn->style;
   }
   return style;
}

UG_S8 UG_ButtonGetHSpace( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;
   UG_S8 hs = 0;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj != NULL )
   {
      btn = (UG_BUTTON*)(obj->data);
      hs = btn->h_space;
   }
   return hs;
}

UG_S8 UG_ButtonGetVSpace( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;
   UG_S8 vs = 0;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj != NULL )
   {
      btn = (UG_BUTTON*)(obj->data);
      vs = btn->v_space;
   }
   return vs;
}

UG_U8 UG_ButtonGetAlignment( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_BUTTON* btn=NULL;
   UG_U8 align = 0;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_BUTTON, id );
   if ( obj != NULL )
   {
      btn = (UG_BUTTON*)(obj->data);
      align = btn->align;
   }
   return align;
}


static void _UG_ButtonUpdate(UG_WINDOW* wnd, UG_OBJECT* obj)
{
   UG_BUTTON* btn;
   UG_AREA a;
   UG_TEXT txt;
   UG_U8 d,o=0;

   /* Get object-specific data */
   btn = (UG_BUTTON*)(obj->data);

   /* -------------------------------------------------- */
   /* Object touch section                               */
   /* -------------------------------------------------- */
   #ifdef UGUI_USE_TOUCH
   if ( (obj->touch_state & OBJ_TOUCH_STATE_CHANGED) )
   {
      /* Is the button pressed down? */
      if ( obj->touch_state & OBJ_TOUCH_STATE_PRESSED_ON_OBJECT )
      {
         btn->state |= BTN_STATE_PRESSED;
         obj->state |= OBJ_STATE_UPDATE;
         obj->event = OBJ_EVENT_PRESSED;
      }
      /* Can we release the button? */
      else if ( btn->state & BTN_STATE_PRESSED )
      {
         btn->state &= ~BTN_STATE_PRESSED;
         obj->state |= OBJ_STATE_UPDATE;
         obj->event = OBJ_EVENT_RELEASED;
      }
      obj->touch_state &= ~OBJ_TOUCH_STATE_CHANGED;
#ifdef BUTTON_TXT_DEPRESS
      obj->state |=  OBJ_STATE_REDRAW;
#endif
   }
   #endif

   /* -------------------------------------------------- */
   /* Object update section                              */
   /* -------------------------------------------------- */
   if ( obj->state & OBJ_STATE_UPDATE )
   {
      if ( obj->state & OBJ_STATE_VISIBLE )
      {
         /* Full redraw necessary? */
         if ( (obj->state & OBJ_STATE_REDRAW) || (btn->state & BTN_STATE_ALWAYS_REDRAW) )
         {
            UG_WindowGetArea(wnd,&a);
            obj->a_abs.xs = obj->a_rel.xs + a.xs;
            obj->a_abs.ys = obj->a_rel.ys + a.ys;
            obj->a_abs.xe = obj->a_rel.xe + a.xs;
            obj->a_abs.ye = obj->a_rel.ye + a.ys;
            if ( obj->a_abs.ye > wnd->ye ) return;
            if ( obj->a_abs.xe > wnd->xe ) return;
#ifdef UGUI_USE_PRERENDER_EVENT
            _UG_SendObjectPrerenderEvent(wnd, obj);
#endif

            /* 3D or 2D style? */
            d = ( btn->style & BTN_STYLE_3D )? 3:1;

            txt.bc = btn->bc;
            txt.fc = btn->fc;

            if( btn->state & BTN_STATE_PRESSED )
            {
              /* o is the text offset, to provide a visual effect when pressed */
              o=1;
               /* "toggle" style? */
               if( btn->style & BTN_STYLE_TOGGLE_COLORS )
               {
                  /* Swap colors */
                  txt.bc = btn->fc;
                  txt.fc = btn->bc;
               }
               /* Use alternate colors? */
               else if ( btn->style & BTN_STYLE_USE_ALTERNATE_COLORS )
               {
                  txt.bc = btn->abc;
                  txt.fc = btn->afc;
               }
            }
            if ( !(btn->style & BTN_STYLE_NO_FILL) )
               UG_FillFrame(obj->a_abs.xs+d, obj->a_abs.ys+d, obj->a_abs.xe-d, obj->a_abs.ye-d, txt.bc);

            /* Draw button text */
            txt.a.xs = obj->a_abs.xs+d+o;
            txt.a.ys = obj->a_abs.ys+d+o;
            txt.a.xe = obj->a_abs.xe-d+o;
            txt.a.ye = obj->a_abs.ye-d+o;
            txt.align = btn->align;
            txt.font = btn->font;
            txt.h_space = 2;
            txt.v_space = 2;
            txt.str = btn->str;
            _UG_PutText( &txt );
            obj->state &= ~OBJ_STATE_REDRAW;
#ifdef UGUI_USE_POSTRENDER_EVENT
            _UG_SendObjectPostrenderEvent(wnd, obj);
#endif
         }
         /* Draw button frame */
         if ( !(btn->style & BTN_STYLE_NO_BORDERS) )
         {
             if ( btn->style & BTN_STYLE_3D )
             {  /* 3D */
                _UG_DrawObjectFrame(obj->a_abs.xs,obj->a_abs.ys,obj->a_abs.xe,obj->a_abs.ye, (btn->state&BTN_STATE_PRESSED)?(UG_COLOR*)pal_button_pressed:(UG_COLOR*)pal_button_released);
             }
             else
             {  /* 2D */
                 UG_DrawFrame(obj->a_abs.xs,obj->a_abs.ys,obj->a_abs.xe,obj->a_abs.ye,(btn->state&BTN_STATE_PRESSED)?btn->abc:btn->afc);
             }
         }
      }
      else
      {
          if ( !(btn->style & BTN_STYLE_NO_FILL) )
            UG_FillFrame(obj->a_abs.xs, obj->a_abs.ys, obj->a_abs.xe, obj->a_abs.ye, wnd->bc);
      }
      obj->state &= ~OBJ_STATE_UPDATE;
   }
}
