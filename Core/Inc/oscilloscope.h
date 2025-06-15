#ifndef __OSCILLOSCOPE_H
#define __OSCILLOSCOPE_H

#include "main.h"

/* 虚拟按钮结构体定义 */
typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    char* text;
    uint16_t color;
    uint16_t highlight_color;
} virtual_button_t;

/* 波形显示相关函数 */
void init_waveform_display(void);
void draw_waveform_point(uint16_t dac_value, uint16_t adc_value);
void detect_period_and_adjust_timebase(uint16_t dac_value);

/* 虚拟按钮相关函数 */
void draw_virtual_buttons(void);
void select_next_button(void);
void press_selected_button(void);

/* 波形显示参数 */
#define WAVE_HEIGHT 400
#define WAVE_START_Y 180
#define WAVE_WIDTH 460
#define WAVE_START_X 10

#endif /* __OSCILLOSCOPE_H */