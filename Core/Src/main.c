/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oscilloscope.h"
#include "buttons.h"
#include "lcd.h"
#include "delay.h"
#include "touch.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* DAC波形控制参数 */
uint16_t dac_amplitude = 1800;
uint16_t dac_offset = 2048;
uint16_t dac_frequency_divider = 8;
uint16_t dac_min_value = 200;
uint16_t dac_max_value = 3800;

volatile uint8_t timer_flag = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance == TIM6)
  {
    timer_flag = 1;
  }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  uint16_t dac_value = 0;
  uint32_t adc_value = 0;
  uint16_t dac_step = 0;
  uint8_t direction = 1;  /* DAC输出方向: 1=上升, 0=下降 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  delay_init(72);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC3_Init();
  MX_DAC_Init();
  MX_FSMC_Init();
  MX_TIM6_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  
  /* 初始化系统延时 */
  delay_ms(100);
  
  /* 初始化LCD */
  lcd_init();
  
  /* 初始化SPI触摸屏 */
  touch_config_t touch_cfg = GT968_SPI_CONFIG;  /* 使用SPI接口的GT968 */
  
  if(touch_init(&touch_cfg)) {
    printf("SPI Touch screen initialized successfully\\r\\n");
  } else {
    printf("SPI Touch screen initialization failed, using key simulation\\r\\n");
  }
  
  printf("Touch control: Real touch + KEY simulation\\r\\n");
  printf("KEY0: Reset, KEY1: Test buttons\\r\\n");
  
  /* 启动DAC */
  HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
  
  /* 显示初始界面 */
  lcd_clear(WHITE);
  lcd_show_string(120, 20, 300, 24, 24, "STM32 Oscilloscope", BLACK);
  lcd_show_string(140, 50, 200, 20, 20, "Auto 2-Period Sync", BLACK);
  
  /* 显示控制说明 */
  lcd_show_string(50, 90, 200, 16, 16, "KEY0: Reset  KEY1: Test", GRAY);
  lcd_show_string(50, 110, 200, 16, 16, "Connect DAC to ADC", GRAY);
  lcd_show_string(50, 130, 200, 16, 16, "Touch buttons for control", GRAY);
  
  /* 初始化波形显示区域 */
  init_waveform_display();
  
  /* 绘制虚拟按钮 */
  draw_virtual_buttons();
  
  /* 启动定时器 */
  HAL_TIM_Base_Start_IT(&htim6);
  
  printf("DAC/ADC Test Started\r\n");
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* 检查定时器标志 */
    if(timer_flag)
    {
      timer_flag = 0;
      
      /* 生成DAC输出 - 可调节三角波 */
      static uint16_t dac_freq_counter = 0;
      
      /* 频率控制 - 只有当计数器达到分频值时才更新DAC */
      if(++dac_freq_counter >= dac_frequency_divider) {
        dac_freq_counter = 0;
        
        /* 计算当前的幅度范围 */
        dac_min_value = dac_offset - dac_amplitude/2;
        dac_max_value = dac_offset + dac_amplitude/2;
        
        /* 限制范围在DAC有效范围内 */
        if(dac_min_value < 100) dac_min_value = 100;
        if(dac_max_value > 3900) dac_max_value = 3900;
        
        /* 计算动态步长，确保固定的三角波周期 */
        uint16_t amplitude_range = dac_max_value - dac_min_value;
        uint16_t triangle_steps = 100;  /* 固定100步完成一个半周期 */
        uint16_t dynamic_step = amplitude_range / triangle_steps;
        if(dynamic_step < 1) dynamic_step = 1;  /* 最小步长 */
        
        if(direction)
        {
          dac_step += dynamic_step;
          if(dac_step >= dac_max_value) 
          {
            dac_step = dac_max_value;
            direction = 0;
          }
        }
        else
        {
          dac_step -= dynamic_step;
          if(dac_step <= dac_min_value)
          {
            dac_step = dac_min_value;
            direction = 1;
          }
        }
      }
      
      /* 设置DAC输出值 */
      dac_value = dac_step;
      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_value);
      
      /* 读取ADC值 (单次采样，最高响应速度) */
      adc_value = adc_get_result(ADC_CHANNEL_1);
      
      
      /* 周期检测和时基自动调整 */
      detect_period_and_adjust_timebase(dac_value);
      
      /* 绘制波形点 - 真正的示波器效果 */
      draw_waveform_point(dac_value, adc_value);
      
      /* 显示基本信息 (每50次更新一次) */
      static uint16_t info_counter = 0;
      if(++info_counter >= 50) {
        info_counter = 0;
        
        uint16_t info_y = 650;
        
        /* 清除信息显示区域 */
        lcd_fill(10, info_y - 5, 470, info_y + 40, WHITE);
        
        /* 显示基本数值 */
        char info_str[50];
        sprintf(info_str, "DAC:%d ADC:%lu FreqDiv:%d", dac_value, adc_value, dac_frequency_divider);
        lcd_show_string(20, info_y, 400, 16, 16, info_str, BLACK);
        
        /* 重绘按钮 */
        draw_virtual_buttons();
      }
      
      /* 串口输出数据 */
      printf("DAC:%d ADC:%lu\r\n", dac_value, adc_value);
    }
    
    /* 检查KEY0 - 切换参数 */
    if(HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin) == GPIO_PIN_RESET)
    {
      delay_ms(20);  /* 消抖 */
      if(HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin) == GPIO_PIN_RESET)
      {
        select_next_button();
        while(HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin) == GPIO_PIN_RESET); /* 等待按键释放 */
      }
    }
    
    /* 检查KEY1 - 调整参数值 */
    if(HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
    {
      delay_ms(20);  /* 消抖 */
      if(HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
      {
        press_selected_button();
        while(HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET); /* 等待按键释放 */
      }
    }
    
    /* 简化后不再需要触摸检测，只使用实体按键 */
    
    /* KEY1功能已移除，专注于自动两周期显示 */
    
    delay_ms(10);  /* 主循环延时 */
    /* USER CODE END WHILE */
 
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
