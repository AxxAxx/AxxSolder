#ifndef UGUI_SIM_H_
#define UGUI_SIM_H_

#include "ugui.h"

typedef struct
{
    int width;
    int height;
    int screenMultiplier;
    int screenMargin;
    uint32_t windowBackColor;
} simcfg_t;

simcfg_t* GUI_SimCfg(void);

void GUI_Setup(UG_DEVICE *device);

void GUI_Process(void);

#endif // UGUI_SIM_H_
