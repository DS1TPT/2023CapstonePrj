/**
  *********************************************************************************************
  * NAME OF THE FILE : buzzer.c
  * BRIEF INFORMATION: Drives piezo buzzer using PWM.
  * 				   This program can drive a buzzer.
  * 				   This program use 1 general or advanced timer
  * 				   This program MUST use timer that runs at 1MHz of frequency.
  *
  * Copyright (c) 2023 Lee Geon-goo.
  * All rights reserved.
  *
  * This file is part of catCareBot.
  *
  *********************************************************************************************
  */

#include "buzzer.h"

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

static TIM_HandleTypeDef* pTimHandle = NULL;
static TIM_TypeDef* pTimInstance = NULL;
static uint8_t duty;
static _Bool timEna = FALSE;
static _Bool pwmEna = FALSE;
static _Bool initStat = FALSE;

void buzzer_setHandle(TIM_HandleTypeDef* ph) {
	pTimHandle = ph;
	pTimInstance = ph->Instance;
}

void buzzer_init() {
	if (pTimHandle == NULL) return;
	duty = 25;
	pwmEna = FALSE;

	if (timEna == FALSE) {
		HAL_TIM_Base_Start_IT(pTimHandle);
		timEna = TRUE;
	}

	// init PWM: set to 440Hz 25%
	pTimInstance->ARR = 2273;
	pTimInstance->CCR1 = pTimInstance->ARR / 4;

	initStat = TRUE;
}

void buzzer_mute() {
	if (initStat == FALSE || pwmEna == FALSE) return;
	HAL_TIM_PWM_Stop(pTimHandle, TIM_CHANNEL_1);
	pwmEna = FALSE;
}

void buzzer_unmute() {
	if (initStat == FALSE || pwmEna == TRUE) return;
	HAL_TIM_PWM_Start(pTimHandle, TIM_CHANNEL_1);
	pwmEna = TRUE;
}

void buzzer_setTone(buzzerToneARRvalTypeDef toneCode) {
	if (initStat == FALSE) return;
	pTimInstance->ARR = toneCode;
	pTimInstance->CCR1 = (uint16_t)((float)pTimInstance->ARR * ((float)duty / 100.0));
}

void buzzer_setFreq(uint16_t freq) {
	if (freq > 10000 || freq < 60) return;
	// arr = 1,000,000 / freq
	pTimInstance->ARR = (uint16_t)(1000000 / (uint16_t)((float)freq + 0.5));
	pTimInstance->CCR1 = (uint16_t)((float)pTimInstance->ARR * ((float)duty / 100.0));
}

void buzzer_setDuty(uint8_t dutyRatio) {
	if (dutyRatio < 5 || dutyRatio > 50) return;
	duty = dutyRatio;
	pTimInstance->CCR1 = (uint16_t)((float)pTimInstance->ARR * ((float)duty / 100.0));
}
