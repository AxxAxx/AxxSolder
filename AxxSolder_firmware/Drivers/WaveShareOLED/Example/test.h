/*****************************************************************************
* | File      	:   test.h
* | Author      :   Waveshare team
* | Function    :   
* | Info        :
*
*----------------
* |	This version:   V1.0
* | Date        :   2020-08-13
* | Info        :   Basic version
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#ifndef _TEST_H_
#define _TEST_H_

#include "DEV_Config.h"
#include "GUI_Paint.h"
#include "ImageData.h"
#include "Debug.h"

#include <stdlib.h> // malloc() free()
#include <math.h>

int OLED_0in91_test(void);
int OLED_0in95_rgb_test(void);
int OLED_0in96_test(void);
int OLED_1in3_test(void);
int OLED_1in3_c_test(void);
int OLED_1in5_test(void);
int OLED_1in5_rgb_test(void);

#endif

