#include "ugui_textbox.h"

/* -------------------------------------------------------------------------------- */
/* -- Textbox FUNCTIONS                                                          -- */
/* -------------------------------------------------------------------------------- */

/* Static functions */
static void _UG_TextboxUpdate(UG_WINDOW* wnd, UG_OBJECT* obj);

UG_RESULT UG_TextboxCreate( UG_WINDOW* wnd, UG_TEXTBOX* txb, UG_U8 id, UG_S16 xs, UG_S16 ys, UG_S16 xe, UG_S16 ye )
{
   UG_OBJECT* obj;

   obj = _UG_GetFreeObject( wnd );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   /* Initialize object-specific parameters */
   txb->str = NULL;
   txb->font = UG_GetGUI() != NULL ? (UG_GetGUI()->font) : NULL;
   txb->style = 0; /* reserved */
   txb->fc = wnd->fc;
   txb->bc = wnd->bc;
   txb->align = ALIGN_CENTER;
   txb->h_space = 0;
   txb->v_space = 0;

   /* Initialize standard object parameters */
   obj->update = _UG_TextboxUpdate;
   #ifdef UGUI_USE_TOUCH
   obj->touch_state = OBJ_TOUCH_STATE_INIT;
   #endif
   obj->type = OBJ_TYPE_TEXTBOX;
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
   obj->data = (void*)txb;

   /* Update function: Do your thing! */
   obj->state &= ~OBJ_STATE_FREE;

   return UG_RESULT_OK;
}

UG_RESULT UG_TextboxDelete( UG_WINDOW* wnd, UG_U8 id )
{
   return _UG_DeleteObject( wnd, OBJ_TYPE_TEXTBOX, id );
}

UG_RESULT UG_TextboxShow( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_TEXTBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   obj->state |= OBJ_STATE_VISIBLE;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_TextboxHide( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_TEXTBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   obj->state &= ~OBJ_STATE_VISIBLE;
   obj->state |= OBJ_STATE_UPDATE;

   return UG_RESULT_OK;
}

UG_RESULT UG_TextboxSetForeColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR fc )
{
   UG_OBJECT* obj=NULL;
   UG_TEXTBOX* txb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_TEXTBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   txb = (UG_TEXTBOX*)(obj->data);
   txb->fc = fc;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_TextboxSetBackColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR bc )
{
   UG_OBJECT* obj=NULL;
   UG_TEXTBOX* txb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_TEXTBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   txb = (UG_TEXTBOX*)(obj->data);
   txb->bc = bc;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_TextboxSetText( UG_WINDOW* wnd, UG_U8 id, char* str )
{
   UG_OBJECT* obj=NULL;
   UG_TEXTBOX* txb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_TEXTBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   txb = (UG_TEXTBOX*)(obj->data);
   txb->str = str;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_TextboxSetFont( UG_WINDOW* wnd, UG_U8 id, UG_FONT* font )
{
   UG_OBJECT* obj=NULL;
   UG_TEXTBOX* txb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_TEXTBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   txb = (UG_TEXTBOX*)(obj->data);
   txb->font = font;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_TextboxSetHSpace( UG_WINDOW* wnd, UG_U8 id, UG_S8 hs )
{
   UG_OBJECT* obj=NULL;
   UG_TEXTBOX* txb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_TEXTBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   txb = (UG_TEXTBOX*)(obj->data);
   txb->h_space = hs;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_TextboxSetVSpace( UG_WINDOW* wnd, UG_U8 id, UG_S8 vs )
{
   UG_OBJECT* obj=NULL;
   UG_TEXTBOX* txb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_TEXTBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   txb = (UG_TEXTBOX*)(obj->data);
   txb->v_space = vs;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_TextboxSetAlignment( UG_WINDOW* wnd, UG_U8 id, UG_U8 align )
{
   UG_OBJECT* obj=NULL;
   UG_TEXTBOX* txb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_TEXTBOX, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   txb = (UG_TEXTBOX*)(obj->data);
   txb->align = align;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_COLOR UG_TextboxGetForeColor( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_TEXTBOX* txb=NULL;
   UG_COLOR c = C_BLACK;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_TEXTBOX, id );
   if ( obj != NULL )
   {
      txb = (UG_TEXTBOX*)(obj->data);
      c = txb->fc;
   }
   return c;
}

UG_COLOR UG_TextboxGetBackColor( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_TEXTBOX* txb=NULL;
   UG_COLOR c = C_BLACK;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_TEXTBOX, id );
   if ( obj != NULL )
   {
      txb = (UG_TEXTBOX*)(obj->data);
      c = txb->bc;
   }
   return c;
}

char* UG_TextboxGetText( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_TEXTBOX* txb=NULL;
   char* str = NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_TEXTBOX, id );
   if ( obj != NULL )
   {
      txb = (UG_TEXTBOX*)(obj->data);
      str = txb->str;
   }
   return str;
}

UG_FONT* UG_TextboxGetFont( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_TEXTBOX* txb=NULL;
   UG_FONT* font = NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_TEXTBOX, id );
   if ( obj != NULL )
   {
      txb = (UG_TEXTBOX*)(obj->data);
      font = (UG_FONT*)txb->font;
   }
   return font;
}

UG_S8 UG_TextboxGetHSpace( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_TEXTBOX* txb=NULL;
   UG_S8 hs = 0;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_TEXTBOX, id );
   if ( obj != NULL )
   {
      txb = (UG_TEXTBOX*)(obj->data);
      hs = txb->h_space;
   }
   return hs;
}

UG_S8 UG_TextboxGetVSpace( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_TEXTBOX* txb=NULL;
   UG_S8 vs = 0;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_TEXTBOX, id );
   if ( obj != NULL )
   {
      txb = (UG_TEXTBOX*)(obj->data);
      vs = txb->v_space;
   }
   return vs;
}

UG_U8 UG_TextboxGetAlignment( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_TEXTBOX* txb=NULL;
   UG_U8 align = 0;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_TEXTBOX, id );
   if ( obj != NULL )
   {
      txb = (UG_TEXTBOX*)(obj->data);
      align = txb->align;
   }
   return align;
}

static void _UG_TextboxUpdate(UG_WINDOW* wnd, UG_OBJECT* obj)
{
   UG_TEXTBOX* txb;
   UG_AREA a;
   UG_TEXT txt;

   /* Get object-specific data */
   txb = (UG_TEXTBOX*)(obj->data);

   /* -------------------------------------------------- */
   /* Object update section                              */
   /* -------------------------------------------------- */
   if ( obj->state & OBJ_STATE_UPDATE )
   {
      if ( obj->state & OBJ_STATE_VISIBLE )
      {
         /* Full redraw necessary? */
         if ( obj->state & OBJ_STATE_REDRAW )
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

            txt.bc = txb->bc;
            txt.fc = txb->fc;

            UG_FillFrame(obj->a_abs.xs, obj->a_abs.ys, obj->a_abs.xe, obj->a_abs.ye, txt.bc);

            /* Draw Textbox text */
            txt.a.xs = obj->a_abs.xs;
            txt.a.ys = obj->a_abs.ys;
            txt.a.xe = obj->a_abs.xe;
            txt.a.ye = obj->a_abs.ye;
            txt.align = txb->align;
            txt.font = txb->font;
            txt.h_space = txb->h_space;
            txt.v_space = txb->v_space;
            txt.str = txb->str;
            _UG_PutText( &txt );
            obj->state &= ~OBJ_STATE_REDRAW;
#ifdef UGUI_USE_POSTRENDER_EVENT
            _UG_SendObjectPostrenderEvent(wnd, obj);
#endif
         }
      }
      else
      {
         UG_FillFrame(obj->a_abs.xs, obj->a_abs.ys, obj->a_abs.xe, obj->a_abs.ye, wnd->bc);
      }
      obj->state &= ~OBJ_STATE_UPDATE;
   }
}
