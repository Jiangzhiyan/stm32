/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dac.c
  * @brief   This file provides code for the configuration
  *          of the DAC instances.
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
#include "dac.h"

/* USER CODE BEGIN 0 */
#include "../../SYSTEM/delay/delay.h"
#include "adc.h"
#include "math.h"
#include "tim.h"
uint16_t g_dac_sin_buf[4096];  
/* USER CODE END 0 */

DAC_HandleTypeDef hdac;

/* DAC init function */
void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */

  /* USER CODE END DAC_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC_Init 1 */

  /* USER CODE END DAC_Init 1 */

  /** DAC Initialization
  */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC_Init 2 */

  /* USER CODE END DAC_Init 2 */

}

void HAL_DAC_MspInit(DAC_HandleTypeDef* dacHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(dacHandle->Instance==DAC)
  {
  /* USER CODE BEGIN DAC_MspInit 0 */

  /* USER CODE END DAC_MspInit 0 */
    /* DAC clock enable */
    __HAL_RCC_DAC_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**DAC GPIO Configuration
    PA4     ------> DAC_OUT1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN DAC_MspInit 1 */

  /* USER CODE END DAC_MspInit 1 */
  }
}

void HAL_DAC_MspDeInit(DAC_HandleTypeDef* dacHandle)
{

  if(dacHandle->Instance==DAC)
  {
  /* USER CODE BEGIN DAC_MspDeInit 0 */

  /* USER CODE END DAC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_DAC_CLK_DISABLE();

    /**DAC GPIO Configuration
    PA4     ------> DAC_OUT1
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);

  /* USER CODE BEGIN DAC_MspDeInit 1 */

  /* USER CODE END DAC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */



/**
 * @brief       设置DAC_OUT1输出三角波
 *   @note      输出频率 ≈ 1000 / (dt * samples) Khz, 不过在dt较小的时候,比如小于5us时, 由于delay_us
 *              本身就不准了(调用函数,计算等都需要时间,延时很小的时候,这些时间会影响到延时), 频率会偏小.
 *
 * @param       maxval : 最大值(0 < maxval < 4096), (maxval + 1)必须大于等于samples/2
 * @param       dt     : 每个采样点的延时时间(单位: us)
 * @param       samples: 采样点的个数, samples必须小于等于(maxval + 1) * 2 , 且maxval不能等于0
 * @param       n      : 输出波形个数,0~65535
 *
 * @retval      无
 */
void dac_triangular_wave(uint16_t maxval, uint16_t dt, uint16_t samples, uint16_t n)
{
    uint16_t i, j;
    float incval;                           /* 递增量 */
    float Curval;                           /* 当前值 */

    if((maxval + 1) <= samples)return ;     /* 数据不合法 */

    incval = (maxval + 1) / (samples / 2);  /* 计算递增量 */

    for(j = 0; j < n; j++)
    {
        Curval = 0;
        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, Curval);    /* 先输出0 */
        for(i = 0; i < (samples / 2); i++)  /* 输出上升沿 */
        {
            Curval  +=  incval;             /* 新的输出值 */
            HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, Curval);
            delay_us(dt);
        }
        for(i = 0; i < (samples / 2); i++)  /* 输出下降沿 */
        {
            Curval  -=  incval;             /* 新的输出值 */
            HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, Curval);
            delay_us(dt);
        }
    }
}

/**
 * @brief       产生正弦波函序列
 *   @note      需保证: maxval > samples/2
 *
 * @param       maxval : 最大值(0 < maxval < 2048)
 * @param       samples: 采样点的个数
 *
 * @retval      无
 */
void dac_creat_sin_buf(uint16_t maxval, uint16_t samples)
{
    uint8_t i;
    float inc = (2 * 3.1415962) / samples; /* 计算增量（一个周期DAC_SIN_BUF个点）*/
    float outdata = 0;

    for (i = 0; i < samples; i++)
    {
        outdata = maxval * (1 + sin(inc * i)); /* 计算以dots个点为周期的每个点的值，放大maxval倍，并偏移到正数区域 */
        if (outdata > 4095)
            outdata = 4095; /* 上限限定 */
        //g_dac_sin_buf[i] = outdata;
        HAL_DAC_SetValue(&hdac,DAC_CHANNEL_1,DAC_ALIGN_12B_R,outdata);
    }
}
/* USER CODE END 1 */
