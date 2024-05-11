/*
 * @Author: xiayuan 1137542776@qq.com
 * @Date: 2024-03-03 19:59:54
 * @LastEditors: xiayuan 1137542776@qq.com
 * @LastEditTime: 2024-03-04 23:09:15
 * @FilePath: \MDK-ARM\all\AS5047.h
 * @Description: 
 * v1.0 实现基本的读取角度功能，可以开启均值滤波 3.4.2024
 * 
 * 使用注意事项：
 * 1. 支持一主机多从机的SPI模式，说白了就是一个SPI可以读取多个编码器，前提是配置好CS脚和结构体
 * 2. 结构体配置中最重要的是目标spi和CS脚，多用多配，记得调用轮询获取角度的函数
 * 3. as5047是一个单圈绝对值编码器，软件多圈可以自己写（v1.0未添加）
 * Copyright (c) 2024 by UESTC_LIMITI, All Rights Reserved. 
 */
#ifndef __AS5047_H__
#define __AS5047_H__

#include "main.h"
#include "spi.h"
#include "stdbool.h"
#include "string.h"
#include "stm32g4xx_hal_gpio.h"  //注意这个头文件可能需要改 

#define CS_PORT GPIOB
#define CS_PIN GPIO_PIN_11

#define NOP           0x0000
#define ERRFL         0x0001
#define PROG          0x0003
#define DIAAGC        0x3FFC
#define MAG           0x3FFD
#define ANGLEUNC      0x3FFE    //无动态角度误差补偿的测量角度
#define ANGLECOM      0x3FFF    //带有动态角度误差补偿的测量角度

#define ZPOSM         0x0016
#define ZPOSL         0x0017
#define SETTINGS1     0x0018
#define SETTINGS2     0x0019

#define FILTER_WINDOW_SIZE 3

typedef struct {
    GPIO_TypeDef* cs_port;
    uint16_t cs_pin;
}hspi_cs_t;

typedef struct 
{
    uint16_t datas[FILTER_WINDOW_SIZE];
    uint32_t count;
    uint32_t sum;
}__as5047_mean_filter_t;

typedef struct {
    float angle_now;
    uint16_t raw_data;
    hspi_cs_t cs;
    SPI_HandleTypeDef *hspi;
    bool use_filter;
    __as5047_mean_filter_t mean_fliter;
}as5047_t;  //as5047配置和数据存放的结构体

static float __as5047_angle_transform (uint16_t count);
static uint16_t __parity_bit_calculate (uint16_t data_2_cal);
static uint16_t __as5047_mean_filter(uint16_t now_data, __as5047_mean_filter_t* filter);

uint16_t AS5047_read_reg (uint16_t add, as5047_t* as5047);
uint16_t SPI_ReadWrite_OneByte (uint16_t txdata, SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_port, uint16_t cs_pin);

void as5047_read_angle_routine (as5047_t* as5047);  //读角度调用这个，用之前先看库文件说明
void usDelayTest (void);
void usDelayOptimize (void);
void Delay_us (uint32_t Delay);


extern as5047_t as5047[4];

#endif
