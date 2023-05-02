/**
  *********************************************************************************************
  * NAME OF THE FILE : l298n.h
  * BRIEF INFORMATION: Drives L298N DC Motor Driver
  * 				   Timer Channels MUST be in following order: A=1 to D=4
  *
  * Copyright (c) 2023 Lee Geon-goo.
  * All rights reserved.
  *
  * This file is part of catCareBot.
  *
  *********************************************************************************************
  */

#ifndef L298N_H
#define L298N_H

#include "main.h"

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

/* definitions */
// DO NOT EDIT
#define L298N_MAX_SPD 100

#define L298N_STOP 0
#define L298N_CW 1
#define L298N_CCW 2

#define L298N_MOTOR_A 0
#define L298N_MOTOR_B 1

// edit here if system configuration is changed
#define L298N_IN_PORT GPIOB
#define L298N_IN_1 GPIO_PIN_4
#define L298N_IN_2 GPIO_PIN_5
#define L298N_IN_3 GPIO_PIN_6
#define L298N_IN_4 GPIO_PIN_7

/* exported struct */
struct L298nStats {
	uint8_t ena;
	uint8_t rotA;
	uint8_t rotB;
	uint8_t spdA;
	uint8_t spdB;
};

/* exported vars */

/* exported func prototypes */
void l298n_setHandle(TIM_HandleTypeDef* ph);
void l298n_init();
void l298n_enable(); // enable motor operation. This starts PWM generation
void l298n_disable(); // implies setRotation( , STOP): disable motor operation. This stops PWM generation
void l298n_setSpeed(uint8_t motorNum, uint8_t spd); // speed scale: 0(stop) to 100(max.)
void l298n_setRotation(uint8_t motorNum, uint8_t dir); // implies setSpeed(motorNum, 0): set rotation CW or CCW.
struct L298nStats l298n_getStat(); // get status struct data

#endif
