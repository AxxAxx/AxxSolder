/*
 * Copyright (c) 2015, Ari Suutari <ari@stonepile.fi>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote
 *     products derived from this software without specific prior written
 *     permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT,  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

 /*
  *  19.10.2017 jkpublic@kartech.biz - Added support for 8BPP fonts ( anti alliased)
  *
  */


#include <stdio.h>
#include <getopt.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#define SCREEN_WIDTH 132
#define SCREEN_HEIGHT 40
#include "ugui.h"

static void drawPixel(UG_S16 x, UG_S16 y, UG_COLOR col);

static UG_GUI gui;
static UG_DEVICE device = {
    .x_dim = SCREEN_WIDTH,
    .y_dim = SCREEN_HEIGHT,
    .pset = drawPixel,
};

static float fontSize = 0;
static int dpi = 0;
static int bpp = 1;
static int enable_widths = 1;

#define isNumber(n) ((n) >= '0' && (n) <= '9')

char     *charArg = "32-126";  // Default, use 32-126 range
uint16_t chars[64*1024];
uint16_t charCount;
uint8_t  offsets[64*1024*2];
uint16_t offsetCount;

// Same as uGUI's UG_FONT_DATA, but without const qualifiers, so we can use ram pointers. Only for ttf2ugui
typedef struct
{
   FONT_TYPE  font_type;
   UG_U8      is_old_font;
   UG_U8      char_width;
   UG_U8      char_height;
   UG_U16     bytes_per_char;
   UG_U16     number_of_chars;
   UG_U16     number_of_offsets;
   UG_U8      * widths;
   UG_U8      * offsets;
   UG_U8      * data;
   UG_FONT    * font;   // Unused here
} UG_FONT_DATA_RAM;

/*
 * "draw" a pixel using ansi escape sequences.
 * Used for printing a ascii art sample of font.
 */
static void drawPixel(UG_S16 x, UG_S16 y, UG_COLOR col)
{
  printf("\033[%d;%dH", y + 1, x + 1);

  if (col == C_WHITE)
    printf("\x1B[0m ");
  else
  {
    if(col == C_BLACK)
    {
        printf("\x1B[0m■");
    }
    else
    {
        printf("\x1B[34m■");
    }

  }

  fflush(stdout);
}

/*
 * Parse chars, create ranges and generate the complete char list to be generated
 * Based on code from: https://github.com/olikraus/u8g2/blob/master/tools/font/otf2bdf/otf2bdf.c
 */
void parse_chars(char *s)
{
    uint16_t l, r;

    /*
     * Make sure to clear the flag and bitmap in case more than one subset is
     * specified on the command line.
     */
    memset(chars, 0, sizeof(chars));  
    charCount = 0;
    uint16_t *charPtr=chars;
    while (*s) {
        /*
         * Collect the next code value.
         */
        for (l = r = 0; *s && isNumber(*s); s++){
          l = (l * 10) + (*s - '0');
    }

        /*
         * If the next character is an '_' or '-', advance and collect the end of the
         * specified range.
         */
        if (*s == '_'|| *s == '-') {
            s++;
            for (; *s && isNumber(*s); s++){
              r = (r * 10) + (*s - '0');
      }
        }
    else{        
      r = l;
    }
    
    for(uint32_t t=l; t<=r; t++){
      charCount++;
      *charPtr++ = t;
    }

        /*
         * Skip all non-digit characters.
         */
        while (*s && !isNumber(*s))
          s++;
    }
  
    /*
    * Compute char ranges
    */
    for (uint16_t ch=0; ch<charCount; ){
      offsets[offsetCount*2]=chars[ch]>>8;
      offsets[(offsetCount*2)+1]=chars[ch]&0xFF;
      offsetCount++;
      ch++;  
      if(chars[ch]==chars[ch-1]+1){
        offsets[(offsetCount-1)*2] |= 0x80;
        //printf("Offset Range Start: %u\n",offsets[offsetCount-1]&0x7FFF);    
        while(ch<charCount-1 && chars[ch]+1==chars[ch+1]){      //Skip consecutive chars
          ch++;
        }
        offsets[offsetCount*2]=chars[ch]>>8;
        offsets[(offsetCount*2)+1]=chars[ch]&0xFF;
        offsetCount++;
        ch++;  
        //printf("Offset Range End: %u\n",offsets[offsetCount-1]&0x7FFF);
      }
      else{
        //printf("Offset Single char: %u\n",offsets[offsetCount-1]);    
      }
    }
}

/*
 * Convert unicode to utf-8 array
 * Credits: https://gist.github.com/MightyPork/52eda3e5677b4b03524e40c9f0ab1da5
 */
int utf8_encode(char *out, uint32_t utf)
{
  if (utf <= 0x7F) {
    // Plain ASCII
    out[0] = (char) utf;
    out[1] = 0;
    return 1;
  }
  else if (utf <= 0x07FF) {
    // 2-byte unicode
    out[0] = (char) (((utf >> 6) & 0x1F) | 0xC0);
    out[1] = (char) (((utf >> 0) & 0x3F) | 0x80);
    out[2] = 0;
    return 2;
  }
  else if (utf <= 0xFFFF) {
    // 3-byte unicode
    out[0] = (char) (((utf >> 12) & 0x0F) | 0xE0);
    out[1] = (char) (((utf >>  6) & 0x3F) | 0x80);
    out[2] = (char) (((utf >>  0) & 0x3F) | 0x80);
    out[3] = 0;
    return 3;
  }
  else if (utf <= 0x10FFFF) {
    // 4-byte unicode
    out[0] = (char) (((utf >> 18) & 0x07) | 0xF0);
    out[1] = (char) (((utf >> 12) & 0x3F) | 0x80);
    out[2] = (char) (((utf >>  6) & 0x3F) | 0x80);
    out[3] = (char) (((utf >>  0) & 0x3F) | 0x80);
    out[4] = 0;
    return 4;
  }
  else { 
    // error - use replacement character
    out[0] = (char) 0xEF;  
    out[1] = (char) 0xBF;
    out[2] = (char) 0xBD;
    out[3] = 0;
    return 0;
  }
}

static int max(int a, int b)
{
  if (a > b)
    return a;

  return b;
}

/*
 * Output C-language code that can be used to include
 * converted font into uGUI application.
 */
static void dumpFont(UG_FONT_DATA_RAM * font, const char* fontFile, float fontSize,int bitsPerPixel)
{
  int bytesPerChar;
  int ch;
  int current;
  int b;
  char fontName[80];
  char fileNameBuf[80];
  const char* baseName;
  char outFileName[80];
  char* ptr;
  FILE* out;
  uint8_t newline=0;

/*
 * Generate name for font by stripping path and suffix from filename, also remove spaces.
 */
  baseName = fontFile;
  ptr = strrchr(baseName, '/');
  if (ptr)
    baseName = ptr + 1;

  strcpy(fileNameBuf, baseName);
  
  for(uint8_t t=0;;t++){
    if(baseName[t]==0){
    fileNameBuf[t] = 0; 
    break;
    }
    else if(baseName[t] == ' '){
    fileNameBuf[t] = '_';
    }
    else{
    fileNameBuf[t] = baseName[t];
    }    
  }
  
  baseName = fileNameBuf;
  ptr = strchr(baseName, '.');
  if (ptr)
    *ptr = '\0';


  sprintf(fontName, "%s_%dX%d", baseName, font->char_width, font->char_height);
  sprintf(outFileName, "%s_%dX%d.c", baseName, font->char_width, font->char_height);


  out = fopen(outFileName, "w");
  if (!out) {

    perror(outFileName);
    exit(2);
  }


  /*
   * First output character bitmaps.
  */
  switch(bitsPerPixel)
  {
    case 1:
    {
        // Round up to full bytes
        bytesPerChar = font->char_height * ((font->char_width +7)/ 8);
    }break;

    case 8:
    {
        bytesPerChar = font->char_height * font->char_width;
    }break;
  }
  
  /*
  * Write font into
  */
  //fprintf(out, "  #include \"%s_%dX%d.h\"\n\n", baseName, font->char_width, font->char_height);
  fprintf(out, "// Converted from %s\n", fontFile);
  fprintf(out, "//  --size %.1f\n", fontSize);
  if (dpi > 0)
    fprintf(out, "//  --dpi %d\n", dpi);
  fprintf(out, "//  --bpp %d\n\n", (int)bitsPerPixel);  
  fprintf(out, "// For copyright, see original font file.\n\n");
  fprintf(out, "/************************************************\n");
  fprintf(out, "Add this lines to ugui.h:\n");
  fprintf(out, "  #ifdef USE_FONT_%s\n",fontName);
  fprintf(out, "  extern UG_FONT FONT_%s[];\n", fontName);
  fprintf(out, "  #endif\n\n");
  fprintf(out, "To enable this font, add this line to ugui_config.h:\n");
  fprintf(out, "  #define UGUI_USE_FONT_%s\n", fontName);
  fprintf(out, "************************************************/\n\n");

  fprintf(out, "#include \"ugui.h\"\n");  
  fprintf(out, "#ifdef UGUI_USE_FONT_%s\n\n",fontName);  
  fprintf(out, "UG_FONT FONT_%s[] = {\n", fontName );
  
  // Print Header
  fprintf(out, "  // BPP, Width, Height, Chars, Offsets, Bytes per char, Widths presence bit\n");
  fprintf(out, "  0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,\n",
          font->font_type,
          font->char_width,
          font->char_height,
          (font->number_of_chars>>8)&0xFF,
          font->number_of_chars&0xFF,
          (font->number_of_offsets>>8)&0xFF,
          font->number_of_offsets&0xFF,
          (font->bytes_per_char>>8)&0xFF,          
          font->bytes_per_char&0xFF,
          enable_widths);
      
      
  // Print char widths if enabled
  if(enable_widths){
    fprintf(out, "  // Widths\n  ");
    newline=0;
    for (ch = 0; ch < charCount;) {
      fprintf(out, "0x%02X,", font->widths[ch++]);  
      newline=0;
      if(ch && ch%10==0){
        fprintf(out, "\n  ");
        newline=1;
      }
    }
    if(!newline)
      fprintf(out, "\n  ");
  }
  else{
    fprintf(out, "  ");
  }
  
  // Print char offsets
  fprintf(out, "// Offsets\n  ");
 
  newline=0;
  for(uint16_t t=0;t<offsetCount*2;){
    newline=0;  
    fprintf(out, "0x%02X,", font->offsets[t++]);
    if(t && t%10==0){
      fprintf(out, "\n  ");
      newline=1;
    }
  }
  if(!newline)
    fprintf(out, "\n  "); 
      
  fprintf(out, "// Bitmap data%*c  Hex     Dec   Char (UTF-8)\n",(bytesPerChar*5)-12, ' ');
  current = 0;
  
  for (ch = 0; ch < charCount; ch++ ) {
  if(!chars[ch]){
    return;
  }
    fprintf(out, "  ");
    for (b = 0; b < bytesPerChar; b++) {
      fprintf(out, "0x%02X,", font->data[current]);
      ++current;
    }
  
  char utf8[5];
  utf8_encode(utf8, chars[ch]);
    fprintf(out, " // 0x%-4X  %-4u  '%s'\n", chars[ch], chars[ch], utf8);
  }

  fprintf(out, "};\n\n#endif\n");

  fclose(out);
}

static UG_FONT_DATA_RAM newFont;

static UG_FONT_DATA_RAM *convertFont(const char *font, int dpi, float fontSize,int bitsPerPixel)
{
  int     error;
  FT_Face   face;
  FT_Library   library;
  int     bpp_mul;


  switch(bitsPerPixel)
  {
    case 1: bpp_mul = 1; break;
    case 8: bpp_mul = 16; break;
    default:
    {
       fprintf(stderr, "Bits per pixel must be 1 or 8, not %d!!\n", bitsPerPixel);
       exit(1);
    }break;

  }

/*
 * Initialize freetype library, load the font
 * and set output character size.
 */
  error = FT_Init_FreeType(&library);
  if (error) {

    fprintf(stderr, "ft init err %d\n", error);
    exit(1);
  }

  error = FT_New_Face(library,
                      font,
                      0,
                      &face);
  if (error) {

    fprintf(stderr, "ew faceerr %d\n", error);
    exit(1);
  }

/*
 * If DPI is not given, use pixes to specify the size.
 */
  if (dpi > 0)
    error = FT_Set_Char_Size(face, 0, fontSize * 64 * bpp_mul, dpi, dpi);
  else
    error = FT_Set_Pixel_Sizes(face, 0, fontSize * bpp_mul);
  if (error) {

    fprintf(stderr, "set pixel sizes err %d\n", error);
    exit(1);
  }
  
  
  parse_chars(charArg);
  

  int i, j,i_idx,j_idx;
  int coverage;
  uint32_t ch;
  int maxWidth = 0;
  int maxHeight = 0;
  int maxAscent = 0;
  int maxDescent = 0;
  int bytesPerChar;
  int bytesPerRow;

/*
 * First found out how big character bitmap is needed. Every character
 * must fit into it so that we can obtain correct character positioning.
 */
  for (ch = 0; ch <charCount; ch++) {

    int ascent;
    int descent;
  if(!chars[ch]){
    fprintf(stderr, "No chars defined %d\n", error);
    exit(1);
  }
    error = FT_Load_Char(face, chars[ch], FT_LOAD_RENDER | FT_LOAD_TARGET_MONO);
    if (error) {

      fprintf(stderr, "load char err %d\n", error);
      exit(1);
    }

    descent = max(0, face->glyph->bitmap.rows - face->glyph->bitmap_top);
    ascent = max(0, max(face->glyph->bitmap_top, face->glyph->bitmap.rows) - descent);

    if (descent > maxDescent)
      maxDescent = descent;

    if (ascent > maxAscent)
      maxAscent = ascent;

    if (face->glyph->bitmap.width > maxWidth)
      maxWidth = face->glyph->bitmap.width;
  }

  maxWidth = maxWidth / bpp_mul;
  maxHeight = (maxAscent + maxDescent) / bpp_mul;

  switch(bitsPerPixel)
  {
    case 1:
    {
        // Round up to full bytes
        bytesPerRow = (maxWidth +7 )/ 8;
    }break;

    case 8:
    {
        bytesPerRow = maxWidth;
    }break;
  }


  bytesPerChar = bytesPerRow * maxHeight;
  newFont.data = calloc(1, bytesPerChar * charCount);
  newFont.number_of_offsets = offsetCount;
  newFont.offsets = offsets;
  
  switch(bitsPerPixel)
  {
        case 1: newFont.font_type = FONT_TYPE_1BPP; break;
        case 8: newFont.font_type = FONT_TYPE_8BPP; break;
  }

  newFont.char_width  = maxWidth;
  newFont.char_height = maxHeight;
  newFont.bytes_per_char = bytesPerChar;
  newFont.number_of_chars = charCount;
  newFont.widths      = malloc(charCount);

/*
 * Render each character.
 */
  for (ch = 0; ch <charCount; ch++) {

    error = FT_Load_Char(face, chars[ch], FT_LOAD_RENDER | FT_LOAD_TARGET_MONO);
    if (error) {

      fprintf(stderr, "load char err %d\n", error);
      exit(1);
    }

    for (i = 0; i < face->glyph->bitmap.rows / bpp_mul; i++)
      for (j = 0; j < face->glyph->bitmap.width / bpp_mul; j++) {

        coverage = 0;

        for(i_idx =0; i_idx < bpp_mul;i_idx++) {
            for(j_idx = 0; j_idx < bpp_mul;j_idx++) {


                uint8_t *bits = (uint8_t *) face->glyph->bitmap.buffer;
                uint8_t b = bits[(i*bpp_mul+i_idx) * face->glyph->bitmap.pitch + ((j*bpp_mul+j_idx) / 8)];


                if (b & (1 << (7 - ((j*bpp_mul+j_idx) % 8))))
                {
                    coverage ++;
                }

            }
        }


        /*
         * Output character to correct position in bitmap
         */

        int xpos, ypos,ind;

        xpos = j + (face->glyph->bitmap_left / bpp_mul);
        ypos = (maxAscent/bpp_mul) + i - (face->glyph->bitmap_top / bpp_mul);


        switch(bitsPerPixel)
        {

            case 1:
            {

                ind = ypos * bytesPerRow;
                ind += xpos / 8;


                if (coverage !=0)
                    newFont.data[(ch * bytesPerChar) + ind] |= (1 << ((xpos % 8)));
            }break;

            case 8:
            {
                ind = ypos * bytesPerRow;
                ind += xpos ;

                newFont.data[(ch * bytesPerChar) + ind  ] = (255 * coverage)/256; // need to be 0..255 range

            }break;
        }
      }

    /*
     * Save character width, freetype uses 1/64 as units for it.
     */
    newFont.widths[ch] = (face->glyph->advance.x >> 6) / bpp_mul;

  }

  return &newFont;
}

/*
 * Draw a simple sample of new font with uGUI.
 */
static void showFont( UG_FONT_DATA_RAM * font, char* text)
{
  UG_Init(&gui, &device);
  UG_FillScreen(C_WHITE);
  UG_DrawFrame(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, C_BLACK);
  memcpy(&gui.currentFont, &newFont, sizeof(UG_FONT_DATA_RAM));
  UG_SetBackcolor(C_WHITE);
  UG_SetForecolor(C_BLACK);
  UG_PutString(2, 2, text);
  UG_DrawPixel(0, SCREEN_HEIGHT - 1, C_WHITE);
  UG_Update();
}

static int dump;
static char* fontFile = NULL;
static char* showText = NULL;

 /* options descriptor */
static struct option longopts[] = {
  {"show", required_argument, NULL, 'a'},
  {"dump", no_argument, &dump, 1},
  {"dpi", required_argument, NULL, 'd'},
  {"chars", optional_argument, NULL, 'c'},
  {"size", required_argument, NULL, 's'},
  {"font", required_argument, NULL, 'f'},
  {"bpp", optional_argument, NULL, 'b'},
  {"mono", optional_argument, NULL, 'm'},
  {NULL, 0, NULL, 0}
};

static void usage()
{
  fprintf(stderr, "\nttf2ugui {--show=\"Text\" | --dump} [--mono] [--dpi=displaydpi] [--bpp=bitsperpixel] [--chars=chars] --font=fontfile --size=fontsize\n\n");  
  fprintf(stderr, "--dump : Create the C font file for uGUI\n");
  fprintf(stderr, "--show : Prints an example text using uGUI and the rendered font. Only works with ASCII text.\n");
  fprintf(stderr, "--dpi  : Sets the dpi. If not given, font size is assumed to be pixels.\n");
  fprintf(stderr, "--bpp  : Sets bits per pixel, must be 1 or 8. Default is 1.\n");
  fprintf(stderr, "--mono : Disables width table generation, saving some space. Non-monospaced fonts will look bad!\n");
  fprintf(stderr, "--font : Specifies the font file to be used.\n");
  fprintf(stderr, "--chars: ASCII or Unicode codes. Can be single and/or ranges in any combination. Needs ordering from lower to higher. Default is 32-126.\n");
  fprintf(stderr, "         Ex. --chars=32-90,176,220-225 will generate chars from 32 to 90, single char 176 and chars from 220 to 225.\n");
}

int main(int argc, char **argv)
{
  int ch;

  while ((ch = getopt_long(argc, argv, "", longopts, NULL)) != -1) {

    switch (ch) {
    case 'f':
      fontFile = optarg;
      break;

    case 'a':
      showText = optarg;
      break;

    case 's':
      sscanf(optarg, "%f", &fontSize);
      break;

    case 'd':
      dpi = atoi(optarg);
      break;

    case 'b':
      bpp = atoi(optarg);

      if( (bpp !=1) && (bpp !=8) )
      {
        fprintf(stderr, "Bits per pixel must be 1 or 8. Default is 1.\n");
        exit(1);
      }
      break;

    case 'c':
      charArg = optarg;
      break;
      
    case 'm':
      enable_widths=0;
      break;

    case 0:
      break;

    default:
      usage();
      exit(1);
    }
  }

  argc -= optind;
  argv += optind;

  
  if ((!dump && showText == NULL) || fontFile == NULL || fontSize == 0) {

    usage();
    exit(1);
  }

  UG_FONT_DATA_RAM *font = convertFont(fontFile, dpi, fontSize,bpp);

  if (showText)
    showFont(font, showText);

  if (dump)
    dumpFont(font, fontFile, fontSize,bpp);
}
