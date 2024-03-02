/* -------------------------------------------------------------------------------- */
/* -- µGUI - Generic GUI module (C)Achim Döbler, 2015                            -- */
/* -------------------------------------------------------------------------------- */
// µGUI is a generic GUI module for embedded systems.
// This is a free software that is open for education, research and commercial
// developments under license policy of following terms.
//
//  Copyright (C) 2015, Achim Döbler, all rights reserved.
//  URL: http://www.embeddedlightning.com/
//
// * The µGUI module is a free software and there is NO WARRANTY.
// * No restriction on use. You can use, modify and redistribute it for
//   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
// * Redistributions of source code must retain the above copyright notice.
//
/* -------------------------------------------------------------------------------- */
#include "ugui.h"

/* Static functions */
static UG_RESULT _UG_WindowDrawTitle( UG_WINDOW* wnd );
static void _UG_WindowUpdate( UG_WINDOW* wnd );
static UG_RESULT _UG_WindowClear( UG_WINDOW* wnd );
static void _UG_FontSelect( UG_FONT *font);
static UG_S16 _UG_PutChar( UG_CHAR chr, UG_S16 x, UG_S16 y, UG_COLOR fc, UG_COLOR bc);
static UG_S16 _UG_GetCharData(UG_CHAR encoding,  const UG_U8 **p);
#ifdef UGUI_USE_UTF8
static UG_U16 _UG_DecodeUTF8(char **str);
#endif

static UG_U16 ptr_8to16(const UG_U8* p){
  UG_U16 d = *p++;
  return ((d<<8) | *p);
}


static const UG_COLOR pal_window[] = {
    C_PAL_WINDOW
};

/* Pointer to the gui */
static UG_GUI* gui;

UG_S16 UG_Init( UG_GUI* g, UG_DEVICE *device )
{
   UG_U8 i;

   g->device = device;
#if defined(UGUI_USE_CONSOLE)
   g->console.x_start = 4;
   g->console.y_start = 4;
   g->console.x_end = g->device->x_dim - g->console.x_start-1;
   g->console.y_end = g->device->y_dim - g->console.x_start-1;
   g->console.x_pos = g->console.x_end;
   g->console.y_pos = g->console.y_end;
#endif
   g->char_h_space = 1;
   g->char_v_space = 1;
   g->font=NULL;
   g->currentFont.bytes_per_char = 0;
   g->currentFont.char_height = 0;
   g->currentFont.char_width = 0;
   g->currentFont.number_of_chars = 0;
   g->currentFont.number_of_offsets = 0;
   g->currentFont.widths = NULL;
   g->currentFont.offsets = NULL;
   g->currentFont.data = NULL;
   g->currentFont.font = NULL;
   g->desktop_color = C_DESKTOP_COLOR;
   g->fore_color = C_WHITE;
   g->back_color = C_BLACK;
   g->next_window = NULL;
   g->active_window = NULL;
   g->last_window = NULL;

   /* Clear drivers */
   for(i=0;i<NUMBER_OF_DRIVERS;i++)
   {
      g->driver[i].driver = NULL;
      g->driver[i].state = 0;
   }

   gui = g;
   return 1;
}

UG_S16 UG_SelectGUI( UG_GUI* g )
{
   gui = g;
   return 1;
}

UG_GUI* UG_GetGUI( void )
{
   return gui;
}

/*
 * Sets the GUI font
 */
void UG_FontSelect( UG_FONT* font )
{
  gui->font = font;
}

void UG_FillScreen( UG_COLOR c )
{
   UG_FillFrame(0,0,gui->device->x_dim-1,gui->device->y_dim-1,c);
}

void UG_FillFrame( UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR c )
{
   UG_S16 n,m;

   if ( x2 < x1 )
     swap(x1,x2);
   if ( y2 < y1 )
     swap(y1,y2);

   /* Is hardware acceleration available? */
   if ( gui->driver[DRIVER_FILL_FRAME].state & DRIVER_ENABLED )
   {
      if( ((UG_RESULT(*)(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR c))gui->driver[DRIVER_FILL_FRAME].driver)(x1,y1,x2,y2,c) == UG_RESULT_OK ) return;
   }

   for( m=y1; m<=y2; m++ )
   {
      for( n=x1; n<=x2; n++ )
      {
         gui->device->pset(n,m,c);
      }
   }
}

void UG_FillRoundFrame( UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_S16 r, UG_COLOR c )
{
   UG_S16  x,y,xd;

   if ( x2 < x1 )
     swap(x1,x2);
   if ( y2 < y1 )
     swap(y1,y2);

   if ( r<=0 ) return;

   xd = 3 - (r << 1);
   x = 0;
   y = r;

   UG_FillFrame(x1 + r, y1, x2 - r, y2, c);

   while ( x <= y )
   {
     if( y > 0 )
     {
        UG_DrawLine(x2 + x - r, y1 - y + r, x2+ x - r, y + y2 - r, c);
        UG_DrawLine(x1 - x + r, y1 - y + r, x1- x + r, y + y2 - r, c);
     }
     if( x > 0 )
     {
        UG_DrawLine(x1 - y + r, y1 - x + r, x1 - y + r, x + y2 - r, c);
        UG_DrawLine(x2 + y - r, y1 - x + r, x2 + y - r, x + y2 - r, c);
     }
     if ( xd < 0 )
     {
        xd += (x << 2) + 6;
     }
     else
     {
        xd += ((x - y) << 2) + 10;
        y--;
     }
     x++;
   }
}

void UG_DrawMesh( UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_U16 spacing, UG_COLOR c )
{
   UG_U16 p;

   if ( x2 < x1 )
     swap(x1,x2);
   if ( y2 < y1 )
     swap(y1,y2);

   for( p=y1; p<y2; p+=spacing )
   {
     UG_DrawLine(x1, p, x2, p, c);
   }
   UG_DrawLine(x1, y2, x2, y2, c);

   for( p=x1; p<x2; p+=spacing )
   {
     UG_DrawLine(p, y1, p, y2, c);
   }
   UG_DrawLine(x2, y1, x2, y2, c);
}

void UG_DrawFrame( UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR c )
{
   UG_DrawLine(x1,y1,x2,y1,c);
   UG_DrawLine(x1,y2,x2,y2,c);
   UG_DrawLine(x1,y1,x1,y2,c);
   UG_DrawLine(x2,y1,x2,y2,c);
}

void UG_DrawRoundFrame( UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_S16 r, UG_COLOR c )
{
   if(r == 0)
   {
      UG_DrawFrame(x1, y1, x2, y2, c);
      return;
   }

   if ( x2 < x1 )
     swap(x1,x2);
   if ( y2 < y1 )
     swap(y1,y2);

   if ( r > x2 ) return;
   if ( r > y2 ) return;

   UG_DrawLine(x1+r, y1, x2-r, y1, c);
   UG_DrawLine(x1+r, y2, x2-r, y2, c);
   UG_DrawLine(x1, y1+r, x1, y2-r, c);
   UG_DrawLine(x2, y1+r, x2, y2-r, c);
   UG_DrawArc(x1+r, y1+r, r, 0x0C, c);
   UG_DrawArc(x2-r, y1+r, r, 0x03, c);
   UG_DrawArc(x1+r, y2-r, r, 0x30, c);
   UG_DrawArc(x2-r, y2-r, r, 0xC0, c);
}

void UG_DrawPixel( UG_S16 x0, UG_S16 y0, UG_COLOR c )
{
   gui->device->pset(x0,y0,c);
}

void UG_DrawCircle( UG_S16 x0, UG_S16 y0, UG_S16 r, UG_COLOR c )
{
   UG_S16 x,y,xd,yd,e;

   if ( x0<0 ) return;
   if ( y0<0 ) return;
   if ( r<=0 ) return;

   xd = 1 - (r << 1);
   yd = 0;
   e = 0;
   x = r;
   y = 0;

   while ( x >= y )
   {
      gui->device->pset(x0 - x, y0 + y, c);
      gui->device->pset(x0 - x, y0 - y, c);
      gui->device->pset(x0 + x, y0 + y, c);
      gui->device->pset(x0 + x, y0 - y, c);
      gui->device->pset(x0 - y, y0 + x, c);
      gui->device->pset(x0 - y, y0 - x, c);
      gui->device->pset(x0 + y, y0 + x, c);
      gui->device->pset(x0 + y, y0 - x, c);

      y++;
      e += yd;
      yd += 2;
      if ( ((e << 1) + xd) > 0 )
      {
         x--;
         e += xd;
         xd += 2;
      }
   }
}

void UG_FillCircle( UG_S16 x0, UG_S16 y0, UG_S16 r, UG_COLOR c )
{
   UG_S16  x,y,xd;

   if ( x0<0 ) return;
   if ( y0<0 ) return;
   if ( r<=0 ) return;

   xd = 3 - (r << 1);
   x = 0;
   y = r;

   while ( x <= y )
   {
     if( y > 0 )
     {
        UG_DrawLine(x0 - x, y0 - y,x0 - x, y0 + y, c);
        UG_DrawLine(x0 + x, y0 - y,x0 + x, y0 + y, c);
     }
     if( x > 0 )
     {
        UG_DrawLine(x0 - y, y0 - x,x0 - y, y0 + x, c);
        UG_DrawLine(x0 + y, y0 - x,x0 + y, y0 + x, c);
     }
     if ( xd < 0 )
     {
        xd += (x << 2) + 6;
     }
     else
     {
        xd += ((x - y) << 2) + 10;
        y--;
     }
     x++;
   }
   UG_DrawCircle(x0, y0, r,c);
}

void UG_DrawArc( UG_S16 x0, UG_S16 y0, UG_S16 r, UG_U8 s, UG_COLOR c )
{
   UG_S16 x,y,xd,yd,e;

   if ( x0<0 ) return;
   if ( y0<0 ) return;
   if ( r<=0 ) return;

   xd = 1 - (r << 1);
   yd = 0;
   e = 0;
   x = r;
   y = 0;

   while ( x >= y )
   {
      // Q1
      if ( s & 0x01 ) gui->device->pset(x0 + x, y0 - y, c);
      if ( s & 0x02 ) gui->device->pset(x0 + y, y0 - x, c);

      // Q2
      if ( s & 0x04 ) gui->device->pset(x0 - y, y0 - x, c);
      if ( s & 0x08 ) gui->device->pset(x0 - x, y0 - y, c);

      // Q3
      if ( s & 0x10 ) gui->device->pset(x0 - x, y0 + y, c);
      if ( s & 0x20 ) gui->device->pset(x0 - y, y0 + x, c);

      // Q4
      if ( s & 0x40 ) gui->device->pset(x0 + y, y0 + x, c);
      if ( s & 0x80 ) gui->device->pset(x0 + x, y0 + y, c);

      y++;
      e += yd;
      yd += 2;
      if ( ((e << 1) + xd) > 0 )
      {
         x--;
         e += xd;
         xd += 2;
      }
   }
}

void UG_DrawLine( UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR c )
{
   UG_S16 n, dx, dy, sgndx, sgndy, dxabs, dyabs, x, y, drawx, drawy;

   /* Is hardware acceleration available? */
   if ( gui->driver[DRIVER_DRAW_LINE].state & DRIVER_ENABLED )
   {
      if( ((UG_RESULT(*)(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR c))gui->driver[DRIVER_DRAW_LINE].driver)(x1,y1,x2,y2,c) == UG_RESULT_OK ) return;
   }

   dx = x2 - x1;
   dy = y2 - y1;
   dxabs = (dx>0)?dx:-dx;
   dyabs = (dy>0)?dy:-dy;
   sgndx = (dx>0)?1:-1;
   sgndy = (dy>0)?1:-1;
   x = dyabs >> 1;
   y = dxabs >> 1;
   drawx = x1;
   drawy = y1;

   gui->device->pset(drawx, drawy,c);

   if( dxabs >= dyabs )
   {
      for( n=0; n<dxabs; n++ )
      {
         y += dyabs;
         if( y >= dxabs )
         {
            y -= dxabs;
            drawy += sgndy;
         }
         drawx += sgndx;
         gui->device->pset(drawx, drawy,c);
      }
   }
   else
   {
      for( n=0; n<dyabs; n++ )
      {
         x += dxabs;
         if( x >= dyabs )
         {
            x -= dyabs;
            drawx += sgndx;
         }
         drawy += sgndy;
         gui->device->pset(drawx, drawy,c);
      }
   }  
}


/* Draw a triangle */
void UG_DrawTriangle( UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_S16 x3, UG_S16 y3, UG_COLOR c ){
  UG_DrawLine(x1, y1, x2, y2, c);
  UG_DrawLine(x2, y2, x3, y3, c);
  UG_DrawLine(x3, y3, x1, y1, c);
}

/* Fill a triangle */
void UG_FillTriangle( UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_S16 x3, UG_S16 y3, UG_COLOR c ){

  UG_S16 a, b, y, last;

  /* Sort coordinates by Y order (y3 >= y2 >= y1) */
  if (y1 > y2) {
    swap(y1, y2); swap(x1, x2);
  }
  if (y2 > y3) {
    swap(y3, y2); swap(x3, x2);
  }
  if (y1 > y2) {
    swap(y1, y2); swap(x1, x2);
  }

  /* Handle awkward all-on-same-line case as its own thing */
  if (y1 == y3) {
    a = b = x1;
    if (x2 < a) {
      a = x2;
    } else if (x2 > b) {
      b = x2;
    }
    if (x3 < a) {
      a = x3;
    } else if (x3 > b) {
      b = x3;
    }
    UG_DrawLine(a, y1, b + 1, y1, c);
    return;
  }

  UG_S16
  dx01 = x2 - x1,
  dy01 = y2 - y1,
  dx02 = x3 - x1,
  dy02 = y3 - y1,
  dx12 = x3 - x2,
  dy12 = y3 - y2,
  sa   = 0,
  sb   = 0;

  /* For upper part of triangle, find scanline crossings for segments
   * 0-1 and 0-2.  If y2=y3 (flat-bottomed triangle), the scanline y2
   * is included here (and second loop will be skipped, avoiding a /0
   * error there), otherwise scanline y2 is skipped here and handled
   * in the second loop...which also avoids a /0 error here if y1=y2
   * (flat-topped triangle).
   */
  if (y2 == y3) {
    last = y2;   /* Include y2 scanline */
  } else {
    last = y2 - 1; /* Skip it */
  }

  for (y = y1; y <= last; y++) {
    a   = x1 + sa / dy01;
    b   = x1 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
       a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
       b = x1 + (x3 - x1) * (y - y1) / (y3 - y1);
       */
    if (a > b) {
      swap(a, b);
    }
    UG_DrawLine(a, y, b + 1, y, c);
  }

  /* For lower part of triangle, find scanline crossings for segments
   * 0-2 and 1-2.  This loop is skipped if y2=y3.
   */
  sa = dx12 * (y - y2);
  sb = dx02 * (y - y1);
  for (; y <= y3; y++) {
    a   = x2 + sa / dy12;
    b   = x1 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
       a = x2 + (x3 - x2) * (y - y2) / (y3 - y2);
       b = x1 + (x3 - x1) * (y - y1) / (y3 - y1);
       */
    if (a > b) {
      swap(a, b);
    }
    UG_DrawLine(a, y, b + 1, y, c);
  }
}

void UG_PutString( UG_S16 x, UG_S16 y, char* str )
{
   UG_S16 xp,yp,cw;
   UG_CHAR chr;

   xp=x;
   yp=y;

   _UG_FontSelect(gui->font);
   while ( *str != 0 )
   {
      #ifdef UGUI_USE_UTF8
      if(! gui->currentFont.is_old_font){                // Old font charset compatibility
         chr = _UG_DecodeUTF8(&str);
      }
      else{
         chr = *str++;
      }
      #else
      chr = *str++;
      #endif

      if ( chr == '\n' )
      {
         xp = gui->device->x_dim;
         continue;
      }
      cw = _UG_GetCharData(chr,NULL);
      if(cw==-1) continue;
      if ( xp + cw > gui->device->x_dim - 1 )
      {
         xp = x;
         yp +=  gui->currentFont.char_height+gui->char_v_space;
      }

      _UG_PutChar(chr, xp, yp, gui->fore_color, gui->back_color);

      xp += cw + gui->char_h_space;
   }
   if((gui->driver[DRIVER_FILL_AREA].state & DRIVER_ENABLED))
     ((void*(*)(UG_S16, UG_S16, UG_S16, UG_S16))gui->driver[DRIVER_FILL_AREA].driver)(-1,-1,-1,-1);   // -1 to indicate finish
}

void UG_PutChar( UG_CHAR chr, UG_S16 x, UG_S16 y, UG_COLOR fc, UG_COLOR bc )
{
    _UG_FontSelect(gui->font);
    _UG_PutChar(chr,x,y,fc,bc);
    if((gui->driver[DRIVER_FILL_AREA].state & DRIVER_ENABLED))
      ((void*(*)(UG_S16, UG_S16, UG_S16, UG_S16))gui->driver[DRIVER_FILL_AREA].driver)(-1,-1,-1,-1);   // -1 to indicate finish
}

#if defined(UGUI_USE_CONSOLE)
void UG_ConsolePutString( char* str )
{
   UG_CHAR chr;
   UG_S16 cw;

   _UG_FontSelect(gui->font);

   while ( *str != 0 )
   {
      #ifdef UGUI_USE_UTF8
      if(! gui->currentFont.is_old_font){                // Old font charset compatibility
        chr = _UG_DecodeUTF8(&str);
      }
      else{
        chr = *str++;
      }
      #else
      chr = *str++;
      #endif
      if ( chr == '\n' )
      {
         gui->console.x_pos = gui->device->x_dim;
         str++;
         continue;
      }
      
      cw = _UG_GetCharData(chr, NULL);
      if(cw==-1){
        continue;
      }
      gui->console.x_pos += cw+gui->char_h_space;

      if ( gui->console.x_pos+cw > gui->console.x_end )
      {
         gui->console.x_pos = gui->console.x_start;
         gui->console.y_pos +=  gui->currentFont.char_height+gui->char_v_space;
      }
      if ( gui->console.y_pos+ gui->currentFont.char_height > gui->console.y_end )
      {
         gui->console.x_pos = gui->console.x_start;
         gui->console.y_pos = gui->console.y_start;
         UG_FillFrame(gui->console.x_start,gui->console.y_start,gui->console.x_end,gui->console.y_end,gui->console.back_color);
      }

      _UG_PutChar(chr, gui->console.x_pos, gui->console.y_pos, gui->console.fore_color, gui->console.back_color);
   }
   if((gui->driver[DRIVER_FILL_AREA].state & DRIVER_ENABLED))
     ((void*(*)(UG_S16, UG_S16, UG_S16, UG_S16))gui->driver[DRIVER_FILL_AREA].driver)(-1,-1,-1,-1);   // -1 to indicate finish
}

void UG_ConsoleSetArea( UG_S16 xs, UG_S16 ys, UG_S16 xe, UG_S16 ye )
{
   gui->console.x_start = xs;
   gui->console.y_start = ys;
   gui->console.x_end = xe;
   gui->console.y_end = ye;
}

void UG_ConsoleSetForecolor( UG_COLOR c )
{
   gui->console.fore_color = c;
}

void UG_ConsoleSetBackcolor( UG_COLOR c )
{
   gui->console.back_color = c;
}
#endif

void UG_SetForecolor( UG_COLOR c )
{
   gui->fore_color = c;
}

void UG_SetBackcolor( UG_COLOR c )
{
   gui->back_color = c;
}

UG_S16 UG_GetXDim( void )
{
   return gui->device->x_dim;
}

UG_S16 UG_GetYDim( void )
{
   return gui->device->y_dim;
}

void UG_FontSetHSpace( UG_U16 s )
{
   gui->char_h_space = s;
}

void UG_FontSetVSpace( UG_U16 s )
{
   gui->char_v_space = s;
}

void UG_FontSetTransparency( UG_U8 t )
{
  gui->transparent_font=t;
}

UG_U8 UG_FontGetTransparency( void )
{
  return gui->transparent_font;
}
/* -------------------------------------------------------------------------------- */
/* -- INTERNAL FUNCTIONS                                                         -- */
/* -------------------------------------------------------------------------------- */
/*
 * Parses a pointer to a string, and converts it to Unicode.
 * Automatically increasing the pointer address, so the calling function doesn't need to take care of that.
 *
 * Based on https://github.com/olikraus/u8g2/blob/master/csrc/u8x8_8x8.c
 *
 */
 #ifdef UGUI_USE_UTF8
UG_CHAR _UG_DecodeUTF8(char **str) {

  char c=**str;

  if ( c < 0x80 )                 // Fast detection for simple ASCII
  {
    *str = *str+1;
    return c;
  }

  UG_U8 bytes_left=0;
  UG_CHAR encoding=0;

  while(**str)
  {
    c=**str;
    *str = *str+1;
    if ( bytes_left == 0 )
    {
      if ( c < 0xe0 )             // 2 byte sequence
      {
        bytes_left = 1;
        c &= 0x01f;
      }
      else if ( c < 0xf0 )        // 3 byte sequence
      {
        bytes_left = 2;
        c &= 15;
      }
      else if ( c < 0xf8 )        // 4 byte sequence
      {
        bytes_left = 3;
        c &= 7;
      }
      else if ( c < 0xfc )        // 5 byte sequence
      {
        bytes_left = 4;
        c &= 3;
      }
      else                        // 6 byte sequence
      {
        bytes_left = 5;
        c &= 1;
      }
      encoding = c;
    }
    else
    {
      encoding<<=6;
      encoding |= (c & 0x3F);
      if ( --bytes_left == 0 )
        break;
    }
  }
  return encoding;
}
#endif

/*
 *  Load char bitmap address into p, return the font width
 */
UG_S16 _UG_GetCharData(UG_CHAR encoding,  const UG_U8 **p){
  static UG_CHAR last_encoding;
  static UG_S16 last_width;
  static const UG_U8 * last_p;
  static UG_FONT * last_font;
  UG_U16 start=0;
  UG_U16 skip=0;
  UG_U16 t=0;
  UG_U8 range=0;
  UG_U8 found=0;

  if( gui->currentFont.font==last_font && encoding==last_encoding){       // If called with the same arguments, return cached data
    if(p){
      *p=last_p;                                                    // Load char bitmap address
    }
    return last_width;
  }

  if( gui->currentFont.is_old_font){                                      // Compatibility with old fonts charset
    switch ( encoding )
    {
       case 0xF6: encoding = 0x94; break; // ö
       case 0xD6: encoding = 0x99; break; // Ö
       case 0xFC: encoding = 0x81; break; // ü
       case 0xDC: encoding = 0x9A; break; // Ü
       case 0xE4: encoding = 0x84; break; // ä
       case 0xC4: encoding = 0x8E; break; // Ä
       case 0xB5: encoding = 0xE6; break; // µ
       case 0xB0: encoding = 0xF8; break; // °
    }
  }

  for(;t< gui->currentFont.number_of_offsets;t++)                         // Seek through the offsets
  {
    UG_U16 curr_offset = ptr_8to16( gui->currentFont.offsets+(t*2));    // Offsets are 16-bit, splitted in 2 byte values

    if(curr_offset&0x8000)                                          // If the offset has the MSB bit set, it means it's the a range start
    {
      start=curr_offset&0x7FFF;                                     // Store range start
      range=1;                                                      // Set flag
    }
    else if(range)                                                  // If range previously set, this is the range end
    {
      if(encoding>=start && encoding<=curr_offset)            // If the encoding is between the range
      {
        skip += (encoding-start);                             // Calculate the skip value
        found=1;
        break;
      }
      else if(encoding<start)                                 // If the encoding is lower than current range start, the char is not in the font
        break;

      skip += ((curr_offset-start)+1);                        // Encoding not found in the current range, increase skip size and clear range flasg
      range=0;
    }
    else                                                            // Range not set, this is a single char offset
    {
      if(encoding==curr_offset)                                     // If matching the current offset char
      {
        found=1;
        break;
      }
      else if (encoding<curr_offset)                                // If the encoding is lower than current range, the char is not in the font
      {
        break;
      }
      skip++;                                                       // Else, increase skip and keep searching
    }
  }

  if(found)                                                         // If char found
  {
    last_font =  gui->currentFont.font;                                     // Update cached data
    last_encoding = encoding;
    last_p = ( gui->currentFont.data+(skip* gui->currentFont.bytes_per_char));
    if( gui->currentFont.widths){                                                // If width table available
      last_width = *( gui->currentFont.widths+skip);                        // Use width from table
    }
    else{
      last_width =  gui->currentFont.char_width;                            // Else use width from char width
    }


    if(p){
      *p=last_p;                                                    // Load char bitmap address
    }
    return(last_width);                                             // Return char width
  }
  return -1;                                                        // -1 = char not found
}

/*
 * Updates the current font data
 */
void _UG_FontSelect( UG_FONT *font){
  if( gui->currentFont.font==font)
    return;
   gui->currentFont.font = font;                          // Save Font pointer
   gui->currentFont.font_type = 0x7F & *font;             // Byte    0: Font_type
   gui->currentFont.is_old_font = (0x80 & *font++)&&1;    // Byte    0: Bit 7 indicates old or new font type. 1=old font, 0=new font
   gui->currentFont.char_width = *font++;                 // Byte    1: Char width
   gui->currentFont.char_height = *font++;                // Byte    2: Char height
   gui->currentFont.number_of_chars = ptr_8to16(font);    // Bytes 3+4: Number of chars
  font+=2;
   gui->currentFont.number_of_offsets = ptr_8to16(font);  // Bytes 5+6: Number of offsets
  font+=2;
   gui->currentFont.bytes_per_char = ptr_8to16(font);     // Bytes 7+8: Bytes per char
  font+=2;
  if(*font++){                                    // Byte 9: 1=Width table present, 0=not present
     gui->currentFont.widths = font;                      // Save pointer to width table
    font+= gui->currentFont.number_of_chars;              // Increase number of chars
  }
  else{
     gui->currentFont.widths = NULL;                      // No width table
  }
   gui->currentFont.offsets = font;                       // Save pointer to offset table
  font += ( gui->currentFont.number_of_offsets*2);        // Increase pointer by number of offsets*2 (2-byte values)
   gui->currentFont.data = font;                          // Save pointer to bitmap data
}

UG_S16 _UG_PutChar( UG_CHAR chr, UG_S16 x, UG_S16 y, UG_COLOR fc, UG_COLOR bc)
{
   UG_U16 x0=0,y0=0,i,j,k,bn,fpixels=0,bpixels=0;
   UG_S16 c;
   UG_U8 b,trans=gui->transparent_font,driver=(gui->driver[DRIVER_FILL_AREA].state & DRIVER_ENABLED);
   const UG_U8 * data;                              // Pointer to current char bitmap
   UG_COLOR color;
   void(*push_pixels)(UG_SIZE, UG_COLOR) = NULL;

   UG_S16 actual_char_width = _UG_GetCharData(chr, &data);
   if(actual_char_width==-1)
        return -1;                                     // Char not presnt in the font

   bn =  gui->currentFont.char_width;
   if ( !bn ){
     return 0;
   }
   bn >>= 3;
   if (  gui->currentFont.char_width % 8 ) bn++;

   /* Is hardware acceleration available? */
   if (driver)
   {
     push_pixels = ((void*(*)(UG_S16, UG_S16, UG_S16, UG_S16))gui->driver[DRIVER_FILL_AREA].driver)(x,y,x+actual_char_width-1,y+ gui->currentFont.char_height-1);
   }

   if ( gui->currentFont.font_type == FONT_TYPE_1BPP)
   {
     for( j=0;j< gui->currentFont.char_height;j++ )
     {
       c=0;
       for( i=0;i<bn;i++ )
       {
         b = *data++;
         for( k=0;(k<8) && c<actual_char_width; k++ )
         {
           if(b & 0x01 )                    // Foreground pixel detected
           {
             if(driver)
             {                              // Accelerated output
               if(bpixels && !trans)        // Draw accumulated background pixels, only if transparent mode disabled
               {
                 push_pixels(bpixels,bc);   // Drawing accumulated pixels removes a lot of overhead, drawing speed is at least 3x faster
                 bpixels=0;
               }
               if(!fpixels && trans)        // Store first foreground pixel position for transparent drawing
               {
                 x0=x+c;
                 y0=y+j;
               }
               fpixels++;                   // Instead writing every pixel, count consecutive pixels, then send accumulated pixels in a single transaction
             }
             else
             {                              // Not accelerated output
               gui->device->pset(x+c,y+j,fc);
             }
           }
           else                             // Background pixel detected
           {
             if(driver)
             {                              // Accelerated output
               if(fpixels)                  // Draw accumulated foreground pixels
               {
                 if(!trans)                 // In transparent mode, we don't need to address pixel areas, so just keep pushing pixels
                 {
                   push_pixels(fpixels,fc);
                   fpixels=0;
                 }
                 else{                                // In transparent mode, drawing needs to be broken in smaller parts, avoiding the background areas
                   while(fpixels)
                   {
                     UG_U16 width = (x+actual_char_width)-x0;         // Detect available pixels in the current row from current x position
                     if(x0==x || fpixels<width)                       // If pixel draw count is lower than available pixels, or drawing at start of the row, drawn as-is
                     {
                       push_pixels = ((void*(*)(UG_S16, UG_S16, UG_S16, UG_S16))gui->driver[DRIVER_FILL_AREA].driver)(x0,y0,x0+width-1,y0+(fpixels/actual_char_width));
                       push_pixels(fpixels,fc);
                       fpixels=0;
                     }
                     else                                             // If  pixel draw count is higher than available pixels, there's at least second line, drawn this row first
                     {
                       push_pixels = ((void*(*)(UG_S16, UG_S16, UG_S16, UG_S16))gui->driver[DRIVER_FILL_AREA].driver)(x0,y0,x0+width-1,y0);
                       push_pixels(fpixels,fc);
                       fpixels -= width;
                       x0=x;
                       y0++;
                     }
                   }
                 }
               }
               bpixels++;
             }
             else if(!trans)                           // Not accelerated output
             {
               gui->device->pset(x+c,y+j,bc);
             }
           }
           b >>= 1;
           c++;
        }
       }
     }
     if(driver){                                            // After finishing, ensure there're no remaining pixels left, make another pass
       if(bpixels && !trans)
       {
         push_pixels(bpixels,bc);
       }
       else if(fpixels)
       {
         if(!trans)
         {
           push_pixels(fpixels,fc);
         }
         else
         {
           while(fpixels)
           {
             UG_U16 width = (x+actual_char_width)-x0;
             if(x0==x || fpixels<width)
             {
               push_pixels = ((void*(*)(UG_S16, UG_S16, UG_S16, UG_S16))gui->driver[DRIVER_FILL_AREA].driver)(x0,y0,x0+width-1,y0+(fpixels/actual_char_width));
               push_pixels(fpixels,fc);
               fpixels=0;
             }
             else
             {
               push_pixels = ((void*(*)(UG_S16, UG_S16, UG_S16, UG_S16))gui->driver[DRIVER_FILL_AREA].driver)(x0,y0,x0+width-1,y0);
               push_pixels(fpixels,fc);
               fpixels -= width;
               x0=x;
               y0++;
             }
           }
         }
       }
     }
   }
   #if defined(UGUI_USE_COLOR_RGB888) || defined(UGUI_USE_COLOR_RGB565)
   else if ( gui->currentFont.font_type == FONT_TYPE_8BPP)
   {
     for( j=0;j< gui->currentFont.char_height;j++ )
     {
       for( i=0;i<actual_char_width;i++ )
       {
         b = *data++;
         color = ((((fc & 0xFF) * b + (bc & 0xFF) * (256 - b)) >> 8) & 0xFF) |            //Blue component
                 ((((fc & 0xFF00) * b + (bc & 0xFF00) * (256 - b)) >> 8)  & 0xFF00) |     //Green component
                 ((((fc & 0xFF0000) * b + (bc & 0xFF0000) * (256 - b)) >> 8) & 0xFF0000); //Red component
         if(driver)
         {
           push_pixels(1,color);                                                          // Accelerated output
         }
         else
         {
           gui->device->pset(x+i,y+j,color);                                                // Not accelerated output
         }
       }
       data +=  gui->currentFont.char_width - actual_char_width;
     }
   }
   #endif
   return (actual_char_width);
}

#ifdef UGUI_USE_TOUCH
static void _UG_ProcessTouchData( UG_WINDOW* wnd )
{
   UG_S16 xp,yp;
   UG_U16 i,objcnt;
   UG_OBJECT* obj;
   UG_U8 objstate;
   UG_U8 objtouch;
   UG_U8 tchstate;

   xp = gui->touch.xp;
   yp = gui->touch.yp;
   tchstate = gui->touch.state;

   objcnt = wnd->objcnt;
   for(i=0; i<objcnt; i++)
   {
      obj = (UG_OBJECT*)&wnd->objlst[i];
      objstate = obj->state;
      objtouch = obj->touch_state;
      if ( !(objstate & OBJ_STATE_FREE) && (objstate & OBJ_STATE_VALID) && (objstate & OBJ_STATE_VISIBLE) && !(objstate & OBJ_STATE_REDRAW))
      {
         /* Process touch data */
         if ( (tchstate) && xp != -1 )
         {
            if ( !(objtouch & OBJ_TOUCH_STATE_IS_PRESSED) )
            {
               objtouch |= OBJ_TOUCH_STATE_PRESSED_OUTSIDE_OBJECT | OBJ_TOUCH_STATE_CHANGED;
               objtouch &= ~(OBJ_TOUCH_STATE_RELEASED_ON_OBJECT | OBJ_TOUCH_STATE_RELEASED_OUTSIDE_OBJECT);
            }
            objtouch &= ~OBJ_TOUCH_STATE_IS_PRESSED_ON_OBJECT;
            if ( xp >= obj->a_abs.xs )
            {
               if ( xp <= obj->a_abs.xe )
               {
                  if ( yp >= obj->a_abs.ys )
                  {
                     if ( yp <= obj->a_abs.ye )
                     {
                        objtouch |= OBJ_TOUCH_STATE_IS_PRESSED_ON_OBJECT;
                        if ( !(objtouch & OBJ_TOUCH_STATE_IS_PRESSED) )
                        {
                           objtouch &= ~OBJ_TOUCH_STATE_PRESSED_OUTSIDE_OBJECT;
                           objtouch |= OBJ_TOUCH_STATE_PRESSED_ON_OBJECT;
                        }
                     }
                  }
               }
            }
            objtouch |= OBJ_TOUCH_STATE_IS_PRESSED;
         }
         else if ( objtouch & OBJ_TOUCH_STATE_IS_PRESSED )
         {
            if ( objtouch & OBJ_TOUCH_STATE_IS_PRESSED_ON_OBJECT )
            {
               objtouch |= OBJ_TOUCH_STATE_RELEASED_ON_OBJECT;
            }
            else
            {
               objtouch |= OBJ_TOUCH_STATE_RELEASED_OUTSIDE_OBJECT;
            }
            if ( objtouch & OBJ_TOUCH_STATE_IS_PRESSED )
            {
               objtouch |= OBJ_TOUCH_STATE_CHANGED;
            }
            objtouch &= ~(OBJ_TOUCH_STATE_PRESSED_OUTSIDE_OBJECT | OBJ_TOUCH_STATE_PRESSED_ON_OBJECT | OBJ_TOUCH_STATE_IS_PRESSED);
         }
      }
      obj->touch_state = objtouch;
   }
}
#endif

static void _UG_UpdateObjects( UG_WINDOW* wnd )
{
   UG_U16 i,objcnt;
   UG_OBJECT* obj;
   UG_U8 objstate;
   #ifdef UGUI_USE_TOUCH
   UG_U8 objtouch;
   #endif

   /* Check each object, if it needs to be updated? */
   objcnt = wnd->objcnt;
   for(i=0; i<objcnt; i++)
   {
      obj = (UG_OBJECT*)&wnd->objlst[i];
      objstate = obj->state;
      #ifdef UGUI_USE_TOUCH
      objtouch = obj->touch_state;
      #endif
      if ( !(objstate & OBJ_STATE_FREE) && (objstate & OBJ_STATE_VALID) )
      {
         if ( objstate & OBJ_STATE_UPDATE )
         {
            obj->update(wnd,obj);
         }
         #ifdef UGUI_USE_TOUCH
         if ( (objstate & OBJ_STATE_VISIBLE) && (objstate & OBJ_STATE_TOUCH_ENABLE) )
         {
            if ( (objtouch & (OBJ_TOUCH_STATE_CHANGED | OBJ_TOUCH_STATE_IS_PRESSED)) )
            {
               obj->update(wnd,obj);
            }
         }
         #endif
      }
   }
}

static void _UG_HandleEvents( UG_WINDOW* wnd )
{
   UG_U16 i,objcnt;
   UG_OBJECT* obj;
   UG_U8 objstate;
   static UG_MESSAGE msg;
   msg.src = NULL;

   /* Handle window-related events */
   //ToDo

   /* Handle object-related events */
   msg.type = MSG_TYPE_OBJECT;
   objcnt = wnd->objcnt;
   for(i=0; i<objcnt; i++)
   {
      obj = (UG_OBJECT*)&wnd->objlst[i];
      objstate = obj->state;
      if ( !(objstate & OBJ_STATE_FREE) && (objstate & OBJ_STATE_VALID) )
      {
         if ( obj->event != OBJ_EVENT_NONE )
         {
            msg.src = obj;
            msg.id = obj->type;
            msg.sub_id = obj->id;
            msg.event = obj->event;

            wnd->cb( &msg );

            obj->event = OBJ_EVENT_NONE;
         }
      }
   }
}

/* -------------------------------------------------------------------------------- */
/* -- INTERNAL API FUNCTIONS                                                         -- */
/* -------------------------------------------------------------------------------- */

void _UG_PutText(UG_TEXT* txt)
{
   if(!txt->font || !txt->str){
     return;
   }

   UG_S16 ye=txt->a.ye;
   UG_S16 ys=txt->a.ys;
   UG_S16 char_height=UG_GetFontHeight(txt->font);

   if ( (ye - ys) < char_height ){
     return;
   }

   UG_U16 sl,rc,wl;
   UG_S16 xp,yp;
   UG_S16 xs=txt->a.xs;
   UG_S16 xe=txt->a.xe;
   UG_U8  align=txt->align;
   UG_S16 char_h_space=txt->h_space;
   UG_S16 char_v_space=txt->v_space;
   UG_S16 w;
   UG_CHAR chr;
   char* str = txt->str;
   char* c = str;

   _UG_FontSelect(txt->font);

   rc=1;
   c=str;

   while (1)
   {
     #ifdef UGUI_USE_UTF8
     if(! gui->currentFont.is_old_font){                // Old font charset compatibility
       chr = _UG_DecodeUTF8(&c);
     }
     else{
       chr = *c++;
     }
     #else
     chr = *c++;
     #endif
     if(!chr) break;
     if ( chr == '\n' ) rc++;
   }

   yp = 0;
   if ( align & (ALIGN_V_CENTER | ALIGN_V_BOTTOM) )
   {
      yp = ye - ys + 1;
      yp -= char_height*rc;
      yp -= char_v_space*(rc-1);
      if ( yp < 0 ){
        return;
      }
   }
   if ( align & ALIGN_V_CENTER ) yp >>= 1;
   yp += ys;

   while( 1 )
   {
      sl=0;
      c=str;
      wl = 0;
      while(1)
      {
        #ifdef UGUI_USE_UTF8
        if(! gui->currentFont.is_old_font){                // Old font charset compatibility
          chr = _UG_DecodeUTF8(&c);
        }
        else{
          chr = *c++;
        }
        #else
        chr = *c++;
        #endif
        if( chr == 0 || chr == '\n'){
          break;
        }
         w = _UG_GetCharData(chr, NULL);
         if (w == -1){continue;}
         sl++;
         wl += w + char_h_space;
      }
      wl -= char_h_space;

      xp = xe - xs + 1;
      xp -= wl;
      if ( xp < 0 ) break;

      if ( align & ALIGN_H_LEFT ) xp = 0;
      else if ( align & ALIGN_H_CENTER ) xp >>= 1;
      xp += xs;


      while(1){
         #ifdef UGUI_USE_UTF8
         if(! gui->currentFont.is_old_font){                // Old font charset compatibility
           chr = _UG_DecodeUTF8(&str);
         }
         else{
           chr = *str++;
         }
         #else
         chr = *str++;
         #endif
         if ( chr == 0 ){
           return;
         }
         else if(chr=='\n'){
           break;
         }
         w = _UG_PutChar(chr,xp,yp,txt->fc,txt->bc);
         if(w!=-1)
           xp += w + char_h_space;
      }
      yp += char_height + char_v_space;
   }
}

UG_OBJECT* _UG_SearchObject( UG_WINDOW* wnd, UG_U8 type, UG_U8 id )
{
   UG_U8 i;
   UG_OBJECT* obj=(UG_OBJECT*)wnd->objlst;

   for(i=0;i<wnd->objcnt;i++)
   {
      obj = (UG_OBJECT*)(&wnd->objlst[i]);
      if ( !(obj->state & OBJ_STATE_FREE) && (obj->state & OBJ_STATE_VALID) )
      {
         if ( (obj->type == type) && (obj->id == id) )
         {
            /* Requested object found! */
            return obj;
         }
      }
   }
   return NULL;
}

void _UG_DrawObjectFrame( UG_S16 xs, UG_S16 ys, UG_S16 xe, UG_S16 ye, UG_COLOR* p )
{
   // Frame 0
   UG_DrawLine(xs, ys  , xe-1, ys  , *p++);
   UG_DrawLine(xs, ys+1, xs  , ye-1, *p++);
   UG_DrawLine(xs, ye  , xe  , ye  , *p++);
   UG_DrawLine(xe, ys  , xe  , ye-1, *p++);
   // Frame 1
   UG_DrawLine(xs+1, ys+1, xe-2, ys+1, *p++);
   UG_DrawLine(xs+1, ys+2, xs+1, ye-2, *p++);
   UG_DrawLine(xs+1, ye-1, xe-1, ye-1, *p++);
   UG_DrawLine(xe-1, ys+1, xe-1, ye-2, *p++);
   // Frame 2
   UG_DrawLine(xs+2, ys+2, xe-3, ys+2, *p++);
   UG_DrawLine(xs+2, ys+3, xs+2, ye-3, *p++);
   UG_DrawLine(xs+2, ye-2, xe-2, ye-2, *p++);
   UG_DrawLine(xe-2, ys+2, xe-2, ye-3, *p);
}

UG_OBJECT* _UG_GetFreeObject( UG_WINDOW* wnd )
{
   UG_U8 i;
   UG_OBJECT* obj=(UG_OBJECT*)wnd->objlst;

   for(i=0;i<wnd->objcnt;i++)
   {
      obj = (UG_OBJECT*)(&wnd->objlst[i]);
      if ( (obj->state & OBJ_STATE_FREE) && (obj->state & OBJ_STATE_VALID) )
      {
         /* Free object found! */
         return obj;
      }
   }
   return NULL;
}

UG_RESULT _UG_DeleteObject( UG_WINDOW* wnd, UG_U8 type, UG_U8 id )
{
   UG_OBJECT* obj=NULL;

   obj = _UG_SearchObject( wnd, type, id );

   /* Object found? */
   if ( obj != NULL )
   {
      /* We dont't want to delete a visible or busy object! */
      if ( (obj->state & OBJ_STATE_VISIBLE) || (obj->state & OBJ_STATE_UPDATE) ) return UG_RESULT_FAIL;
      obj->state = OBJ_STATE_INIT;
      obj->data = NULL;
      obj->event = 0;
      obj->id = 0;
      #ifdef UGUI_USE_TOUCH
      obj->touch_state = 0;
      #endif
      obj->type = 0;
      obj->update = NULL;
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

#ifdef UGUI_USE_PRERENDER_EVENT
void _UG_SendObjectPrerenderEvent( UG_WINDOW *wnd, UG_OBJECT *obj )
{
   UG_MESSAGE msg;
   msg.event = OBJ_EVENT_PRERENDER;
   msg.type = MSG_TYPE_OBJECT;
   msg.id = obj->type;
   msg.sub_id = obj->id;
   msg.src = obj;

   wnd->cb(&msg);
}
#endif

#ifdef UGUI_USE_POSTRENDER_EVENT
void _UG_SendObjectPostrenderEvent( UG_WINDOW *wnd, UG_OBJECT *obj )
{
   UG_MESSAGE msg;
   msg.event = OBJ_EVENT_POSTRENDER;
   msg.type = MSG_TYPE_OBJECT;
   msg.id = obj->type;
   msg.sub_id = obj->id;
   msg.src = obj;

   wnd->cb(&msg);
}
#endif

UG_U32 _UG_ConvertRGB565ToRGB888(UG_U16 c)
{
   UG_U32 r,g,b;

   r = (c&0xF800)<<8;
   r += (r+7)>>5;

   g = (c&0x7E0)<<5;
   g += (g+3)>>6;

   b = (c&0x1F)<<3;
   b += (b+7)>>5;

   return (r | g | b);
}

/* -------------------------------------------------------------------------------- */
/* -- DRIVER FUNCTIONS                                                           -- */
/* -------------------------------------------------------------------------------- */
void UG_DriverRegister( UG_U8 type, void* driver )
{
   if ( type >= NUMBER_OF_DRIVERS ) return;

   gui->driver[type].driver = driver;
   gui->driver[type].state = DRIVER_REGISTERED | DRIVER_ENABLED;
}

void UG_DriverEnable( UG_U8 type )
{
   if ( type >= NUMBER_OF_DRIVERS ) return;
   if ( gui->driver[type].state & DRIVER_REGISTERED )
   {
      gui->driver[type].state |= DRIVER_ENABLED;
   }
}

void UG_DriverDisable( UG_U8 type )
{
   if ( type >= NUMBER_OF_DRIVERS ) return;
   if ( gui->driver[type].state & DRIVER_REGISTERED )
   {
      gui->driver[type].state &= ~DRIVER_ENABLED;
   }
}

/* -------------------------------------------------------------------------------- */
/* -- MISCELLANEOUS FUNCTIONS                                                    -- */
/* -------------------------------------------------------------------------------- */
void UG_Update( void )
{
   UG_WINDOW* wnd;

   /* Is somebody waiting for this update? */
   if ( gui->state & UG_STATUS_WAIT_FOR_UPDATE ) gui->state &= ~UG_STATUS_WAIT_FOR_UPDATE;

   /* Keep track of the windows */
   if ( gui->next_window != gui->active_window )
   {
      if ( gui->next_window != NULL )
      {
         gui->last_window = gui->active_window;
         gui->active_window = gui->next_window;

         /* Do we need to draw an inactive title? */
         if ((gui->last_window != NULL) && (gui->last_window->style & WND_STYLE_SHOW_TITLE) && (gui->last_window->state & WND_STATE_VISIBLE) )
         {
            /* Do both windows differ in size */
            if ( (gui->last_window->xs != gui->active_window->xs) || (gui->last_window->xe != gui->active_window->xe) || (gui->last_window->ys != gui->active_window->ys) || (gui->last_window->ye != gui->active_window->ye) )
            {
               /* Redraw title of the last window */
               _UG_WindowDrawTitle( gui->last_window );
            }
         }
         gui->active_window->state &= ~WND_STATE_REDRAW_TITLE;
         gui->active_window->state |= WND_STATE_UPDATE | WND_STATE_VISIBLE;
      }
   }

   /* Is there an active window */
   if ( gui->active_window != NULL )
   {
      wnd = gui->active_window;

      /* Does the window need to be updated? */
      if ( wnd->state & WND_STATE_UPDATE )
      {
         /* Do it! */
         _UG_WindowUpdate( wnd );
      }

      /* Is the window visible? */
      if ( wnd->state & WND_STATE_VISIBLE )
      {
         #ifdef UGUI_USE_TOUCH
         _UG_ProcessTouchData( wnd );
         #endif
         _UG_UpdateObjects( wnd );
         _UG_HandleEvents( wnd );
      }
   }
   if(gui->device->flush){
     gui->device->flush();
   }
}

void UG_WaitForUpdate( void )
{
   gui->state |= UG_STATUS_WAIT_FOR_UPDATE;
   #ifdef UGUI_USE_MULTITASKING    
   while ( (volatile UG_U8)gui->state & UG_STATUS_WAIT_FOR_UPDATE ){};
   #endif    
   #ifndef UGUI_USE_MULTITASKING    
   while ( (UG_U8)gui->state & UG_STATUS_WAIT_FOR_UPDATE ){};
   #endif    
}

void UG_DrawBMP( UG_S16 xp, UG_S16 yp, UG_BMP* bmp )
{
   UG_COLOR c;
   UG_S16 x,y;

   if ( bmp->p == NULL ) return;

   if ( bmp->bpp == BMP_BPP_1){
     UG_U8 xx,yy,b;
     const UG_U8* p = (UG_U8*)bmp->p;         // This is untested !
      for(y=0;y<bmp->height;y++)
      {
         for(x=0;x<bmp->width;x++)
         {
            yy = y / 8 ;
            xx = y % 8;
            b = p[x + yy * bmp->width];
            xx = 1 << xx;
            xx = xx & b;
            if(xx) c = gui->fore_color;
            else c = gui->back_color;
            UG_DrawPixel( x + xp , y + yp , c );
         }
      }
     return;
   }
   #if defined UGUI_USE_COLOR_RGB888 || defined UGUI_USE_COLOR_RGB565
   else if ( bmp->bpp == BMP_BPP_16){

     /* Is hardware acceleration available? */

      if ( gui->driver[DRIVER_DRAW_BMP].state & DRIVER_ENABLED)
      {
        ((void(*)(UG_S16, UG_S16, UG_BMP* bmp))gui->driver[DRIVER_DRAW_BMP].driver)(xp,yp, bmp);
        return;
      }
      else if ( gui->driver[DRIVER_FILL_AREA].state & DRIVER_ENABLED)
      {
         void(*push_pixels)(UG_U16, UG_COLOR) = ((void*(*)(UG_S16, UG_S16, UG_S16, UG_S16))gui->driver[DRIVER_FILL_AREA].driver)(xp,yp,xp+bmp->width-1,yp+bmp->height-1);
         UG_U16 *p = (UG_U16*)bmp->p;
         for(y=0;y<bmp->height;y++)
         {
           for(x=0;x<bmp->width;x++)
           {
             #ifdef UGUI_USE_COLOR_RGB888
             push_pixels(1, _UG_ConvertRGB565ToRGB888(*p++)); /* Convert RGB565 to RGB888 */
             #elif defined UGUI_USE_COLOR_RGB565
             push_pixels(1, *p++);
             #endif
           }
           yp++;
         }
         return;
      }

     UG_U16 *p = (UG_U16*)bmp->p;
     for(y=0;y<bmp->height;y++)
     {
        for(x=0;x<bmp->width;x++)
        {
          #ifdef UGUI_USE_COLOR_RGB888
           UG_DrawPixel( xp+x , yp , _UG_ConvertRGB565ToRGB888(*p++) ); /* Convert RGB565 to RGB888 */
          #elif defined UGUI_USE_COLOR_RGB565
           UG_DrawPixel( xp+x , yp , *p++ );
          #endif
        }
        yp++;
     }
   }
   #endif
}

#ifdef UGUI_USE_TOUCH
void UG_TouchUpdate( UG_S16 xp, UG_S16 yp, UG_U8 state )
{
   gui->touch.xp = xp;
   gui->touch.yp = yp;
   gui->touch.state = state;
}
#endif

/* -------------------------------------------------------------------------------- */
/* -- WINDOW FUNCTIONS                                                           -- */
/* -------------------------------------------------------------------------------- */
UG_RESULT UG_WindowCreate( UG_WINDOW* wnd, UG_OBJECT* objlst, UG_U8 objcnt, void (*cb)( UG_MESSAGE* ) )
{
   UG_U8 i;
   UG_OBJECT* obj=NULL;

   if ( (wnd == NULL) || (objlst == NULL) || (objcnt == 0) ) return UG_RESULT_FAIL;

   /* Initialize all objects of the window */
   for(i=0; i<objcnt; i++)
   {
      obj = (UG_OBJECT*)&objlst[i];
      obj->state = OBJ_STATE_INIT;
      obj->data = NULL;
   }

   /* Initialize window */
   wnd->objcnt = objcnt;
   wnd->objlst = objlst;
   wnd->state = WND_STATE_VALID;
   wnd->fc = C_FORE_COLOR;
   wnd->bc = C_BACK_COLOR;
   wnd->xs = 0;
   wnd->ys = 0;
   wnd->xe = UG_GetXDim()-1;
   wnd->ye = UG_GetYDim()-1;
   wnd->cb = cb;
   wnd->style = WND_STYLE_3D | WND_STYLE_SHOW_TITLE;

   /* Initialize window title-bar */
   wnd->title.str = NULL;
   if (gui != NULL) wnd->title.font = gui->font;
   else wnd->title.font = NULL;
   wnd->title.h_space = 2;
   wnd->title.v_space = 2;
   wnd->title.align = ALIGN_CENTER_LEFT;
   wnd->title.fc = C_TITLE_FORE_COLOR;
   wnd->title.bc = C_TITLE_BACK_COLOR;
   wnd->title.ifc = C_INACTIVE_TITLE_FORE_COLOR;
   wnd->title.ibc = C_INACTIVE_TITLE_BACK_COLOR;
   wnd->title.height = 15;

   return UG_RESULT_OK;
}

UG_RESULT UG_WindowDelete( UG_WINDOW* wnd )
{
   if ( wnd == gui->active_window ) return UG_RESULT_FAIL;

   /* Only delete valid windows */
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      wnd->state = 0;
      wnd->cb = NULL;
      wnd->objcnt = 0;
      wnd->objlst = NULL;
      wnd->xs = 0;
      wnd->ys = 0;
      wnd->xe = 0;
      wnd->ye = 0;
      wnd->style = 0;
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_RESULT UG_WindowShow( UG_WINDOW* wnd )
{
   if ( wnd != NULL )
   {
      /* Force an update, even if this is the active window! */
      wnd->state |= WND_STATE_VISIBLE | WND_STATE_UPDATE;
      wnd->state &= ~WND_STATE_REDRAW_TITLE;
      gui->next_window = wnd;
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_RESULT UG_WindowHide( UG_WINDOW* wnd )
{
   if ( wnd != NULL )
   {
      if ( wnd == gui->active_window )
      {
         /* Is there an old window which just lost the focus? */
         if ( (gui->last_window != NULL) && (gui->last_window->state & WND_STATE_VISIBLE) )
         {
            if ( (gui->last_window->xs > wnd->xs) || (gui->last_window->ys > wnd->ys) || (gui->last_window->xe < wnd->xe) || (gui->last_window->ye < wnd->ye) )
            {
               _UG_WindowClear( wnd );
            }
            gui->next_window = gui->last_window;
         }
         else
         {
            gui->active_window->state &= ~WND_STATE_VISIBLE;
            gui->active_window->state |= WND_STATE_UPDATE;
         }
      }
      else
      {
         /* If the old window is visible, clear it! */
         _UG_WindowClear( wnd );
      }
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_RESULT UG_WindowResize( UG_WINDOW* wnd, UG_S16 xs, UG_S16 ys, UG_S16 xe, UG_S16 ye )
{
   UG_S16 pos;
   UG_S16 xmax,ymax;

   xmax = UG_GetXDim()-1;
   ymax = UG_GetYDim()-1;

   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      /* Do some checks... */
      if ( (xs < 0) || (ys < 0) ) return UG_RESULT_FAIL;
      if ( (xe > xmax) || (ye > ymax) ) return UG_RESULT_FAIL;
      pos = xe-xs;
      if ( pos < 10 ) return UG_RESULT_FAIL;
      pos = ye-ys;
      if ( pos < 10 ) return UG_RESULT_FAIL;

      /* ... and if everything is OK move the window! */
      wnd->xs = xs;
      wnd->ys = ys;
      wnd->xe = xe;
      wnd->ye = ye;

      if ( (wnd->state & WND_STATE_VISIBLE) && (gui->active_window == wnd) )
      {
         if ( wnd->ys ) UG_FillFrame(0, 0, xmax,wnd->ys-1,gui->desktop_color);
         pos = wnd->ye+1;
         if ( !(pos > ymax) ) UG_FillFrame(0, pos, xmax,ymax,gui->desktop_color);
         if ( wnd->xs ) UG_FillFrame(0, wnd->ys, wnd->xs-1,wnd->ye,gui->desktop_color);
         pos = wnd->xe+1;
         if ( !(pos > xmax) ) UG_FillFrame(pos, wnd->ys,xmax,wnd->ye,gui->desktop_color);

         wnd->state &= ~WND_STATE_REDRAW_TITLE;
         wnd->state |= WND_STATE_UPDATE;
      }
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_RESULT UG_WindowAlert( UG_WINDOW* wnd )
{
   UG_COLOR c;
   c = UG_WindowGetTitleTextColor( wnd );
   if ( UG_WindowSetTitleTextColor( wnd, UG_WindowGetTitleColor( wnd ) ) == UG_RESULT_FAIL ) return UG_RESULT_FAIL;
   if ( UG_WindowSetTitleColor( wnd, c ) == UG_RESULT_FAIL ) return UG_RESULT_FAIL;
   return UG_RESULT_OK;
}

UG_RESULT UG_WindowSetForeColor( UG_WINDOW* wnd, UG_COLOR fc )
{
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      wnd->fc = fc;
      wnd->state |= WND_STATE_UPDATE;
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_RESULT UG_WindowSetBackColor( UG_WINDOW* wnd, UG_COLOR bc )
{
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      wnd->bc = bc;
      wnd->state |= WND_STATE_UPDATE;
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_RESULT UG_WindowSetTitleTextColor( UG_WINDOW* wnd, UG_COLOR c )
{
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      wnd->title.fc = c;
      wnd->state |= WND_STATE_UPDATE | WND_STATE_REDRAW_TITLE;
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_RESULT UG_WindowSetTitleColor( UG_WINDOW* wnd, UG_COLOR c )
{
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      wnd->title.bc = c;
      wnd->state |= WND_STATE_UPDATE | WND_STATE_REDRAW_TITLE;
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_RESULT UG_WindowSetTitleInactiveTextColor( UG_WINDOW* wnd, UG_COLOR c )
{
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      wnd->title.ifc = c;
      wnd->state |= WND_STATE_UPDATE | WND_STATE_REDRAW_TITLE;
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_RESULT UG_WindowSetTitleInactiveColor( UG_WINDOW* wnd, UG_COLOR c )
{
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      wnd->title.ibc = c;
      wnd->state |= WND_STATE_UPDATE | WND_STATE_REDRAW_TITLE;
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_RESULT UG_WindowSetTitleText( UG_WINDOW* wnd, char* str )
{
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      wnd->title.str = str;
      wnd->state |= WND_STATE_UPDATE | WND_STATE_REDRAW_TITLE;
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_RESULT UG_WindowSetTitleTextFont( UG_WINDOW* wnd, UG_FONT* font )
{
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      wnd->state |= WND_STATE_UPDATE | WND_STATE_REDRAW_TITLE;
      wnd->title.font = font;
      if ( wnd->title.height <= (UG_GetFontHeight(font) + 1) )
      {
         wnd->title.height = UG_GetFontWidth(font) + 2;
         wnd->state &= ~WND_STATE_REDRAW_TITLE;
      }
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_RESULT UG_WindowSetTitleTextHSpace( UG_WINDOW* wnd, UG_S8 hs )
{
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      wnd->title.h_space = hs;
      wnd->state |= WND_STATE_UPDATE | WND_STATE_REDRAW_TITLE;
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_RESULT UG_WindowSetTitleTextVSpace( UG_WINDOW* wnd, UG_S8 vs )
{
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      wnd->title.v_space = vs;
      wnd->state |= WND_STATE_UPDATE | WND_STATE_REDRAW_TITLE;
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_RESULT UG_WindowSetTitleTextAlignment( UG_WINDOW* wnd, UG_U8 align )
{
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      wnd->title.align = align;
      wnd->state |= WND_STATE_UPDATE | WND_STATE_REDRAW_TITLE;
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_RESULT UG_WindowSetTitleHeight( UG_WINDOW* wnd, UG_U8 height )
{
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      wnd->title.height = height;
      wnd->state &= ~WND_STATE_REDRAW_TITLE;
      wnd->state |= WND_STATE_UPDATE;
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_RESULT UG_WindowSetXStart( UG_WINDOW* wnd, UG_S16 xs )
{
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      wnd->xs = xs;
      if ( UG_WindowResize( wnd, wnd->xs, wnd->ys, wnd->xe, wnd->ye) == UG_RESULT_FAIL ) return UG_RESULT_FAIL;
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_RESULT UG_WindowSetYStart( UG_WINDOW* wnd, UG_S16 ys )
{
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      wnd->ys = ys;
      if ( UG_WindowResize( wnd, wnd->xs, wnd->ys, wnd->xe, wnd->ye) == UG_RESULT_FAIL ) return UG_RESULT_FAIL;
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_RESULT UG_WindowSetXEnd( UG_WINDOW* wnd, UG_S16 xe )
{
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      wnd->xe = xe;
      if ( UG_WindowResize( wnd, wnd->xs, wnd->ys, wnd->xe, wnd->ye) == UG_RESULT_FAIL ) return UG_RESULT_FAIL;
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_RESULT UG_WindowSetYEnd( UG_WINDOW* wnd, UG_S16 ye )
{
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      wnd->ye = ye;
      if ( UG_WindowResize( wnd, wnd->xs, wnd->ys, wnd->xe, wnd->ye) == UG_RESULT_FAIL ) return UG_RESULT_FAIL;
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_RESULT UG_WindowSetStyle( UG_WINDOW* wnd, UG_U8 style )
{
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      /* 3D or 2D? */
      if ( style & WND_STYLE_3D )
      {
         wnd->style |= WND_STYLE_3D;
      }
      else
      {
         wnd->style &= ~WND_STYLE_3D;
      }
      /* Show title-bar? */
      if ( style & WND_STYLE_SHOW_TITLE )
      {
         wnd->style |= WND_STYLE_SHOW_TITLE;
      }
      else
      {
         wnd->style &= ~WND_STYLE_SHOW_TITLE;
      }
      wnd->state |= WND_STATE_UPDATE;
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_COLOR UG_WindowGetForeColor( UG_WINDOW* wnd )
{
   UG_COLOR c = C_BLACK;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      c = wnd->fc;
   }
   return c;
}

UG_COLOR UG_WindowGetBackColor( UG_WINDOW* wnd )
{
   UG_COLOR c = C_BLACK;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      c = wnd->bc;
   }
   return c;
}

UG_COLOR UG_WindowGetTitleTextColor( UG_WINDOW* wnd )
{
   UG_COLOR c = C_BLACK;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      c = wnd->title.fc;
   }
   return c;
}

UG_COLOR UG_WindowGetTitleColor( UG_WINDOW* wnd )
{
   UG_COLOR c = C_BLACK;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      c = wnd->title.bc;
   }
   return c;
}

UG_COLOR UG_WindowGetTitleInactiveTextColor( UG_WINDOW* wnd )
{
   UG_COLOR c = C_BLACK;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      c = wnd->title.ifc;
   }
   return c;
}

UG_COLOR UG_WindowGetTitleInactiveColor( UG_WINDOW* wnd )
{
   UG_COLOR c = C_BLACK;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      c = wnd->title.ibc;
   }
   return c;
}

char* UG_WindowGetTitleText( UG_WINDOW* wnd )
{
   char* str = NULL;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      str = wnd->title.str;
   }
   return str;
}

UG_FONT* UG_WindowGetTitleTextFont( UG_WINDOW* wnd )
{
   UG_FONT* f = NULL;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      f = (UG_FONT*)wnd->title.font;
   }
   return f;
}

UG_S8 UG_WindowGetTitleTextHSpace( UG_WINDOW* wnd )
{
   UG_S8 hs = 0;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      hs = wnd->title.h_space;
   }
   return hs;
}

UG_S8 UG_WindowGetTitleTextVSpace( UG_WINDOW* wnd )
{
   UG_S8 vs = 0;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      vs = wnd->title.v_space;
   }
   return vs;
}

UG_U8 UG_WindowGetTitleTextAlignment( UG_WINDOW* wnd )
{
   UG_U8 align = 0;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      align = wnd->title.align;
   }
   return align;
}

UG_U8 UG_WindowGetTitleHeight( UG_WINDOW* wnd )
{
   UG_U8 h = 0;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      h = wnd->title.height;
   }
   return h;
}

UG_S16 UG_WindowGetXStart( UG_WINDOW* wnd )
{
   UG_S16 xs = -1;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      xs = wnd->xs;
   }
   return xs;
}

UG_S16 UG_WindowGetYStart( UG_WINDOW* wnd )
{
   UG_S16 ys = -1;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      ys = wnd->ys;
   }
   return ys;
}

UG_S16 UG_WindowGetXEnd( UG_WINDOW* wnd )
{
   UG_S16 xe = -1;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      xe = wnd->xe;
   }
   return xe;
}

UG_S16 UG_WindowGetYEnd( UG_WINDOW* wnd )
{
   UG_S16 ye = -1;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      ye = wnd->ye;
   }
   return ye;
}

UG_U8 UG_WindowGetStyle( UG_WINDOW* wnd )
{
   UG_U8 style = 0;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      style = wnd->style;
   }
   return style;
}

UG_RESULT UG_WindowGetArea( UG_WINDOW* wnd, UG_AREA* a )
{
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      a->xs = wnd->xs;
      a->ys = wnd->ys;
      a->xe = wnd->xe;
      a->ye = wnd->ye;
      if ( wnd->style & WND_STYLE_3D )
      {
         a->xs+=3;
         a->ys+=3;
         a->xe-=3;
         a->ye-=3;
      }
      if ( wnd->style & WND_STYLE_SHOW_TITLE )
      {
         a->ys+= wnd->title.height+1;
      }
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

UG_S16 UG_WindowGetInnerWidth( UG_WINDOW* wnd )
{
   UG_S16 w = 0;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      w = wnd->xe-wnd->xs;

      /* 3D style? */
      if ( wnd->style & WND_STYLE_3D ) w-=6;

      if ( w < 0 ) w = 0;
   }
   return w;
}

UG_S16 UG_WindowGetOuterWidth( UG_WINDOW* wnd )
{
   UG_S16 w = 0;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      w = wnd->xe-wnd->xs;

      if ( w < 0 ) w = 0;
   }
   return w;
}

UG_S16 UG_WindowGetInnerHeight( UG_WINDOW* wnd )
{
   UG_S16 h = 0;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      h = wnd->ye-wnd->ys;

      /* 3D style? */
      if ( wnd->style & WND_STYLE_3D ) h-=6;

      /* Is the title active */
      if ( wnd->style & WND_STYLE_SHOW_TITLE ) h-=wnd->title.height;

      if ( h < 0 ) h = 0;
   }
   return h;
}

UG_S16 UG_WindowGetOuterHeight( UG_WINDOW* wnd )
{
   UG_S16 h = 0;
   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      h = wnd->ye-wnd->ys;

      if ( h < 0 ) h = 0;
   }
   return h;
}

static UG_RESULT _UG_WindowDrawTitle( UG_WINDOW* wnd )
{
   UG_TEXT txt;
   UG_S16 xs,ys,xe,ye;

   if ( (wnd != NULL) && (wnd->state & WND_STATE_VALID) )
   {
      xs = wnd->xs;
      ys = wnd->ys;
      xe = wnd->xe;
      ye = wnd->ye;

      /* 3D style? */
      if ( wnd->style & WND_STYLE_3D )
      {
         xs+=3;
         ys+=3;
         xe-=3;
         ye-=3;
      }

      /* Is the window active or inactive? */
      if ( wnd == gui->active_window )
      {
         txt.bc = wnd->title.bc;
         txt.fc = wnd->title.fc;
      }
      else
      {
         txt.bc = wnd->title.ibc;
         txt.fc = wnd->title.ifc;
      }

      /* Draw title */
      UG_FillFrame(xs,ys,xe,ys+wnd->title.height-1,txt.bc);

      /* Draw title text */
      txt.str = wnd->title.str;
      txt.font = wnd->title.font;
      txt.a.xs = xs+3;
      txt.a.ys = ys;
      txt.a.xe = xe;
      txt.a.ye = ys+wnd->title.height-1;
      txt.align = wnd->title.align;
      txt.h_space = wnd->title.h_space;
      txt.v_space = wnd->title.v_space;
      _UG_PutText( &txt );

      /* Draw line */
      UG_DrawLine(xs,ys+wnd->title.height,xe,ys+wnd->title.height,pal_window[11]);
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}

static void _UG_WindowUpdate( UG_WINDOW* wnd )
{
   UG_U16 i,objcnt;
   UG_OBJECT* obj;
   UG_S16 xs,ys,xe,ye;

   xs = wnd->xs;
   ys = wnd->ys;
   xe = wnd->xe;
   ye = wnd->ye;

   wnd->state &= ~WND_STATE_UPDATE;
   /* Is the window visible? */
   if ( wnd->state & WND_STATE_VISIBLE )
   {
      /* 3D style? */
      if ( (wnd->style & WND_STYLE_3D) && !(wnd->state & WND_STATE_REDRAW_TITLE) )
      {
         _UG_DrawObjectFrame(xs,ys,xe,ye,(UG_COLOR*)pal_window);
         xs+=3;
         ys+=3;
         xe-=3;
         ye-=3;
      }
      /* Show title bar? */
      if ( wnd->style & WND_STYLE_SHOW_TITLE )
      {
         _UG_WindowDrawTitle( wnd );
         ys += wnd->title.height+1;
         if ( wnd->state & WND_STATE_REDRAW_TITLE )
         {
            wnd->state &= ~WND_STATE_REDRAW_TITLE;
            return;
         }
      }
      /* Draw window area? */
      UG_FillFrame(xs,ys,xe,ye,wnd->bc);

      /* Force each object to be updated! */
      objcnt = wnd->objcnt;
      for(i=0; i<objcnt; i++)
      {
         obj = (UG_OBJECT*)&wnd->objlst[i];
         if ( !(obj->state & OBJ_STATE_FREE) && (obj->state & OBJ_STATE_VALID) && (obj->state & OBJ_STATE_VISIBLE) ) obj->state |= (OBJ_STATE_UPDATE | OBJ_STATE_REDRAW);
      }
   }
   else
   {
      UG_FillFrame(wnd->xs,wnd->xs,wnd->xe,wnd->ye,gui->desktop_color);
   }
}

static UG_RESULT _UG_WindowClear( UG_WINDOW* wnd )
{
   if ( wnd != NULL )
   {
      if (wnd->state & WND_STATE_VISIBLE)
      {
         wnd->state &= ~WND_STATE_VISIBLE;
         UG_FillFrame( wnd->xs, wnd->ys, wnd->xe, wnd->ye, gui->desktop_color );

         if ( wnd != gui->active_window )
         {
            /* If the current window is visible, update it! */
            if ( gui->active_window->state & WND_STATE_VISIBLE )
            {
               gui->active_window->state &= ~WND_STATE_REDRAW_TITLE;
               gui->active_window->state |= WND_STATE_UPDATE;
            }
         }
      }
      return UG_RESULT_OK;
   }
   return UG_RESULT_FAIL;
}
