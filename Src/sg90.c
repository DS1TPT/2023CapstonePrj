/**
  *********************************************************************************************
  * NAME OF THE FILE : sg90.c
  * BRIEF INFORMATION: Driver SW: SG90 servo motor
  * 				   This program can control up to 4 SG90s with timer 1.
  *
  * !ATTENTION!
  *
  * Copyright (c) 2023 Lee Geon-goo.
  * All rights reserved.
  *
  * This file is part of catCareBot.
  * catCareBot is free software: you can redistribute it and/or modify it under the
  * terms of the GNU General Public License as published by the Free Software Foundation,
  * either version 3 of the License, or (at your option) any later version.
  *
  * catCareBot is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
  * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  * See the GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License along with catCareBot.
  * If not, see <https://www.gnu.org/licenses/>.
  *
  *********************************************************************************************
  */

#include "main.h"
#include "sg90.h"

struct SG90Stats SG;
float angleMultr;
uint16_t CCRmin;
uint16_t CCRmax;

void sg90_init() {
	CCRmin = (uint16_t)(SG90_TIM->ARR * SG90_MIN_DUTY / 100);
	CCRmax = (uint16_t)(SG90_TIM->ARR * SG90_MAX_DUTY / 100);
	angleMultr = (CCRmax - CCRmin) / 180.0;
	SG90_TIM->CCR1 = (uint32_t)CCRmin;
	SG90_TIM->CCR2 = (uint32_t)CCRmin;
	SG90_TIM->CCR3 = (uint32_t)CCRmin;
	SG90_TIM->CCR4 = (uint32_t)CCRmin;

	for (int i = 0; i < SG90_MOTOR_CNT; i++) {
		SG.angle[i] = 0;
		SG.ena[i] = 0;
	}
}

void sg90_enable(uint8_t motorNum, uint8_t angle) { // start giving PWM signal
	if (motorNum >= SG90_MOTOR_CNT) return;
	else if (SG.ena[motorNum] == TRUE) return;

	switch (motorNum) {
	case SG90_MOTOR_A:
		HAL_TIM_PWM_START(SG90_TIM_HANDLE, TIM_CHANNEL_1);
		ena[SG90_MOTOR_A] = 1;
		break;
	case SG90_MOTOR_B:
		HAL_TIM_PWM_START(SG90_TIM_HANDLE, TIM_CHANNEL_2);
		ena[SG90_MOTOR_B] = 1;
		break;
	case SG90_MOTOR_C:
		HAL_TIM_PWM_START(SG90_TIM_HANDLE, TIM_CHANNEL_3);
		ena[SG90_MOTOR_C] = 1;
		break;
	case SG90_MOTOR_D:
		HAL_TIM_PWM_START(SG90_TIM_HANDLE, TIM_CHANNEL_4);
		ena[SG90_MOTOR_D] = 1;
		break;
	}

	sg90_setAngle(motorNum, angle);
}

void sg90_disable(uint8_t motorNum) { // disable motor by stop giving PWM signal
	if (motorNum >= SG90_MOTOR_CNT) return;
	else if (SG.ena[motorNum] == FALSE) return;

	switch (motorNum) {
	case SG90_MOTOR_A:
		HAL_TIM_PWM_STOP(SG90_TIM_HANDLE, TIM_CHANNEL_1);
		ena[SG90_MOTOR_A] = 0;
		break;
	case SG90_MOTOR_B:
		HAL_TIM_PWM_STOP(SG90_TIM_HANDLE, TIM_CHANNEL_2);
		ena[SG90_MOTOR_B] = 0;
		break;
	case SG90_MOTOR_C:
		HAL_TIM_PWM_STOP(SG90_TIM_HANDLE, TIM_CHANNEL_3);
		ena[SG90_MOTOR_C] = 0;
		break;
	case SG90_MOTOR_D:
		HAL_TIM_PWM_STOP(SG90_TIM_HANDLE, TIM_CHANNEL_4);
		ena[SG90_MOTOR_D] = 0;
		break;
	}
}

void sg90_setAngle(uint8_t motorNum, uint8_t angle) { // implies setSpeed(motorNum, 0): set rotation CW or CCW.
	if (motorNum >= SG90_MOTOR_CNT) return;
	else if (SG.ena[motorNum] == FALSE) return;

	uint32_t ccrval = (uint32_t)(CCRmin + (uint16_t)((float)angle * angleMultr));

	switch (motorNum) {
	case SG90_MOTOR_A:
		SG90_TIM->CCR1 = ccrval;
		SG.angle[SG90_MOTOR_A] = angle;
		break;
	case SG90_MOTOR_B:
		SG90_TIM->CCR2 = ccrval;
		SG.angle[SG90_MOTOR_B] = angle;
		break;
	case SG90_MOTOR_C:
		SG90_TIM->CCR3 = ccrval;
		SG.angle[SG90_MOTOR_C] = angle;
		break;
	case SG90_MOTOR_D:
		SG90_TIM->CCR4 = ccrval;
		SG.angle[SG90_MOTOR_D] = angle;
		break;
	}
}

struct SG90Stats sg90_getStat(motor) { // get status struct data
	return SG;
}
