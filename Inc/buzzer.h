/**
  *********************************************************************************************
  * NAME OF THE FILE : buzzer.h
  * BRIEF INFORMATION: Drives piezo buzzer using PWM.
  * 				   This program can drive a buzzer.
  * 				   This program use 1 general or advanced timer
  * 				   This program MUST use timer that runs at 1MHz of frequency.
  * 				   Default duty ratio is 25% and default frequency is 440Hz(A4).
  *
  * Copyright (c) 2023 Lee Geon-goo.
  * All rights reserved.
  *
  * This file is part of catCareBot.
  *
  *********************************************************************************************
  */

#ifndef BUZZER_H
#define BUZZER_H

#include "main.h"

/*
 * !!!MAKE SURE TO SET THE TIMER CLOCK FREQUENCY TO 1MHz(AFTER PSC)!!!
 * EXAMPLE: 40MHz SYSTEM CLOCK: SET PSC TO 40 TO GET 1MHz
 *
 * initialization order: setHandle -> setBuzzerCount -> init
 */

/* definitions */

/* exported struct */

/* exported typedef */
typedef enum {
	toneC2 = 15385,
	toneCS2 = 14493,
	toneD2 = 13699,
	toneDS2 = 12821,
	toneE2 = 12195,
	toneF2 = 11494,
	toneFS2 = 10753,
	toneG2 = 10204,
	toneGS2 = 9615,
	toneA2 = 9091,
	toneAS2 = 8547,
	toneB2 = 8065,
	toneC3 = 7634,
	toneCS3 = 7194,
	toneD3 = 6803,
	toneDS3 = 6410,
	toneE3 = 6061,
	toneF3 = 5714,
	toneFS3 = 5405,
	toneG3 = 5102,
	toneGS3 = 4808,
	toneA3 = 4545,
	toneAS3 = 4292,
	toneB3 = 4049,
	toneC4 = 3817,
	toneCS4 = 3597,
	toneD4 = 3401,
	toneDS4 = 3215,
	toneE4 = 3030,
	toneF4 = 2865,
	toneFS4 = 2703,
	toneG4 = 2551,
	toneGS4 = 2410,
	toneA4 = 2273,
	toneAS4 = 2146,
	toneB4 = 2024,
	toneC5 = 1912,
	toneCS5 = 1805,
	toneD5 = 1704,
	toneDS5 = 1608,
	toneE5 = 1517,
	toneF5 = 1431,
	toneFS5 = 1351,
	toneG5 = 1276,
	toneGS5 = 1203,
	toneA5 = 1136,
	toneAS5 = 1073,
	toneB5 = 1012,
	toneC6 = 955,
	toneCS6 = 902,
	toneD6 = 851,
	toneDS6 = 803,
	toneE6 = 758,
	toneF6 = 716,
	toneFS6 = 678,
	toneG6 = 638,
	toneGS6 = 602,
	toneA6 = 568,
	toneAS6 = 536,
	toneB6 = 506,
	toneC7 = 478,
	toneCS7 = 451,
	toneD7 = 426,
	toneDS7 = 402,
	toneE7 = 379,
	toneF7 = 358,
	toneFS7 = 338,
	toneG7 = 319,
	toneGS7 = 301,
	toneA7 = 284,
	toneAS7 = 268,
	toneB7 = 253,
	toneC8 = 239,
	toneCS8 = 225,
	toneD8 = 213,
	toneDS8 = 201,
	toneE8 = 190,
	toneF8 = 179,
	toneFS8 = 169,
	toneG8 = 159,
	toneGS8 = 150,
	toneA8 = 142,
	toneAS8 = 134,
	toneB8 = 127
} buzzerToneARRvalTypeDef;

/* exported vars */

/* exported func prototypes */
void buzzer_setHandle(TIM_HandleTypeDef* ph);
void buzzer_init();
void buzzer_mute(); // mute buzzer and stop PWM generation.
void buzzer_unmute(); // un-mute buzzer by starting PWM generation.
void buzzer_setTone(buzzerToneARRvalTypeDef toneCode);
void buzzer_setFreq(uint16_t freq); // frequency range: 60 <= freq <= 10000
void buzzer_setDuty(uint8_t dutyRatio); // set Duty ratio. initial value is 50.
										// duty ratio value is unaffected by other functions except for init.
										// duty ratio range: 5 < duty < 50

#endif
