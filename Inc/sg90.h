/**
  *********************************************************************************************
  * NAME OF THE FILE : sg90.h
  * BRIEF INFORMATION: Drives SG90 servo motor
  * 				   This program can control up to 4 SG90s with timer 1.
  * 				   Timer Channels MUST be in following order: A=1 to D=4
  *
  * Copyright (c) 2023 Lee Geon-goo.
  * All rights reserved.
  *
  * This file is part of catCareBot.
  *
  *********************************************************************************************
  */

#ifndef SG90_H
#define SG90_H

#include "main.h"

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

/* definitions */
// DO NOT EDIT
#define SG90_MOTOR_A 0
#define SG90_MOTOR_B 1
#define SG90_MOTOR_C 2
#define SG90_MOTOR_D 3
#define SG90_MIN_DUTY 5
#define SG90_MAX_DUTY 10

// edit here if system configuration is changed
#define SG90_MOTOR_CNT 1 // order: A B C D. ex: MOTOR CNT == 2 then A and B will be used

/* exported struct */
struct SG90Stats {
	uint8_t ena[SG90_MOTOR_CNT];
	uint8_t angle[SG90_MOTOR_CNT];
};

/* exported vars */

/* exported func prototypes */
void sg90_setHandle(TIM_HandleTypeDef* ph);
void sg90_init();
void sg90_enable(uint8_t motorNum, uint8_t angle); // start giving PWM signal
void sg90_disable(uint8_t motorNum); // disable motor by stop giving PWM signal
void sg90_setAngle(uint8_t motorNum, uint8_t angle); // set angle
struct SG90Stats sg90_getStat(uint8_t motor); // get status struct data

#endif
