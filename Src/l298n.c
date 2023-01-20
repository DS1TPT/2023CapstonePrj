/**
  *********************************************************************************************
  * NAME OF THE FILE : l298n.c
  * BRIEF INFORMATION: Driver SW: L298N DC Motor Driver
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
#include "l298n.h"

struct L298nStats L298Nstat;
uint16_t spdMultr;
uint16_t spd16a;
uint16_t spd16b;

void l298n_init() {
	// init status struct
	L298Nstat.ena = FALSE;
	L298Nstat.rotA = L298N_STOP;
	L298Nstat.rotB = L298N_STOP;
	L298Nstat.spdA = 0;
	L298Nstat.spdB = 0;
	spd16a = 0;
	spd16b = 0;

	// init GPIO
	HAL_GPIO_WritePin(L298N_IN_PORT, L298N_IN_1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(L298N_IN_PORT, L298N_IN_2, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(L298N_IN_PORT, L298N_IN_3, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(L298N_IN_PORT, L298N_IN_4, GPIO_PIN_RESET);

	// calculate timer period
	spdMultr = (uint16_t)(L298N_TIM->ARR / 100);

	// init PWM: set to LOW
	L298N_TIM->CCR1 = 0;
	L298N_TIM->CCR2 = 0;
}

void l298n_enable() { // enable motor operation. This starts PWM generation.
	if (L298Nstat.ena == TRUE) return;
	HAL_TIM_PWM_START(L298N_TIM_HANDLE, TIM_CHANNEL_1);
	HAL_TIM_PWM_START(L298N_TIM_HANDLE, TIM_CHANNEL_2);
	L298Nstat.ena = TRUE;
}

void l298n_disable() { // implies setRotation( , STOP): disable motor operation. This stops PWM generation.
	if (L298Nstat.ena == FALSE) return;

	l298n_setRotation(L298N_MOTOR_A, L298N_STOP);
	l298n_setRotation(L298N_MOTOR_B, L298N_STOP);
	HAL_TIM_PWM_STOP(L298N_TIM_HANDLE, TIM_CHANNEL_1);
	HAL_TIM_PWM_STOP(L298N_TIM_HANDLE, TIM_CHANNEL_2);
	L298Nstat.ena = FALSE;
}

void l298n_setSpeed(uint8_t motorNum, uint8_t spd) { // speed scale: 0(stop) to 100(max.)
	if (L298Nstat.ena == FALSE) return;
	if (motorNum > L298N_MOTOR_B) return;
	if (spd > 100) return; // limit max inp val to 100

	else if (motorNum == L298N_MOTOR_A) {
		L298Nstat.spdA = spd;
		spd16a = (uint16_t)(spd * spdMultr);
		L298N_TIM->CCR1 = (uint32_t)spd16a;
	}
	else if (motorNum == L298N_MOTOR_B) {
		L298Nstat.spdB = spd;
		spd16b = (uint16_t)(spd * spdMultr);
		L298N_TIM->CCR2 = (uint32_t)spd16b;
	}
}

void l298n_setRotation(uint8_t motorNum, uint8_t dir) { // implies setSpeed(motorNum, 0): set rotation CW or CCW.
	if (L298Nstat.ena == FALSE) return;
	if (motorNum > L298N_MOTOR_B) return;

	l298n_setSpeed(motorNum, 0);
	if (motorNum == L298N_MOTOR_A) {
		switch (dir) {
		case L298N_STOP:
			HAL_GPIO_WritePin(L298N_IN_PORT, L298N_IN_1, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(L298N_IN_PORT, L298N_IN_2, GPIO_PIN_RESET);
			L298Nstat.rotA = L298N_STOP;
			break;
		case L298N_CW:
			HAL_GPIO_WritePin(L298N_IN_PORT, L298N_IN_1, GPIO_PIN_SET);
			HAL_GPIO_WritePin(L298N_IN_PORT, L298N_IN_2, GPIO_PIN_RESET);
			L298Nstat.rotA = L298N_CW;
			break;
		case L298N_CCW:
			HAL_GPIO_WritePin(L298N_IN_PORT, L298N_IN_1, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(L298N_IN_PORT, L298N_IN_2, GPIO_PIN_SET);
			L298Nstat.rotA = L298N_CCW;
			break;
		}
	}
	else if (motorNum == L298N_MOTOR_B) {
		switch (dir) {
		case L298N_STOP:
			HAL_GPIO_WritePin(L298N_IN_PORT, L298N_IN_3, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(L298N_IN_PORT, L298N_IN_4, GPIO_PIN_RESET);
			L298Nstat.rotB = L298N_STOP;
			break;
		case L298N_CW:
			HAL_GPIO_WritePin(L298N_IN_PORT, L298N_IN_3, GPIO_PIN_SET);
			HAL_GPIO_WritePin(L298N_IN_PORT, L298N_IN_4, GPIO_PIN_RESET);
			L298Nstat.rotB = L298N_CW;
			break;
		case L298N_CCW:
			HAL_GPIO_WritePin(L298N_IN_PORT, L298N_IN_3, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(L298N_IN_PORT, L298N_IN_4, GPIO_PIN_SET);
			L298Nstat.rotB = L298N_CCW;
			break;
		}
	}
}

struct L298nStats l298n_getStat() { // get status struct data
	return L298Nstat;
}
