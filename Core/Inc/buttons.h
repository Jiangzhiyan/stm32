#ifndef __BUTTONS_H
#define __BUTTONS_H

#include "main.h"
#include "oscilloscope.h"

/* 按钮数量定义 */
#define BUTTON_COUNT 5

/* 虚拟按钮函数 */
void draw_virtual_buttons(void);
void select_next_button(void);
void press_selected_button(void);

/* 外部变量声明 */
extern virtual_button_t virtual_buttons[BUTTON_COUNT];
extern uint8_t selected_button;

#endif /* __BUTTONS_H */