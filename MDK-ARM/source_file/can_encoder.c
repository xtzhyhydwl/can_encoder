/*
 * @Author: xiayuan 1137542776@qq.com
 * @Date: 2024-05-10 20:02:07
 * @LastEditors: xiayuan 1137542776@qq.com
 * @LastEditTime: 2024-05-12 17:32:19
 * @FilePath: \MDK-ARM\source_file\can_encoder.c
 * @Description: 
 * 
 * Copyright (c) 2024 by UESTC_LIMITI, All Rights Reserved. 
 */
#include "can_encoder.h"

encoder_all_status_t all_status = {0};

void paramter_init (void) {
  encoder_all_status_t* s = &all_status;
  uint32_t arr_value = 0;

	s->FDCAN_BAUDRATE = BAUDRATE_1M;
	s->tx_freq = 1000;
	s->internal_id = 0x01;

	if (s->internal_id > 0xff) {
		s->internal_id = 0x01;
	}
	else if (s->internal_id = 0x00) {
		s->internal_id = 0x01;
	}

	arr_value = (uint32_t)((1000.0f / s->tx_freq) * 1000.0f);
	if (arr_value > ARR_MAXIUM_VALUE) {
		arr_value = ARR_MAXIUM_VALUE;
	}
	else if (arr_value < ARR_MINIUM_VALUE) {
		arr_value < ARR_MINIUM_VALUE;
	}
	TIM15->ARR = arr_value;
  HAL_TIM_Base_Start_IT(&htim15);
  HAL_TIM_Base_Start_IT(&htim16);

	s->need_store_parameter = false;

}

void paramter_deinit (void) {
  HAL_TIM_Base_Stop_IT(&htim15);
  HAL_TIM_Base_Stop_IT(&htim16);
	HAL_FDCAN_DeInit(&hfdcan1);
}

void parameter_reinit (void) {
  encoder_all_status_t* s = &all_status;
  uint32_t arr_value = 0;

	// freq change
	arr_value = (uint32_t)((1000.0f / s->tx_freq) * 1000.0f);
	if (arr_value > ARR_MAXIUM_VALUE) {
		arr_value = ARR_MAXIUM_VALUE;
	}
	else if (arr_value < ARR_MINIUM_VALUE) {
		arr_value < ARR_MINIUM_VALUE;
	}
	TIM15->ARR = arr_value;

	// fdcan baudrate change
	if (__fdcan_parameter_reinit() == HAL_ERROR) {
		Error_Handler();
	}

	s->need_store_parameter = true;

}

static HAL_StatusTypeDef __fdcan_parameter_reinit (void) {
	uint32_t baudrate = 0;
	switch (all_status.FDCAN_BAUDRATE)
	{
	case BAUDRATE_1M:
		baudrate = 1e6;
		break;
	
	case BAUDRATE_500k:
		baudrate = 5e5;
		break;
	
	case FDCAN_BAUDRATE_250k:
		baudrate = 2.5e5;
		break;
	
	default:
		return HAL_ERROR;
		break;
	}
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_FD_BRS;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = ENABLE;
  hfdcan1.Init.TransmitPause = ENABLE;
  hfdcan1.Init.ProtocolException = ENABLE;
  hfdcan1.Init.NominalPrescaler = (int)(1e6 / baudrate) ;
  hfdcan1.Init.NominalSyncJumpWidth = 16;
  hfdcan1.Init.NominalTimeSeg1 = 110;
  hfdcan1.Init.NominalTimeSeg2 = 49;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 16;
  hfdcan1.Init.DataTimeSeg1 = 20;
  hfdcan1.Init.DataTimeSeg2 = 11;
  hfdcan1.Init.StdFiltersNbr = 1;
  hfdcan1.Init.ExtFiltersNbr = 1;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }

	FDCAN_Init(&hfdcan1);
	return HAL_OK;
}
 
// void as5047_data_transform (void) {
    
// }

/**
 * 设备id映射到stdid：
 * 
 * | bit 15 | bit 14 | bit 13 | bit 12 | bit 11 | bit 10 | bit 9  | bit 8  | bit 7  | bit 6  | bit 5  | bit 4  | bit 3  | bit 2  | bit 1  | bit 0  |
 * |  res   |  res   |  res   |  res   |  res   | sID 6  | sID 5  | sID 4  | sID 3  | sID 2  | sID 1  | sID 0  | CMD 3  | CMD 2  | CMD 1  | CMD 0  |
 * 
 * 
 * internal_id：sID 范围 1~127
 * CMD发送/解析：CMD 范围 0~7
 */
void send_angle (void) {
	uint32_t stdid = COMMAND_SEND_ANGLE;
	stdid |= all_status.internal_id << 4;
	FDCAN_SendData(&hfdcan1, (uint8_t*)(&(as5047[0].angle_now)), stdid, 4);
}

void comman_decoded (uint32_t stdid, uint8_t* rx_data) {
	encoder_all_status_t* s = &all_status;
	int32_t command = stdid & 0x000f;
	bool parameter_changed = false;
	switch (command) {
	case COMMAND_CHANGE_FREQ: 
		memcpy(s->tx_freq, rx_data, 4);
		parameter_changed = true;
		break;

	case COMMAND_CHANGE_BAUDRATE: 
		memcpy(s->FDCAN_BAUDRATE, rx_data, 4);
		parameter_changed = true;
		break;

	case COMMAND_CHANGE_FDCAN_ID: 
		parameter_changed = true;
		break;
	}
	if (parameter_changed) {
		status_deinit();

	}
}



