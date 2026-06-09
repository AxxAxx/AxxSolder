#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_
#include "colors_legacy.h"   /* legacy palette: restore original on-screen colours */

/* Includes ------------------------------------------------------------------*/

/* Definitions ---------------------------------------------------------------*/

extern uint8_t state_power_frame_to_LCD;


extern uint16_t FIELD_W_SET;
extern uint16_t FIELD_W_SET_SMALL;
extern uint16_t FIELD_W_ACT;

extern uint8_t initialized;

#define background C_BLACK_LEGACY
#define color_text C_WHITE_LEGACY
#define color_setup B_SILVER

#define color_actual B_MANGO
               //#define color_FrameActual B_LEMON
#define color_FrameActual B_WHITE_SMOKE

#define color_text_info      C_WHITE_LEGACY
#define color_text_percent   C_BLUE_LEGACY
#define color_text_power     C_GREEN_LEGACY

/* Function prototypes -------------------------------------------------------*/
void add_data_point(uint16_t temp, uint16_t power, uint16_t set_temp);

void draw_graph_update(void);

void draw_graph_init(void);

// Universal function for a line with a specified thickness
void draw_line_with_thickness(int x1, int y1, int x2, int y2, uint16_t color, uint8_t thickness);

// Universal dashed line using UG_FillFrame()
// dash_len  ??? dash length in pixels
// gap_len   ??? gap length in pixels
// thickness ??? line thickness in pixels
void draw_dashed_line_fillframe(int x1, int y1, int x2, int y2, uint16_t color,
                                uint8_t dash_len, uint8_t gap_len, uint8_t thickness);


#endif /* INC_DISPLAY_H_ */
