/**
  *********************************************************************************************
  * NAME OF THE FILE : l298n.h
  * BRIEF INFORMATION: L298N DC Motor Driver
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

#ifndef L298N_H
#define L298N_H

/* definitions */
#define L298N_STOP 0
#define L298N_CW 1
#define L298N_CCW 2

#define L298N_MOTOR_A 0
#define L298N_MOTOR_B 1

// edit here if system configuration is changed
#define L298N_TIM TIM3 // CAUTION: PSC, ARR, CCR use 16b val, but stm32cubeide's typedef is 32b.
const uint32_t L298N_TIM_APB_CLK = 84000000; // in MHz.
#define L298N_IN_PORT GPIOC // C0~C3
#define L298N_IN_1 GPIO_PIN_0
#define L298N_IN_2 GPIO_PIN_1
#define L298N_IN_3 GPIO_PIN_2
#define L298N_IN_4 GPIO_PIN_3
#define L298N_PWM_PORT GPIOA // A6, A7
#define L298N_PWM_A GPIO_PIN_6 // CH1
#define L298N_PWM_B GPIO_PIN_7 // CH2

/* includes */
#include "main.h"

/* exported struct */
struct L298nStats {
	uint8_t rotA;
	uint8_t rotB;
	uint8_t spdA;
	uint8_t spdB;
};

/* exported vars */

/* exported func prototypes */
void l298n_init();
void l298n_disable(); // implies setRotation( , STOP): disable motor operation
void l298n_setSpeed(uint8_t motorNum, uint8_t spd); // speed scale: 0(stop) to 255(max.)
void l298n_setRotation(uint8_t motorNum, uint8_t dir); // implies setSpeed(motorNum, 0): set rotation CW or CCW.
struct L298nStats l298n_getStat(); // get status struct data

#endif
