/**
  *********************************************************************************************
  * NAME OF THE FILE : sg90.h
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

#ifndef SG90_H
#define SG90_H

/* definitions */
// DO NOT EDIT
#define SG90_MOTOR_A 0
#define SG90_MOTOR_B 1
#define SG90_MOTOR_C 2
#define SG90_MOTOR_D 3
#define SG90_MIN_DUTY 5
#define SG90_MAX_DUTY 10

// edit here if system configuration is changed
#define SG90_TIM TIM1 // CAUTION: PSC, ARR, CCR use 16b val, but stm32cubeide's typedef is 32b.
#define SG90_TIM_HANDLE &htim1
#define SG90_MOTOR_CNT 2
#define SG90_PWM_PORT GPIOA // A8, A9
#define SG90_PWM_A GPIO_PIN_8 // CH1
#define SG90_PWM_B GPIO_PIN_9 // CH2
//#define SG90_PWM_C GPIO_PIN_ // CH3
//#define SG90_PWM_D GPIO_PIN_ // CH4

/* exported struct */
struct SG90Stats {
	uint8_t ena[SG90_MOTOR_CNT];
	uint8_t angle[SG90_MOTOR_CNT];
};

/* exported vars */

/* exported func prototypes */
void sg90_init();
void sg90_enable(uint8_t motorNum, uint8_t angle); // start giving PWM signal
void sg90_disable(uint8_t motorNum); // disable motor by stop giving PWM signal
void sg90_setAngle(uint8_t motorNum, uint8_t angle); // implies setSpeed(motorNum, 0): set rotation CW or CCW.
struct SG90Stats sg90_getStat(motor); // get status struct data

#endif
