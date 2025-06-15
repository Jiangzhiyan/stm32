#include "oscilloscope.h"
#include "lcd.h"
#include "delay.h"
#include <stdio.h>
#include <string.h>

/* 静态变量 - 波形显示状态 */
static uint16_t current_x = WAVE_START_X;
static uint16_t prev_dac_y = 0;
static uint16_t prev_adc_y = 0;
static uint8_t wave_initialized = 0;

/* 周期检测和时基控制 */
static uint16_t timebase_divider = 1;
static uint16_t sample_counter = 0;
static uint16_t last_dac_value = 2048;
static uint8_t rising_edge_detected = 0;
static uint16_t period_start_x = 0;
static uint16_t period_samples = 0;
static float estimated_frequency = 0.0f;
static float estimated_period_ms = 0.0f;

/* DAC波形控制参数 - 外部变量声明 */
extern uint16_t dac_amplitude;
extern uint16_t dac_offset;
extern uint16_t dac_frequency_divider;
extern uint16_t dac_min_value;
extern uint16_t dac_max_value;

/* 虚拟按钮相关 */
extern virtual_button_t virtual_buttons[];
extern uint8_t selected_button;

/* 初始化波形显示区域 */
void init_waveform_display(void)
{
  uint16_t i, x_pos;
  
  /* 清除波形显示区域 */
  lcd_fill(WAVE_START_X, WAVE_START_Y, WAVE_START_X + WAVE_WIDTH, WAVE_START_Y + WAVE_HEIGHT, WHITE);
  
  /* 绘制网格线 */
  for(i = 0; i <= 8; i++) {
    x_pos = WAVE_START_X + i * (WAVE_WIDTH / 8);
    lcd_draw_line(x_pos, WAVE_START_Y, x_pos, WAVE_START_Y + WAVE_HEIGHT, LGRAY);
  }
  for(i = 0; i <= 5; i++) {
    uint16_t y_pos = WAVE_START_Y + i * (WAVE_HEIGHT / 5);
    lcd_draw_line(WAVE_START_X, y_pos, WAVE_START_X + WAVE_WIDTH, y_pos, LGRAY);
  }
  
  /* 绘制中心分隔线 */
  uint16_t center_y = WAVE_START_Y + WAVE_HEIGHT / 2;
  lcd_draw_line(WAVE_START_X, center_y, WAVE_START_X + WAVE_WIDTH, center_y, BLACK);
  
  /* 绘制边框 */
  lcd_draw_line(WAVE_START_X, WAVE_START_Y, WAVE_START_X + WAVE_WIDTH, WAVE_START_Y, BLACK);
  lcd_draw_line(WAVE_START_X, WAVE_START_Y + WAVE_HEIGHT, WAVE_START_X + WAVE_WIDTH, WAVE_START_Y + WAVE_HEIGHT, BLACK);
  lcd_draw_line(WAVE_START_X, WAVE_START_Y, WAVE_START_X, WAVE_START_Y + WAVE_HEIGHT, BLACK);
  lcd_draw_line(WAVE_START_X + WAVE_WIDTH, WAVE_START_Y, WAVE_START_X + WAVE_WIDTH, WAVE_START_Y + WAVE_HEIGHT, BLACK);
  
  /* 绘制标签 */
  lcd_show_string(15, WAVE_START_Y - 20, 50, 16, 16, "DAC", BLUE);
  lcd_show_string(15, WAVE_START_Y + 10, 50, 16, 16, "3.3V", GRAY);
  lcd_show_string(15, WAVE_START_Y + WAVE_HEIGHT/2 - 20, 50, 16, 16, "0V", GRAY);
  
  lcd_show_string(15, WAVE_START_Y + WAVE_HEIGHT/2 + 10, 50, 16, 16, "ADC", RED);
  lcd_show_string(15, WAVE_START_Y + WAVE_HEIGHT/2 + 30, 50, 16, 16, "3.3V", GRAY);
  lcd_show_string(15, WAVE_START_Y + WAVE_HEIGHT - 20, 50, 16, 16, "0V", GRAY);
  
  /* 绘制图例 */
  lcd_show_string(80, WAVE_START_Y + WAVE_HEIGHT + 10, 200, 16, 16, "Upper:DAC  Lower:ADC", BLACK);
  
  /* X轴时间标注 */
  for(i = 0; i <= 8; i++) {
    x_pos = WAVE_START_X + i * (WAVE_WIDTH / 8);
    char time_str[10];
    float time_ms = (float)i * 10.0f;
    sprintf(time_str, "%.0fms", time_ms);
    lcd_show_string(x_pos - 15, WAVE_START_Y + WAVE_HEIGHT + 5, 30, 12, 12, time_str, BLACK);
  }
  
  wave_initialized = 1;
}

/* 周期检测和时基自动调整 */
void detect_period_and_adjust_timebase(uint16_t dac_value)
{
  uint16_t center_value = dac_offset;
  uint16_t threshold = 100;
  
  if(dac_value > last_dac_value + threshold && last_dac_value < center_value) {
    if(!rising_edge_detected) {
      rising_edge_detected = 1;
      period_start_x = current_x;
      period_samples = 0;
    } else {
      uint16_t period_width = current_x - period_start_x;
      if(period_width > 10) {
        period_samples = sample_counter;
        
        if(period_samples > 0) {
          estimated_frequency = 78.9f / period_samples;
          estimated_period_ms = 1000.0f / estimated_frequency;
        }
        
        uint16_t target_width = WAVE_WIDTH / 5;
        
        if(period_width < target_width / 2) {
          if(timebase_divider > 1) timebase_divider--;
        } else if(period_width > target_width * 2) {
          if(timebase_divider < 16) timebase_divider++;
        }
      }
      
      period_start_x = current_x;
      period_samples = 0;
    }
    sample_counter = 0;
  }
  
  last_dac_value = dac_value;
  sample_counter++;
}

/* 绘制单个波形点 */
void draw_waveform_point(uint16_t dac_value, uint16_t adc_value)
{
  uint16_t dac_y, adc_y;
  
  /* 计算Y坐标 */
  dac_y = WAVE_START_Y + (WAVE_HEIGHT/2) - (dac_value * (WAVE_HEIGHT/2) / 4096);
  adc_y = WAVE_START_Y + (WAVE_HEIGHT/2) + 10 + (4096 - adc_value) * (WAVE_HEIGHT/2 - 20) / 4096;
  
  /* 清除当前X位置的垂直线 */
  lcd_draw_line(current_x, WAVE_START_Y + 1, current_x, WAVE_START_Y + WAVE_HEIGHT - 1, WHITE);
  lcd_draw_line(current_x + 1, WAVE_START_Y + 1, current_x + 1, WAVE_START_Y + WAVE_HEIGHT - 1, WHITE);
  lcd_draw_line(current_x + 2, WAVE_START_Y + 1, current_x + 2, WAVE_START_Y + WAVE_HEIGHT - 1, WHITE);
  
  /* 重绘网格线 */
  if((current_x - WAVE_START_X) % (WAVE_WIDTH / 8) == 0) {
    lcd_draw_line(current_x, WAVE_START_Y, current_x, WAVE_START_Y + WAVE_HEIGHT, LGRAY);
  }
  uint16_t center_y = WAVE_START_Y + WAVE_HEIGHT / 2;
  lcd_draw_point(current_x, center_y, GRAY);
  
  /* 绘制连接线 */
  if(wave_initialized && current_x > WAVE_START_X) {
    /* DAC连接线 */
    for(int thick = -1; thick <= 1; thick++) {
      lcd_draw_line(current_x - 1, prev_dac_y + thick, current_x, dac_y + thick, BLUE);
      lcd_draw_line(current_x - 1 + thick, prev_dac_y, current_x + thick, dac_y, BLUE);
    }
    
    /* ADC连接线 */
    for(int thick = -1; thick <= 1; thick++) {
      lcd_draw_line(current_x - 1, prev_adc_y + thick, current_x, adc_y + thick, RED);
      lcd_draw_line(current_x - 1 + thick, prev_adc_y, current_x + thick, adc_y, RED);
    }
  }
  
  /* 绘制当前点 */
  for(int dx = -1; dx <= 1; dx++) {
    for(int dy = -1; dy <= 1; dy++) {
      lcd_draw_point(current_x + dx, dac_y + dy, BLUE);
      lcd_draw_point(current_x + dx, adc_y + dy, RED);
    }
  }
  
  prev_dac_y = dac_y;
  prev_adc_y = adc_y;
  
  current_x += timebase_divider;
  
  if(current_x >= WAVE_START_X + WAVE_WIDTH - timebase_divider) {
    current_x = WAVE_START_X;
    rising_edge_detected = 0;
    period_samples = 0;
    sample_counter = 0;
    
    lcd_draw_line(current_x, WAVE_START_Y, current_x, WAVE_START_Y + WAVE_HEIGHT, YELLOW);
  }
}