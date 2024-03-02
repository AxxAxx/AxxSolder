#include "ugui_image.h"

/* -------------------------------------------------------------------------------- */
/* -- IMAGE FUNCTIONS                                                            -- */
/* -------------------------------------------------------------------------------- */

/* Static functions */
static void _UG_ImageUpdate(UG_WINDOW* wnd, UG_OBJECT* obj);

UG_RESULT UG_ImageCreate( UG_WINDOW* wnd, UG_IMAGE* img, UG_U8 id, UG_S16 xs, UG_S16 ys, UG_S16 xe, UG_S16 ye )
{
   UG_OBJECT* obj;

   obj = _UG_GetFreeObject( wnd );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   /* Initialize object-specific parameters */
   img->img = NULL;
   img->type = IMG_TYPE_BMP;

   /* Initialize standard object parameters */
   obj->update = _UG_ImageUpdate;
   #ifdef UGUI_USE_TOUCH
   obj->touch_state = OBJ_TOUCH_STATE_INIT;
   #endif
   obj->type = OBJ_TYPE_IMAGE;
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
   obj->data = (void*)img;

   /* Update function: Do your thing! */
   obj->state &= ~OBJ_STATE_FREE;

   return UG_RESULT_OK;
}

UG_RESULT UG_ImageDelete( UG_WINDOW* wnd, UG_U8 id )
{
   return _UG_DeleteObject( wnd, OBJ_TYPE_IMAGE, id );
}

UG_RESULT UG_ImageShow( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_IMAGE, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   obj->state |= OBJ_STATE_VISIBLE;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_ImageHide( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_IMAGE, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   obj->state &= ~OBJ_STATE_VISIBLE;
   obj->state |= OBJ_STATE_UPDATE;

   return UG_RESULT_OK;
}

UG_RESULT UG_ImageSetBMP( UG_WINDOW* wnd, UG_U8 id, const UG_BMP* bmp )
{
   UG_OBJECT* obj=NULL;
   UG_IMAGE* img=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_IMAGE, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   img = (UG_IMAGE*)(obj->data);
   img->img = (void*)bmp;
   img->type = IMG_TYPE_BMP;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

static void _UG_ImageUpdate(UG_WINDOW* wnd, UG_OBJECT* obj)
{
   UG_IMAGE* img;
   UG_AREA a;

   /* Get object-specific data */
   img = (UG_IMAGE*)(obj->data);

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
            /* ToDo: more/better image features */
            obj->a_abs.xs = obj->a_rel.xs + a.xs;
            obj->a_abs.ys = obj->a_rel.ys + a.ys;
            obj->a_abs.xe = obj->a_rel.xs + ((UG_BMP*)img->img)->width + a.xs;
            obj->a_abs.ye = obj->a_rel.ys + ((UG_BMP*)img->img)->height + a.ys;
            if ( obj->a_abs.ye > wnd->ye ) return;
            if ( obj->a_abs.xe > wnd->xe ) return;

            /* Draw Image */
            if ( (img->img != NULL) && (img->type & IMG_TYPE_BMP) )
            {
               UG_DrawBMP(obj->a_abs.xs,obj->a_abs.ys,(UG_BMP*)img->img);
            }

            obj->state &= ~OBJ_STATE_REDRAW;
         }
      }
      else
      {
         UG_FillFrame(obj->a_abs.xs, obj->a_abs.ys, obj->a_abs.xe, obj->a_abs.ye, wnd->bc);
      }
      obj->state &= ~OBJ_STATE_UPDATE;
   }
}
