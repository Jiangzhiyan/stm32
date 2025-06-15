#include "buttons.h"
#include "lcd.h"
#include "oscilloscope.h"
#include <stdio.h>
#include <string.h>

/* 虚拟按钮定义 */
virtual_button_t virtual_buttons[BUTTON_COUNT] = {
    {20,  750, 80, 30, "Freq+", BLUE, YELLOW},
    {110, 750, 80, 30, "Freq-", BLUE, YELLOW},
    {200, 750, 80, 30, "Volt+", RED, YELLOW},
    {290, 750, 80, 30, "Volt-", RED, YELLOW},
    {380, 750, 80, 30, "Reset", GRAY, YELLOW}
};

uint8_t selected_button = 0;

/* 外部变量声明 */
extern uint16_t dac_amplitude;
extern uint16_t dac_frequency_divider;
extern uint16_t dac_offset;

/* 绘制虚拟按钮 */
void draw_virtual_buttons(void)
{
    for(int i = 0; i < BUTTON_COUNT; i++) {
        uint16_t bg_color = (i == selected_button) ? virtual_buttons[i].highlight_color : virtual_buttons[i].color;
        uint16_t text_color = (i == selected_button) ? BLACK : WHITE;
        
        /* 绘制按钮背景 */
        lcd_fill(virtual_buttons[i].x, virtual_buttons[i].y, 
                 virtual_buttons[i].x + virtual_buttons[i].width, 
                 virtual_buttons[i].y + virtual_buttons[i].height, 
                 bg_color);
        
        /* 绘制按钮边框 */
        lcd_draw_line(virtual_buttons[i].x, virtual_buttons[i].y, 
                      virtual_buttons[i].x + virtual_buttons[i].width, virtual_buttons[i].y, BLACK);
        lcd_draw_line(virtual_buttons[i].x, virtual_buttons[i].y, 
                      virtual_buttons[i].x, virtual_buttons[i].y + virtual_buttons[i].height, BLACK);
        lcd_draw_line(virtual_buttons[i].x + virtual_buttons[i].width, virtual_buttons[i].y, 
                      virtual_buttons[i].x + virtual_buttons[i].width, virtual_buttons[i].y + virtual_buttons[i].height, BLACK);
        lcd_draw_line(virtual_buttons[i].x, virtual_buttons[i].y + virtual_buttons[i].height, 
                      virtual_buttons[i].x + virtual_buttons[i].width, virtual_buttons[i].y + virtual_buttons[i].height, BLACK);
        
        /* 绘制按钮文字 */
        uint16_t text_x = virtual_buttons[i].x + (virtual_buttons[i].width - strlen(virtual_buttons[i].text) * 8) / 2;
        uint16_t text_y = virtual_buttons[i].y + (virtual_buttons[i].height - 16) / 2;
        lcd_show_string(text_x, text_y, virtual_buttons[i].width, 16, 16, virtual_buttons[i].text, text_color);
    }
    
    /* 显示操作说明 */
    lcd_fill(20, 720, 450, 740, WHITE);
    lcd_show_string(20, 720, 300, 16, 16, "KEY0: Select   KEY1: Press", BLACK);
    
    /* 显示当前选中的按钮 */
    char selected_str[50];
    sprintf(selected_str, "Selected: %s", virtual_buttons[selected_button].text);
    lcd_show_string(300, 720, 150, 16, 16, selected_str, BLUE);
}

/* 切换选中的按钮 */
void select_next_button(void)
{
    selected_button = (selected_button + 1) % BUTTON_COUNT;
    draw_virtual_buttons();
}

/* 点击当前选中的按钮 */
void press_selected_button(void)
{
    char action_str[50] = "";
    
    switch(selected_button) {
        case 0:  /* Freq+ */
            if(dac_frequency_divider > 1) {
                dac_frequency_divider--;
                sprintf(action_str, "Freq+ Applied: %d", dac_frequency_divider);
            } else {
                sprintf(action_str, "Freq+ Max reached");
            }
            break;
            
        case 1:  /* Freq- */
            if(dac_frequency_divider < 32) {
                dac_frequency_divider++;
                sprintf(action_str, "Freq- Applied: %d", dac_frequency_divider);
            } else {
                sprintf(action_str, "Freq- Min reached");
            }
            break;
            
        case 2:  /* Volt+ */
            if(dac_amplitude < 3600) {
                dac_amplitude += 200;
                sprintf(action_str, "Volt+ Applied: %d", dac_amplitude);
            } else {
                sprintf(action_str, "Volt+ Max reached");
            }
            break;
            
        case 3:  /* Volt- */
            if(dac_amplitude > 400) {
                dac_amplitude -= 200;
                sprintf(action_str, "Volt- Applied: %d", dac_amplitude);
            } else {
                sprintf(action_str, "Volt- Min reached");
            }
            break;
            
        case 4:  /* Reset */
            dac_amplitude = 1800;
            dac_frequency_divider = 8;
            dac_offset = 2048;
            sprintf(action_str, "Reset Applied");
            init_waveform_display();
            break;
            
        default:
            sprintf(action_str, "Unknown button");
            break;
    }
    
    /* 显示动作提示 */
    lcd_fill(20, 700, 450, 720, WHITE);
    lcd_show_string(20, 700, 450, 16, 16, action_str, BLUE);
    
    draw_virtual_buttons();
}