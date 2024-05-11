/*
 * @Author: xiayuan 1137542776@qq.com
 * @Date: 2024-03-03 19:59:54
 * @LastEditors: xiayuan 1137542776@qq.com
 * @LastEditTime: 2024-05-10 21:44:49
 * @FilePath: \MDK-ARM\source_file\AS5047.c
 * @Description: 
 * v1.0 实现基本的读取角度功能，可以开启均值滤波 3.4.2024
 * 
 * 使用注意事项：
 * 1. 支持一主机多从机的SPI模式，说白了就是一个SPI可以读取多个编码器，前提是配置好CS脚和结构体
 * 2. 结构体配置中最重要的是目标spi和CS脚，多用多配，记得调用轮询获取角度的函数
 * 3. as5047是一个单圈绝对值编码器，软件多圈可以自己写（v1.0未添加）
 * Copyright (c) 2024 by UESTC_LIMITI, All Rights Reserved. 
 */

#include "AS5047.h"

//一个板子需要读多个数据，在这里扩大数组和定义SPI、CS脚
as5047_t as5047[4] = {  
{
    .cs = {GPIOA, GPIO_PIN_15},
    .hspi = &hspi3,
	.use_filter = true,
},
};

static float __as5047_angle_transform (uint16_t count) {
    return ((float)count / 16383 ) * 360.0f;
}

/**
 * @description: as5047要求的偶校验
 * @param {uint16_t} data_2_cal 发送数据之前要计算校验位
 * @return {*}  算出来0或1
 */
static uint16_t __parity_bit_calculate (uint16_t data_2_cal) {
	uint16_t parity_bit_value=0;
	while(data_2_cal != 0)
	{
		parity_bit_value ^= data_2_cal; 
		data_2_cal >>=1;
	}
	return (parity_bit_value & 0x1); 
}

//由于 359->0 度存在跳变，均值滤波可能得到不正确数据，所以默认不启用滤波
/**
 * @description: 均值滤波
 * @param {uint16_t} now_data 新数据
 * @param {__as5047_mean_filter_t*} filter 均值滤波结构体
 * @return {*} 返回滤波后的数据
 */
static uint16_t __as5047_mean_filter(uint16_t now_data, __as5047_mean_filter_t* filter) {
	uint32_t size = sizeof(filter->datas)/sizeof(uint16_t);
	filter->sum -= filter->datas[filter->count];
	filter->sum += now_data;
	filter->datas[filter->count] = now_data;
	filter->count++;
	if (filter->count >= size) {
		filter->count = 0;
	}
	return (uint16_t)(filter->sum / size);
}  

/**
 * @description: SPI读写
 * @param {uint16_t} txdata 要发的东西
 * @param {SPI_HandleTypeDef*} hspi 目标spi
 * @param {GPIO_TypeDef*} cs_port 目标spi设备的cs端口
 * @param {uint16_t} cs_pin 目标spi设备的cs引脚
 * @param {uint16_t} rxdata 接收到的数据
 * @return {*}  返回rxdata
 */
uint16_t SPI_ReadWrite_OneByte (uint16_t txdata, SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_port, uint16_t cs_pin) {
	HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET); //拉低片选								
	uint16_t rxdata;
	if (HAL_SPI_TransmitReceive(hspi,(uint8_t *)&txdata,(uint8_t *)&rxdata,1,1000) != HAL_OK)
		rxdata=0;
	HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);  //拉高								
	return rxdata;
}

/**
 * @description: as5047读寄存器
 * @param {uint16_t} add 寄存器地址
 * @param {as5047_t*} as5047 目标as5047
 * @return {*} 返回除去校验位和错误位的14位有效数据
 */
uint16_t AS5047_read_reg (uint16_t add, as5047_t* as5047) {
    SPI_HandleTypeDef* target_hspi = as5047->hspi;
    GPIO_TypeDef* cs_port = as5047->cs.cs_port;
    uint16_t cs_pin = as5047->cs.cs_pin;
	uint16_t data;
	add |= 0x4000;	
	if(__parity_bit_calculate(add)==1) add=add|0x8000; 
	data=SPI_ReadWrite_OneByte(ERRFL|0x4000, target_hspi, cs_port, cs_pin);  //每次读一个错误位清除错误标志
	Delay_us(10);
	data=SPI_ReadWrite_OneByte(add, target_hspi, cs_port, cs_pin);	//接收错误位数据，发送读取寄存器数据的命令
	Delay_us(10);
	data=SPI_ReadWrite_OneByte(NOP|0x4000, target_hspi, cs_port, cs_pin);     //接收想读的寄存器的数据，发送空指令
	Delay_us(10);
	data &=0x3fff;  //除去校验位和错误位 
	return data;
}

/**
 * @description:  as5047读取角度的主函数，需要轮询调用来读取
 * @param {as5047_t*} as5047 目标as5047
 */
void as5047_read_angle_routine (as5047_t* as5047) {
    uint16_t raw_data = AS5047_read_reg(ANGLECOM, as5047);
	if (as5047->use_filter == true) {
		raw_data = __as5047_mean_filter(raw_data, &as5047->mean_fliter);
	}
    as5047->angle_now = __as5047_angle_transform(raw_data);  //读14位原始数据，然后角度转换
}

__IO float usDelayBase;
void usDelayTest(void) {
  __IO uint32_t firstms, secondms;
  __IO uint32_t counter = 0;

  firstms = HAL_GetTick()+1;
  secondms = firstms+1;

  while(uwTick!=firstms) ;

  while(uwTick!=secondms) counter++;

  usDelayBase = ((float)counter)/1000;
}

void usDelayOptimize(void) {
  __IO uint32_t firstms, secondms;
  __IO float coe = 1.0;

  firstms = HAL_GetTick();
  Delay_us(1000000) ;
  secondms = HAL_GetTick();

  coe = ((float)1000)/(secondms-firstms);
  usDelayBase = coe*usDelayBase;
}

void Delay_us(uint32_t Delay) {
  __IO uint32_t delayReg;
  __IO uint32_t usNum = (uint32_t)(Delay*usDelayBase);

  delayReg = 0;
  while(delayReg!=usNum) delayReg++;
}



