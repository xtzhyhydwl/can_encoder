/*
 * @Author: xiayuan 1137542776@qq.com
 * @Date: 2024-05-10 20:02:30
 * @LastEditors: xiayuan 1137542776@qq.com
 * @LastEditTime: 2024-05-12 14:56:09
 * @FilePath: \MDK-ARM\source_file\can_encoder.h
 * @Description: 
 * 
 * Copyright (c) 2024 by UESTC_LIMITI, All Rights Reserved. 
 */
#ifndef __CAN_ENCODER_H__
#define __CAN_ENCODER_H__

#include "AS5047.h"
#include "main.h"
#include "fdcan_bsp.h"
#include "tim.h"
#include "stdbool.h"

#define ARR_MINIUM_VALUE 200
#define ARR_MAXIUM_VALUE 1000*999

typedef enum {
	BAUDRATE_1M = 0,
	BAUDRATE_500k = 1,
	FDCAN_BAUDRATE_250k = 2,
} FDCAN_BAUDRATE_t;

typedef enum {
	COMMAND_CHANGE_FREQ = 0,
	COMMAND_CHANGE_BAUDRATE = 1,
	COMMAND_CHANGE_FDCAN_ID = 2,
	COMMAND_SEND_ANGLE = 3,
} COMMAND_t;

typedef struct {
  uint32_t tx_freq;
  FDCAN_BAUDRATE_t FDCAN_BAUDRATE;
	uint8_t internal_id;
	bool need_store_parameter;
} encoder_all_status_t;

static HAL_StatusTypeDef __fdcan_parameter_reinit (void);

void paramter_init (void);
void paramter_deinit (void);
void parameter_reinit (void);
void send_angle (void);
void comman_decoded (uint32_t stdid, uint8_t* rx_data);
#endif
