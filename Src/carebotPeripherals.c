/**
  *********************************************************************************************
  * NAME OF THE FILE : carebotPeripherals.c
  * BRIEF INFORMATION: drive peripheral devices
  *
  * Copyright (c) 2023 Lee Geon-goo.
  * All rights reserved.
  *
  * This file is part of catCareBot.
  *
  *********************************************************************************************
  */

#include "main.h"
#include "carebotPeripherals.h"
#include <math.h> // to use pow()

static ADC_HandleTypeDef* pAdcHandle;
static uint32_t adcDta8 = 0;
static float distCM = 0.0; // F411RE has single precision FPU
static HAL_StatusTypeDef halStat;

void periph_setHandle(ADC_HandleTypeDef* ph) {
	pAdcHandle = ph;
}

void periph_init() {
	HAL_GPIO_WritePin(LASER_PORT, LASER_PIN, GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
	HAL_ADC_Start(pAdcHandle);
}

void periph_laser_on() {
	HAL_GPIO_WritePin(LASER_PORT, LASER_PIN, GPIO_PIN_SET);
}

void periph_laser_off() {
	HAL_GPIO_WritePin(LASER_PORT, LASER_PIN, GPIO_PIN_RESET);
}

int periph_irSnsrChk(int mode) {
	halStat = HAL_ADC_PollForConversion(pAdcHandle, IR_SNSR_POLL_TIMEOUT);
	if (halStat != HAL_OK) // couldn't poll
		return IR_SNSR_ERR;
	adcDta8 = HAL_ADC_GetValue(pAdcHandle); // get data
	/* equation for GP2Y0A02 (y: voltage, x = cm)
	 * y = 32.467x^-0.8504
	 * x = 59.88676548 / (y^1.17591721)
	 * range of x: 15cm(min) or 20cm(typ) to 150cm
	*/

	distCM = 59.88676548 / pow((float)adcDta8, 1.17591721); // calculate distance

	switch (mode) { // decide near/far according to pre-set distance of a mode
	case IR_SNSR_MODE_OP:
		if (distCM <= IR_SNSR_TRIG_DIST_OP) return IR_SNSR_NEAR;
		else return IR_SNSR_FAR;
		break;
	case IR_SNSR_MODE_FIND:
		if (distCM <= IR_SNSR_TRIG_DIST_FIND) return IR_SNSR_NEAR;
		else return IR_SNSR_FAR;
		break;
	case IR_SNSR_MODE_SNACK:
		if (distCM <= IR_SNSR_TRIG_DIST_SNACK) return IR_SNSR_NEAR;
		else return IR_SNSR_FAR;
		break;
	}
}
