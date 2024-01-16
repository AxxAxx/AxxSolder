#include "ugui_checkbox.h"

/* -------------------------------------------------------------------------------- */
/* -- Checkbox FUNCTIONS                                                           -- */
/* -------------------------------------------------------------------------------- */

/* Static functions */
static void _UG_CheckboxUpdate(UG_WINDOW* wnd, UG_OBJECT* obj);

const UG_COLOR pal_checkbox_pressed[] = {
    C_PAL_CHECKBOX_PRESSED
};

const UG_COLOR pal_checkbox_released[] = {
    C_PAL_CHECKBOX_RELEASED
};

UG_RESULT UG_CheckboxCreate( UG_WINDOW* wnd, UG_CHECKBOX* chb, UG_U8 id, UG_S16 xs, UG_S16 ys, UG_S16 xe, UG_S16 ye )
{
   UG_OBJECT* obj;

   obj = _UG_GetFreeObject( wnd );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   /* Initialize object-specific parameters */
   chb->state = CHB_STATE_RELEASED;
   chb->bc = wnd->bc;
   chb->fc = wnd->fc;
   chb->abc = wnd->bc;
   chb->afc = wnd->fc;
   chb->style = CHB_STYLE_3D;
   chb->align = ALIGN_TOP_LEFT;
   chb->font = UG_GetGUI() != NULL ? (UG_GetGUI()->font) : NULL;
   chb->str = "-";
   chb->checked = 0; 

   /* Initialize standard object parameters */
   obj->update = _UG_CheckboxUpdate;
   #ifdef UGUI_USE_TOUCH
   obj->touch_state = OBJ_TOUCH_STATE_INIT;
   #endif
   obj->type = OBJ_TYPE_CHECKBOX;
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
   obj->data = (void*)chb;

   /* Update function: Do your thing! */
   obj->state &= ~OBJ_STATE_FREE;

   return UG_RESULT_OK;
}

UG_RESULT UG_CheckboxDelete( UG_WINDOW* wnd, UG_U8 id )
{
   return _UG_DeleteObject( wnd, OBJ_TYPE_CHECKBOX, id );
}

UG_RESULT UG_CheckboxShow( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   obj->state |= OBJ_STATE_VISIBLE;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_CheckboxHide( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   chb = (UG_CHECKBOX*)(obj->data);

   chb->state &= ~CHB_STATE_PRESSED;
   #ifdef UGUI_USE_TOUCH
   obj->touch_state = OBJ_TOUCH_STATE_INIT;
   #endif
   obj->event = OBJ_EVENT_NONE;
   obj->state &= ~OBJ_STATE_VISIBLE;
   obj->state |= OBJ_STATE_UPDATE;

   return UG_RESULT_OK;
}

UG_RESULT UG_CheckboxSetChecked( UG_WINDOW* wnd, UG_U8 id, UG_U8 ch )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   chb = (UG_CHECKBOX*)(obj->data);
   chb->checked = ch;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}


UG_RESULT UG_CheckboxSetForeColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR fc )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   chb = (UG_CHECKBOX*)(obj->data);
   chb->fc = fc;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_CheckboxSetBackColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR bc )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   chb = (UG_CHECKBOX*)(obj->data);
   chb->bc = bc;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_CheckboxSetAlternateForeColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR afc )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   chb = (UG_CHECKBOX*)(obj->data);
   chb->afc = afc;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_CheckboxSetAlternateBackColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR abc )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   chb = (UG_CHECKBOX*)(obj->data);
   chb->abc = abc;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_CheckboxSetText( UG_WINDOW* wnd, UG_U8 id, char* str )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   chb = (UG_CHECKBOX*)(obj->data);
   chb->str = str;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_CheckboxSetFont( UG_WINDOW* wnd, UG_U8 id, UG_FONT* font )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   chb = (UG_CHECKBOX*)(obj->data);
   chb->font = font;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_CheckboxSetStyle( UG_WINDOW* wnd, UG_U8 id, UG_U8 style )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chk=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   chk = (UG_CHECKBOX*)(obj->data);

   /* Select color scheme */
   chk->style &= ~(CHB_STYLE_USE_ALTERNATE_COLORS | CHB_STYLE_TOGGLE_COLORS | CHB_STYLE_NO_BORDERS | CHB_STYLE_NO_FILL);
   chk->state |= CHB_STATE_ALWAYS_REDRAW;
   if ( style & CHB_STYLE_NO_BORDERS )
   {
      chk->style |= CHB_STYLE_NO_BORDERS;
   }
   if ( style & CHB_STYLE_NO_FILL )
   {
      chk->style |= CHB_STYLE_NO_FILL;
   }
   if ( style & CHB_STYLE_TOGGLE_COLORS )
   {
      chk->style |= CHB_STYLE_TOGGLE_COLORS;
   }
   else if ( style & CHB_STYLE_USE_ALTERNATE_COLORS )
   {
      chk->style |= CHB_STYLE_USE_ALTERNATE_COLORS;
   }
   else
   {
      chk->state &= ~CHB_STATE_ALWAYS_REDRAW;
   }

   /* 3D or 2D */
   if ( style & CHB_STYLE_3D )
   {
      chk->style |= CHB_STYLE_3D;
   }
   else
   {
      chk->style &= ~CHB_STYLE_3D;
   }
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_CheckboxSetHSpace( UG_WINDOW* wnd, UG_U8 id, UG_S8 hs )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   chb = (UG_CHECKBOX*)(obj->data);
   chb->h_space = hs;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_CheckboxSetVSpace( UG_WINDOW* wnd, UG_U8 id, UG_S8 vs )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   chb = (UG_CHECKBOX*)(obj->data);
   chb->v_space = vs;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_CheckboxSetAlignment( UG_WINDOW* wnd, UG_U8 id, UG_U8 align )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   chb = (UG_CHECKBOX*)(obj->data);
   chb->align = align;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_U8 UG_CheckboxGetChecked( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;
   UG_U8 c = 0;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj != NULL )
   {
      chb = (UG_CHECKBOX*)(obj->data);
      c = chb->checked;
   }
   return c;
}

UG_COLOR UG_CheckboxGetForeColor( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;
   UG_COLOR c = C_BLACK;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj != NULL )
   {
      chb = (UG_CHECKBOX*)(obj->data);
      c = chb->fc;
   }
   return c;
}

UG_COLOR UG_CheckboxGetBackColor( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;
   UG_COLOR c = C_BLACK;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj != NULL )
   {
      chb = (UG_CHECKBOX*)(obj->data);
      c = chb->bc;
   }
   return c;
}

UG_COLOR UG_CheckboxGetAlternateForeColor( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;
   UG_COLOR c = C_BLACK;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj != NULL )
   {
      chb = (UG_CHECKBOX*)(obj->data);
      c = chb->afc;
   }
   return c;
}

UG_COLOR UG_CheckboxGetAlternateBackColor( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;
   UG_COLOR c = C_BLACK;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj != NULL )
   {
      chb = (UG_CHECKBOX*)(obj->data);
      c = chb->abc;
   }
   return c;
}

char* UG_CheckboxGetText( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;
   char* str = NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj != NULL )
   {
      chb = (UG_CHECKBOX*)(obj->data);
      str = chb->str;
   }
   return str;
}

UG_FONT* UG_CheckboxGetFont( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;
   UG_FONT* font = NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj != NULL )
   {
      chb = (UG_CHECKBOX*)(obj->data);
      font = (UG_FONT*)chb->font;
   }
   return font;
}

UG_U8 UG_CheckboxGetStyle( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;
   UG_U8 style = 0;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj != NULL )
   {
      chb = (UG_CHECKBOX*)(obj->data);
      style = chb->style;
   }
   return style;
}

UG_S8 UG_CheckboxGetHSpace( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;
   UG_S8 hs = 0;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj != NULL )
   {
      chb = (UG_CHECKBOX*)(obj->data);
      hs = chb->h_space;
   }
   return hs;
}

UG_S8 UG_CheckboxGetVSpace( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;
   UG_S8 vs = 0;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj != NULL )
   {
      chb = (UG_CHECKBOX*)(obj->data);
      vs = chb->v_space;
   }
   return vs;
}

UG_U8 UG_CheckboxGetAlignment( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_CHECKBOX* chb=NULL;
   UG_U8 align = 0;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_CHECKBOX, id );
   if ( obj != NULL )
   {
      chb = (UG_CHECKBOX*)(obj->data);
      align = chb->align;
   }
   return align;
}

/* ======================================================== */
static void _UG_CheckboxUpdate(UG_WINDOW* wnd, UG_OBJECT* obj)
{
   UG_CHECKBOX* chb;
   UG_AREA a;
   UG_TEXT txt;
   UG_U8 d;
   UG_U8 d2;
   UG_COLOR c;

   /* Get object-specific data */
   chb = (UG_CHECKBOX*)(obj->data);

   #ifdef UGUI_USE_TOUCH
   /* -------------------------------------------------- */
   /* Object touch section                               */
   /* -------------------------------------------------- */
   if ( (obj->touch_state & OBJ_TOUCH_STATE_CHANGED) )
   {
      /* Is the Checkbox pressed down? */
      if ( obj->touch_state & OBJ_TOUCH_STATE_PRESSED_ON_OBJECT )
      {
         chb->state |= CHB_STATE_PRESSED;
         obj->state |= OBJ_STATE_UPDATE;
         obj->event = OBJ_EVENT_PRESSED;
      }
      /* Can we release the Checkbox? */
      else if ( chb->state & CHB_STATE_PRESSED )
      {
         chb->state &= ~CHB_STATE_PRESSED;
         obj->state |= OBJ_STATE_UPDATE;
         obj->event = OBJ_EVENT_RELEASED;
          
         chb->checked = !chb->checked; 
      }
      obj->touch_state &= ~OBJ_TOUCH_STATE_CHANGED;
   }
   #endif

   /* -------------------------------------------------- */
   /* Object update section                              */
   /* -------------------------------------------------- */
   if ( obj->state & OBJ_STATE_UPDATE )
   {
      UG_WindowGetArea(wnd,&a);
      obj->a_abs.xs = obj->a_rel.xs + a.xs;
      obj->a_abs.ys = obj->a_rel.ys + a.ys;
      obj->a_abs.xe = obj->a_rel.xe + a.xs;
      obj->a_abs.ye = obj->a_rel.ye + a.ys;
      if ( obj->a_abs.ye > wnd->ye ) return;
      if ( obj->a_abs.xe > wnd->xe ) return;
       
      if ( obj->state & OBJ_STATE_VISIBLE )
      {
         /* 3D or 2D style? */
         d  = ( chb->style & CHB_STYLE_3D )? 3:1;
         d2 = (UG_GetFontWidth(chb->font) < UG_GetFontHeight(chb->font)) ? UG_GetFontHeight(chb->font) : UG_GetFontWidth(chb->font);
          
         /* Full redraw necessary? */
         if ( (obj->state & OBJ_STATE_REDRAW) || (chb->state & CHB_STATE_ALWAYS_REDRAW) )
         {
#ifdef UGUI_USE_PRERENDER_EVENT
            _UG_SendObjectPrerenderEvent(wnd, obj);
#endif
            txt.bc = chb->bc;
            txt.fc = chb->fc;

            if( chb->state & CHB_STATE_PRESSED )
            {
               /* "toggle" style? */
               if( chb->style & CHB_STYLE_TOGGLE_COLORS )
               {
                  /* Swap colors */
                  txt.bc = chb->fc;
                  txt.fc = chb->bc;
               }
               /* Use alternate colors? */
               else if ( chb->style & CHB_STYLE_USE_ALTERNATE_COLORS )
               {
                  txt.bc = chb->abc;
                  txt.fc = chb->afc;
               }
            }
            if ( !(chb->style & CHB_STYLE_NO_FILL) )
               UG_FillFrame(obj->a_abs.xs+d, obj->a_abs.ys+d, obj->a_abs.xe-d, obj->a_abs.ye-d, txt.bc);

            /* Draw Checkbox text */
            txt.a.xs = obj->a_abs.xs + d2 + 3*d;
            txt.a.ys = obj->a_abs.ys + d;
            txt.a.xe = obj->a_abs.xe;
            txt.a.ye = obj->a_abs.ye;
            txt.align = chb->align;
            txt.font = chb->font;
            txt.h_space = 2;
            txt.v_space = 2;
            txt.str = chb->str;
            _UG_PutText( &txt );
            obj->state &= ~OBJ_STATE_REDRAW;
#ifdef UGUI_USE_POSTRENDER_EVENT
            _UG_SendObjectPostrenderEvent(wnd, obj);
#endif
         }

         /* Draw Checkbox X */
         c = chb->checked ? chb->fc : chb->bc;
         UG_DrawLine(obj->a_abs.xs+d+1,  obj->a_abs.ys+d,   obj->a_abs.xs+d2+d-1, obj->a_abs.ys+d2+d-2, c);
         UG_DrawLine(obj->a_abs.xs+d,    obj->a_abs.ys+d,   obj->a_abs.xs+d2+d-1, obj->a_abs.ys+d2+d-1, c);
         UG_DrawLine(obj->a_abs.xs+d,    obj->a_abs.ys+d+1, obj->a_abs.xs+d2+d-2, obj->a_abs.ys+d2+d-1, c);

         UG_DrawLine(obj->a_abs.xs+d2+d-1,  obj->a_abs.ys+d+1, obj->a_abs.xs+d+1, obj->a_abs.ys+d2+d-1, c);
         UG_DrawLine(obj->a_abs.xs+d2+d-1,  obj->a_abs.ys+d,   obj->a_abs.xs+d,   obj->a_abs.ys+d2+d-1, c);
         UG_DrawLine(obj->a_abs.xs+d2+d-2,  obj->a_abs.ys+d,   obj->a_abs.xs+d,   obj->a_abs.ys+d2+d-2, c);

         /* Draw Checkbox frame */
         if ( !(chb->style & CHB_STYLE_NO_BORDERS) )
         {
             if ( chb->style & CHB_STYLE_3D )
             {  /* 3D */
                _UG_DrawObjectFrame(obj->a_abs.xs,obj->a_abs.ys,obj->a_abs.xs+d2+2*d-1,obj->a_abs.ys+d2+2*d-1, (chb->state&CHB_STATE_PRESSED)?(UG_COLOR*)pal_checkbox_pressed:(UG_COLOR*)pal_checkbox_released);
             }
             else
             {  /* 2D */
                 UG_DrawFrame(obj->a_abs.xs,obj->a_abs.ys,obj->a_abs.xs+d2+2*d-1,obj->a_abs.ys+d2+2*d-1,(chb->state&CHB_STATE_PRESSED)?chb->abc:chb->afc);
             }
         }
      }
      else
      {
          if ( !(chb->style & CHB_STYLE_NO_FILL) )
            UG_FillFrame(obj->a_abs.xs, obj->a_abs.ys, obj->a_abs.xe, obj->a_abs.ye, wnd->bc);
      }
      obj->state &= ~OBJ_STATE_UPDATE;
   }
}
