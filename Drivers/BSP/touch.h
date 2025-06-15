/**
 ****************************************************************************************************
 * @file        touch.h
 * @author      STM32 Oscilloscope Project
 * @version     V1.0
 * @date        2025-01-15
 * @brief       通用触摸屏驱动头文件
 *              支持常见触摸控制器: GT9147, GT911, FT5206, FT6236等
 ****************************************************************************************************
 */

#ifndef __TOUCH_H
#define __TOUCH_H

#include "main.h"

/* 触摸屏分辨率设置 */
#define TOUCH_MAX_X     480     /* 触摸屏X轴最大值 */
#define TOUCH_MAX_Y     800     /* 触摸屏Y轴最大值 (4.3寸竖屏) */

/* 触摸状态定义 */
#define TOUCH_PRESSED   1       /* 触摸按下 */
#define TOUCH_RELEASED  0       /* 触摸释放 */

/* 触摸点结构体 */
typedef struct {
    uint16_t x;                 /* X坐标 */
    uint16_t y;                 /* Y坐标 */
    uint8_t status;             /* 触摸状态 */
} touch_point_t;

/* 触摸屏驱动类型选择 */
typedef enum {
    TOUCH_TYPE_NONE = 0,        /* 无触摸 */
    TOUCH_TYPE_GT9147,          /* GT9147 (I2C) */
    TOUCH_TYPE_GT911,           /* GT911 (I2C) */
    TOUCH_TYPE_GT968_I2C,       /* GT968 (I2C) */
    TOUCH_TYPE_GT1151_I2C,      /* GT1151 (I2C) */
    TOUCH_TYPE_GT968_SPI,       /* GT968 (SPI) */
    TOUCH_TYPE_GT1151_SPI,      /* GT1151 (SPI) */
    TOUCH_TYPE_FT5206,          /* FT5206 (I2C) */
    TOUCH_TYPE_FT6236,          /* FT6236 (I2C) */
    TOUCH_TYPE_SIMULATE         /* 模拟触摸 (按键) */
} touch_type_t;

/* 触摸配置结构体 */
typedef struct {
    touch_type_t type;          /* 触摸类型 */
    uint8_t i2c_addr;           /* I2C地址 (I2C模式使用) */
    
    /* SPI引脚配置 (SPI模式使用) */
    GPIO_TypeDef* spi_sck_port; /* SPI时钟引脚端口 */
    uint16_t spi_sck_pin;       /* SPI时钟引脚 */
    GPIO_TypeDef* spi_mosi_port;/* SPI数据输出引脚端口 */
    uint16_t spi_mosi_pin;      /* SPI数据输出引脚 */
    GPIO_TypeDef* spi_miso_port;/* SPI数据输入引脚端口 */
    uint16_t spi_miso_pin;      /* SPI数据输入引脚 */
    GPIO_TypeDef* spi_cs_port;  /* SPI片选引脚端口 */
    uint16_t spi_cs_pin;        /* SPI片选引脚 */
    
    /* 通用控制引脚 */
    GPIO_TypeDef* rst_port;     /* 复位引脚端口 */
    uint16_t rst_pin;           /* 复位引脚 */
    GPIO_TypeDef* int_port;     /* 中断引脚端口 */
    uint16_t int_pin;           /* 中断引脚 */
} touch_config_t;

/* 函数声明 */
uint8_t touch_init(touch_config_t* config);
uint8_t touch_scan(touch_point_t* point);
void touch_calibrate(void);
uint8_t touch_read_point(uint16_t* x, uint16_t* y);

/* 硬件相关函数 (需要用户实现) */
void touch_reset(void);
uint8_t touch_i2c_write(uint8_t addr, uint8_t reg, uint8_t* data, uint16_t len);
uint8_t touch_i2c_read(uint8_t addr, uint8_t reg, uint8_t* data, uint16_t len);

/* SPI相关函数 */
void touch_spi_init(void);
uint8_t touch_spi_read_write(uint8_t data);
void touch_spi_cs_set(uint8_t state);

/* 常用触摸控制器配置 */
extern const touch_config_t GT9147_CONFIG;
extern const touch_config_t GT911_CONFIG;
extern const touch_config_t GT968_I2C_CONFIG;
extern const touch_config_t GT1151_I2C_CONFIG;
extern const touch_config_t GT968_SPI_CONFIG;
extern const touch_config_t GT1151_SPI_CONFIG;
extern const touch_config_t FT5206_CONFIG;
extern const touch_config_t SIMULATE_CONFIG;

#endif /* __TOUCH_H */