/**
  *********************************************************************************************
  * NAME OF THE FILE : carebotCore.c
  * BRIEF INFORMATION: robot program core
  *
  * Copyright (c) 2023 Lee Geon-goo.
  * All rights reserved.
  *
  * This file is part of catCareBot.
  *
  *********************************************************************************************
  */

#include "carebotCore.h"
#include "carebotPeripherals.h"
#include "opman.h"
#include "rpicomm.h"
#include "scheduler.h"
#include "l298n.h"
#include "sg90.h"

struct SerialDta rpidta;

// system properties (editable)
const uint8_t MAN_ROT_SPD = 44; // RANGE: 6~114, EVEN NUMBER. AFFECTS AUTO SPEED
const uint8_t MAN_DRV_SPD = 88; // RANGE: 6~114, EVEN NUMBER. AFFECTS AUTO SPEED
const uint8_t SPD_ADDEND = 40; // THIS NUMBER MUST NOT EXCEED: 254 - MANUAL SPEED * 2
const uint8_t SPD_SUBTRAHEND = 10; // THIS NUMBER MUST BE LESS THAN: MANUAL SPEED / 4
const uint8_t DEF_ANG_A = 45; // default angle of toy motor, WRITE RETRACTED ANGLE
const uint8_t DEF_ANG_B = 30; // default angle of snack motor
const uint16_t OP_SNACK_RET_MOTOR_WAITING_TIME = 500;
const int32_t PATTERN_WAIT_AND_FLEE_WAIT_TIME = 20; // RANGE: 1 ~ 60, in seconds

// derived properties (NOT EDITABLE)
const uint8_t AUTO_DEF_ROT_SPD = MAN_ROT_SPD / 2;
const uint8_t AUTO_DEF_DRV_SPD = MAN_DRV_SPD / 2;
const uint8_t AUTO_MIN_ROT_SPD = (uint8_t)((float)AUTO_DEF_ROT_SPD / 2.0) - (((float)AUTO_DEF_ROT_SPD / 2.0 > 0) ? 0 : 1);
const uint8_t AUTO_MIN_DRV_SPD = (uint8_t)((float)AUTO_DEF_DRV_SPD / 2.0) - (((float)AUTO_DEF_ROT_SPD / 2.0 > 0) ? 0 : 1);
const uint8_t SPD_OVERSHOOT_ADDEND = ((AUTO_DEF_ROT_SPD > AUTO_DEF_DRV_SPD) ? (254 - AUTO_DEF_DRV_SPD * 4) : (254 - AUTO_DEF_ROT_SPD * 4));
const uint8_t TOY_ANG_DRAW = DEF_ANG_A + 90; // max angle(draw) of toy motor
const uint8_t TOY_ANG_HALF = DEF_ANG_A + 45; // half draw angle
const uint8_t TOY_ANG_RETRACT = DEF_ANG_A;
const uint8_t SNACK_ANG_RDY = DEF_ANG_B;
const uint8_t SNACK_ANG_GIVE = DEF_ANG_B + 90;

// system variables
static uint8_t flagTimeElapsed = FALSE;
//static uint8_t flagHibernate = FALSE;
static uint8_t flagAutorun = FALSE;
static uint8_t recvScheduleMode = FALSE;
static uint8_t initState = FALSE;
static uint8_t secTimEna = FALSE;

static TIM_HandleTypeDef* pTimHandle = NULL;

static void exePattern(int code) {
	uint8_t spdMultiplier = 0;
	uint8_t rotSpd, drvSpd;
	int32_t interval = 0; // seconds
	int32_t rptNum = 1;
	int32_t rptTime = 1;
	int32_t cnt = 0;

	spdMultiplier = scheduler_getSpd();
	interval = scheduler_getInterval();


	if (!flagAutorun) interval = 1;

	if (spdMultiplier) {
		rotSpd = AUTO_DEF_ROT_SPD * spdMultiplier;
		drvSpd = AUTO_DEF_DRV_SPD * spdMultiplier;
	}
	else {
		rotSpd = AUTO_MIN_ROT_SPD;
		drvSpd = AUTO_MIN_DRV_SPD;
	}

	HAL_Delay(300); // give a slight delay between patterns

	switch (code) {
	case 1: // Waltz(S-shaped route zig-zaging)
		rptNum = interval / 3;
		if (rptNum < 2) rptNum = 1; // execute at least one time
		l298n_setRotation(L298N_MOTOR_A, L298N_CCW); // initial rotation
		l298n_setRotation(L298N_MOTOR_B, L298N_CW);
		l298n_setSpeed(L298N_MOTOR_A, AUTO_DEF_ROT_SPD); // rotation speed will not be affected by speed multiplier
		l298n_setSpeed(L298N_MOTOR_B, AUTO_MIN_ROT_SPD);
		HAL_Delay(500);
		for (int32_t i32 = 0; i32 < rptNum; i32++) {
			// forward
			l298n_setSpeed(L298N_MOTOR_A, drvSpd);
			l298n_setSpeed(L298N_MOTOR_B, drvSpd);
			HAL_Delay(300);
			l298n_setSpeed(L298N_MOTOR_A, AUTO_MIN_ROT_SPD); // rotation speed will not be affected by speed multiplier
			l298n_setSpeed(L298N_MOTOR_B, AUTO_DEF_ROT_SPD);
			HAL_Delay(1000);
			l298n_setSpeed(L298N_MOTOR_A, drvSpd);
			l298n_setSpeed(L298N_MOTOR_B, drvSpd);
			HAL_Delay(300);
			l298n_setSpeed(L298N_MOTOR_A, AUTO_DEF_ROT_SPD); // rotation speed will not be affected by speed multiplier
			l298n_setSpeed(L298N_MOTOR_B, AUTO_MIN_ROT_SPD);
			HAL_Delay(1000);
		}
		break;
	case 2: // loop of Sudden accel., decel.
		rptNum = interval / 12;
		if (rptNum < 2) rptNum = 1; // execute at least one time
		for (int32_t i32 = 0; i32 < rptNum; i32++) {
			// forward
			l298n_setRotation(L298N_MOTOR_A, L298N_CCW);
			l298n_setRotation(L298N_MOTOR_B, L298N_CW);
			for (int i = 0; i < 4; i++) {
				l298n_setSpeed(L298N_MOTOR_A, drvSpd + SPD_OVERSHOOT_ADDEND);
				l298n_setSpeed(L298N_MOTOR_B, drvSpd + SPD_OVERSHOOT_ADDEND);
				HAL_Delay(200);
				l298n_setSpeed(L298N_MOTOR_A, drvSpd);
				l298n_setSpeed(L298N_MOTOR_B, drvSpd);
				HAL_Delay(300);
				l298n_setSpeed(L298N_MOTOR_A, 0);
				l298n_setSpeed(L298N_MOTOR_B, 0);
				HAL_Delay(1000);
			}
			// backward
			l298n_setRotation(L298N_MOTOR_A, L298N_CW);
			l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
			for (int i = 0; i < 4; i++) {
				l298n_setSpeed(L298N_MOTOR_A, drvSpd + SPD_OVERSHOOT_ADDEND);
				l298n_setSpeed(L298N_MOTOR_B, drvSpd + SPD_OVERSHOOT_ADDEND);
				HAL_Delay(200);
				l298n_setSpeed(L298N_MOTOR_A, drvSpd);
				l298n_setSpeed(L298N_MOTOR_B, drvSpd);
				HAL_Delay(300);
				l298n_setSpeed(L298N_MOTOR_A, 0);
				l298n_setSpeed(L298N_MOTOR_B, 0);
				HAL_Delay(1000);
			}

		}
		break;
	case 3: // crawling, left wheel forwards a little bit, right goes next, then left goes again...
		rptNum = interval / 10;
		if (rptNum < 2) rptNum = 1; /// execute at least one time
		for (int32_t i32 = 0; i32 < rptNum; i32++) {
			for (int i = 0; i < 5; i++) {
				l298n_setRotation(L298N_MOTOR_A, L298N_CCW);
				l298n_setRotation(L298N_MOTOR_B, L298N_STOP);
				l298n_setSpeed(L298N_MOTOR_A, rotSpd);
				HAL_Delay(500);
				l298n_setRotation(L298N_MOTOR_A, L298N_STOP);
				l298n_setRotation(L298N_MOTOR_B, L298N_CW);
				l298n_setSpeed(L298N_MOTOR_B, rotSpd);
				HAL_Delay(500);
			}
			for (int i = 0; i < 5; i++) {
				l298n_setRotation(L298N_MOTOR_A, L298N_CW);
				l298n_setRotation(L298N_MOTOR_B, L298N_STOP);
				l298n_setSpeed(L298N_MOTOR_A, rotSpd);
				HAL_Delay(500);
				l298n_setRotation(L298N_MOTOR_A, L298N_STOP);
				l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
				l298n_setSpeed(L298N_MOTOR_B, rotSpd);
				HAL_Delay(500);
			}
		}
		break;
	case 4: // draw circle fast
		rptTime = interval;
		if (rptTime < 2) rptTime = 10; // ensure execution
		l298n_setRotation(L298N_MOTOR_A, L298N_CCW); // right
		l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
		l298n_setSpeed(L298N_MOTOR_A, drvSpd + SPD_ADDEND);
		l298n_setSpeed(L298N_MOTOR_B, drvSpd - SPD_SUBTRAHEND);
		HAL_Delay(rptTime * 1000);
		break;
	case 5: // shake the toy left and right but doesn't go anywhere
		// this pattern will rotate the robot faster than pattern 8
		rptNum = interval;
		if (rptNum < 2) rptNum = 1; // execute at least one time
		for (int32_t i32 = 0; i32 < rptNum; i32++) {
			l298n_setRotation(L298N_MOTOR_A, L298N_CCW); // right
			l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
			l298n_setSpeed(L298N_MOTOR_A, rotSpd + SPD_OVERSHOOT_ADDEND);
			l298n_setSpeed(L298N_MOTOR_B, rotSpd + SPD_OVERSHOOT_ADDEND);
			HAL_Delay(150);
			l298n_setSpeed(L298N_MOTOR_A, rotSpd);
			l298n_setSpeed(L298N_MOTOR_B, rotSpd);
			HAL_Delay(150);
			l298n_setSpeed(L298N_MOTOR_A, 0);
			l298n_setSpeed(L298N_MOTOR_B, 0);
			HAL_Delay(100);
			l298n_setRotation(L298N_MOTOR_A, L298N_CW); // left
			l298n_setRotation(L298N_MOTOR_B, L298N_CW);
			l298n_setSpeed(L298N_MOTOR_A, rotSpd + SPD_OVERSHOOT_ADDEND);
			l298n_setSpeed(L298N_MOTOR_B, rotSpd + SPD_OVERSHOOT_ADDEND);
			HAL_Delay(150);
			l298n_setSpeed(L298N_MOTOR_A, rotSpd);
			l298n_setSpeed(L298N_MOTOR_B, rotSpd);
			HAL_Delay(150);
			l298n_setSpeed(L298N_MOTOR_A, 0);
			l298n_setSpeed(L298N_MOTOR_B, 0);
			HAL_Delay(100);
		}
		break;
	case 6: // rotate, go to somewhere else, then rotate again
		rptNum = interval / 6;
		if (rptNum < 2) rptNum = 1; // execute at least one time
		for (int32_t i32 = 0; i32 < rptNum; i32++) {
			l298n_setRotation(L298N_MOTOR_A, L298N_CCW); // right
			l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
			l298n_setSpeed(L298N_MOTOR_A, rotSpd);
			l298n_setSpeed(L298N_MOTOR_B, rotSpd);
			HAL_Delay(2000);
			l298n_setRotation(L298N_MOTOR_A, L298N_CCW); // forward
			l298n_setRotation(L298N_MOTOR_B, L298N_CW);
			l298n_setSpeed(L298N_MOTOR_A, drvSpd);
			l298n_setSpeed(L298N_MOTOR_B, drvSpd);
			HAL_Delay(1000);
			l298n_setRotation(L298N_MOTOR_A, L298N_CW); // left
			l298n_setRotation(L298N_MOTOR_B, L298N_CW);
			l298n_setSpeed(L298N_MOTOR_A, rotSpd);
			l298n_setSpeed(L298N_MOTOR_B, rotSpd);
			HAL_Delay(2000);
			l298n_setRotation(L298N_MOTOR_A, L298N_CW); // backward
			l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
			l298n_setSpeed(L298N_MOTOR_A, drvSpd);
			l298n_setSpeed(L298N_MOTOR_B, drvSpd);
			HAL_Delay(1000);
		}
		break;
	case 7: // wait until something reaches in front of IR sensor, then flee backwards
		// this pattern is not affected by interval time and it'll be executed only one time
		// if pre defined time has been elapsed, the robot will do nothing
		cnt = PATTERN_WAIT_AND_FLEE_WAIT_TIME;
		int i = 0;
		while (1) {
			if (i == 10) {
				if (cnt <= 0) break;
				i = 0;
				cnt--;
			}
			if (periph_irSnsrChk(IR_SNSR_MODE_OP) == IR_SNSR_NEAR) {
				l298n_setRotation(L298N_MOTOR_A, L298N_CW); // backward
				l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
				l298n_setSpeed(L298N_MOTOR_A, drvSpd + SPD_OVERSHOOT_ADDEND);
				l298n_setSpeed(L298N_MOTOR_B, drvSpd + SPD_OVERSHOOT_ADDEND);
				HAL_Delay(200);
				l298n_setSpeed(L298N_MOTOR_A, drvSpd);
				l298n_setSpeed(L298N_MOTOR_B, drvSpd);
				HAL_Delay(400);
				l298n_setSpeed(L298N_MOTOR_A, 0);
				l298n_setSpeed(L298N_MOTOR_B, 0);
				break;
			}
			HAL_Delay(100);
		}
		break;
	case 8: // shake the toy left and right, flee to somewhere else, then shake the toy again
		rptNum = interval / 2;
		if (rptNum < 2) rptNum = 1; // execute at least one time
		for (int32_t i32 = 0; i32 < rptNum; i32++) {
			for (int i = 0; i < 5; i++) { // shake
				l298n_setRotation(L298N_MOTOR_A, L298N_CW); // left
				l298n_setRotation(L298N_MOTOR_B, L298N_CW);
				l298n_setSpeed(L298N_MOTOR_A, rotSpd + SPD_OVERSHOOT_ADDEND / 2);
				l298n_setSpeed(L298N_MOTOR_B, rotSpd + SPD_OVERSHOOT_ADDEND / 2);
				HAL_Delay(150);
				l298n_setSpeed(L298N_MOTOR_A, rotSpd);
				l298n_setSpeed(L298N_MOTOR_B, rotSpd);
				HAL_Delay(150);
				l298n_setSpeed(L298N_MOTOR_A, 0);
				l298n_setSpeed(L298N_MOTOR_B, 0);
				HAL_Delay(100);
				l298n_setRotation(L298N_MOTOR_A, L298N_CCW); // right
				l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
				l298n_setSpeed(L298N_MOTOR_A, rotSpd + SPD_OVERSHOOT_ADDEND / 2);
				l298n_setSpeed(L298N_MOTOR_B, rotSpd + SPD_OVERSHOOT_ADDEND / 2);
				HAL_Delay(150);
				l298n_setSpeed(L298N_MOTOR_A, rotSpd);
				l298n_setSpeed(L298N_MOTOR_B, rotSpd);
				HAL_Delay(150);
				l298n_setSpeed(L298N_MOTOR_A, 0);
				l298n_setSpeed(L298N_MOTOR_B, 0);
				HAL_Delay(100);
			}
			l298n_setRotation(L298N_MOTOR_A, L298N_CCW); // forward
			l298n_setRotation(L298N_MOTOR_B, L298N_CW);
			l298n_setSpeed(L298N_MOTOR_A, drvSpd + SPD_OVERSHOOT_ADDEND / 2);
			l298n_setSpeed(L298N_MOTOR_B, drvSpd + SPD_OVERSHOOT_ADDEND / 2);
			HAL_Delay(200);
			l298n_setSpeed(L298N_MOTOR_A, drvSpd);
			l298n_setSpeed(L298N_MOTOR_B, drvSpd);
			HAL_Delay(300);
			l298n_setSpeed(L298N_MOTOR_A, 0);
			l298n_setSpeed(L298N_MOTOR_B, 0);
			HAL_Delay(200);
			for (int i = 0; i < 5; i++) { // shake again
				l298n_setRotation(L298N_MOTOR_A, L298N_CW); // left
				l298n_setRotation(L298N_MOTOR_B, L298N_CW);
				l298n_setSpeed(L298N_MOTOR_A, rotSpd + SPD_OVERSHOOT_ADDEND / 2);
				l298n_setSpeed(L298N_MOTOR_B, rotSpd + SPD_OVERSHOOT_ADDEND / 2);
				HAL_Delay(150);
				l298n_setSpeed(L298N_MOTOR_A, rotSpd);
				l298n_setSpeed(L298N_MOTOR_B, rotSpd);
				HAL_Delay(150);
				l298n_setSpeed(L298N_MOTOR_A, 0);
				l298n_setSpeed(L298N_MOTOR_B, 0);
				HAL_Delay(100);
				l298n_setRotation(L298N_MOTOR_A, L298N_CCW); // right
				l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
				l298n_setSpeed(L298N_MOTOR_A, rotSpd + SPD_OVERSHOOT_ADDEND / 2);
				l298n_setSpeed(L298N_MOTOR_B, rotSpd + SPD_OVERSHOOT_ADDEND / 2);
				HAL_Delay(150);
				l298n_setSpeed(L298N_MOTOR_A, rotSpd);
				l298n_setSpeed(L298N_MOTOR_B, rotSpd);
				HAL_Delay(150);
				l298n_setSpeed(L298N_MOTOR_A, 0);
				l298n_setSpeed(L298N_MOTOR_B, 0);
				HAL_Delay(100);
			}
		}
		break;
	case 9: // stand still, move toy up and down like the robot is fishing
		rptNum = interval / 2;
		if (rptNum < 4) rptNum = 3; // execute at least 3 times
		sg90_setAngle(SG90_MOTOR_A, TOY_ANG_DRAW);
		HAL_Delay(400);
		for (int32_t i32 = 0; i32 < rptNum; i32++) {
			sg90_setAngle(SG90_MOTOR_A, TOY_ANG_RETRACT);
			HAL_Delay(350);
			sg90_setAngle(SG90_MOTOR_A, TOY_ANG_HALF);
			HAL_Delay(300);
			sg90_setAngle(SG90_MOTOR_A, TOY_ANG_DRAW);
			HAL_Delay(350);
			sg90_setAngle(SG90_MOTOR_A, TOY_ANG_RETRACT);
			HAL_Delay(350);
			sg90_setAngle(SG90_MOTOR_A, TOY_ANG_DRAW);
			HAL_Delay(350);
			sg90_setAngle(SG90_MOTOR_A, TOY_ANG_HALF);
			HAL_Delay(300);
		}
		break;
	}
	l298n_setRotation(L298N_MOTOR_A, L298N_STOP); // stop motor rotation after each pattern exe
	l298n_setRotation(L298N_MOTOR_B, L298N_STOP);
}

static void manualDrive() {
	// enable motor first
	l298n_enable();
	sg90_enable(SG90_MOTOR_A, DEF_ANG_A);
	sg90_enable(SG90_MOTOR_B, DEF_ANG_B);
	while (1) {
		if (rpi_serialDtaAvailable()) {
			if (rpi_getSerialDta(rpidta)) {
				if (rpidta.type == TYPE_MANUAL_CTRL) {
					switch (rpidta.container[0]) {
					case 0x00: // stop
						l298n_setRotation(L298N_MOTOR_A, L298N_STOP);
						l298n_setRotation(L298N_MOTOR_B, L298N_STOP);
						break;
					case 0x01: // left
						l298n_setRotation(L298N_MOTOR_A, L298N_CW);
						l298n_setRotation(L298N_MOTOR_B, L298N_CW);
						l298n_setSpeed(L298N_MOTOR_A, MAN_ROT_SPD);
						l298n_setSpeed(L298N_MOTOR_B, MAN_ROT_SPD);
						break;
					case 0x02: // right
						l298n_setRotation(L298N_MOTOR_A, L298N_CCW);
						l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
						l298n_setSpeed(L298N_MOTOR_A, MAN_ROT_SPD);
						l298n_setSpeed(L298N_MOTOR_B, MAN_ROT_SPD);
						break;
					case 0x04: // forward
						l298n_setRotation(L298N_MOTOR_A, L298N_CCW);
						l298n_setRotation(L298N_MOTOR_B, L298N_CW);
						l298n_setSpeed(L298N_MOTOR_A, MAN_DRV_SPD);
						l298n_setSpeed(L298N_MOTOR_B, MAN_DRV_SPD);
						break;
					case 0x08: // reverse
						l298n_setRotation(L298N_MOTOR_A, L298N_CW);
						l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
						l298n_setSpeed(L298N_MOTOR_A, MAN_DRV_SPD);
						l298n_setSpeed(L298N_MOTOR_B, MAN_DRV_SPD);
						break;
					case 0x10: // snack
						sg90_setAngle(SG90_MOTOR_B, SNACK_ANG_GIVE);
						opman_addPendingOp(OP_SNACK_RET_MOTOR, OP_SNACK_RET_MOTOR_WAITING_TIME);
						break;
					case 0x20: // hide toy
						sg90_setAngle(SG90_MOTOR_A, TOY_ANG_RETRACT);
						break;
					case 0x40: // draw toy
						sg90_setAngle(SG90_MOTOR_A, TOY_ANG_DRAW);
						break;
					case 0x80: // undefined
						break;
					}
				}
				else if (rpidta.type == TYPE_SYS && rpidta.container[0] == 2) {
					// stop manual drive
					l298n_disable();
					sg90_disable(SG90_MOTOR_A);
					sg90_disable(SG90_MOTOR_B);
				}
			}
		}
	}
}

static void autoDrive() {
	//uint8_t rpiPinDta = 0;
	int patternCode = 0, patternCodePrev = 0;

	// enable motor
	l298n_enable();
	sg90_enable(SG90_MOTOR_A, DEF_ANG_A);
	sg90_enable(SG90_MOTOR_B, DEF_ANG_B);

	// call & find cat
	rpi_sendPin(RPI_PINCODE_O_SCHEDULE_EXE);
	while (1) {
		if (rpi_getPinDta() & RPI_PINCODE_I_FOUNDCAT) {
			// do something
			break;
		}
	}

	// draw toy
	sg90_setAngle(SG90_MOTOR_A, TOY_ANG_DRAW);

	// play
	while (1) {
		// get pattern code and move robot according to dequeued code
		patternCodePrev = patternCode;
		patternCode = scheduler_dequeuePattern();
		if (!patternCode) { // Auto-decide
			/*
			 * if active pattern was executed previously, do more static ones
			 * if not, do more active ones
			 * every pattern will be executed with auto-decide mode only, although it's not recommended
			 * pattern execution order for full-auto mode: 5-6-1-4-9-8-3-2-7-5-...
			 */
			switch (patternCodePrev) {
			case 1: // Waltz(S-shaped route zig-zaging)
				patternCode = 4;
				exePattern(4);
				break;
			case 2: // loop of Sudden accel., decel.
				patternCode = 7;
				exePattern(7);
				break;
			case 3: // crawling, left wheel forwards a little bit, right goes next, then left goes again...
				patternCode = 2;
				exePattern(2);
				break;
			case 4: // draw circle fast
				patternCode = 9;
				exePattern(9);
				break;
			case 5: // shake the toy left and right but doesn't go anywhere
				patternCode = 6;
				exePattern(6);
				break;
			case 6: // rotate, go to somewhere else, then rotate again
				patternCode = 1;
				exePattern(1);
				break;
			case 7: // wait until something reaches in front of IR sensor, then flee backwards
				patternCode = 5;
				exePattern(5);
				break;
			case 8: // shake the toy left and right, flee to somewhere else, then shake the toy again
				patternCode = 3;
				exePattern(3);
				break;
			case 9: // stand still, move toy up and down like the robot is fishing
				patternCode = 8;
				exePattern(8);
				break;
			case 0: // if first scheduled pattern is auto decide, do code 5(shake)
				patternCode = 5;
				exePattern(5);
				break;
			}
		}
		else {
			exePattern(patternCode);
		}

	}

	// retract toy and disable servo
	sg90_disable(SG90_MOTOR_A);
	sg90_disable(SG90_MOTOR_B);

	// move away from cat(park near a wall)
	while (1) {

	}

	// after parking, turn off motor
	l298n_disable();
}

static void coreMain() {
	int32_t i32 = 0;
	uint8_t* pu8 = 0;
	// check for rpi data
	while (1) {
		if (rpi_serialDtaAvailable()) { // process data if available
			if (rpi_getSerialDta(rpidta)) { // get data
				switch (rpidta.type) {
				case TYPE_NO:
					break;
				case TYPE_CONN:
					break;
				case TYPE_SCHEDULE_TIME:
					if (!recvScheduleMode) break;
					i32 = 0;
					pu8 = &i32;
					for (int i = 0; i < 4; i++) {
						*(pu8++) = rpidta.container[i];
					}
					if (scheduler_setTime(i32) == ERR) {
						// error
					}
					break;
				case TYPE_SCHEDULE_PATTERN:
					if (!recvScheduleMode) break;
					for (int i = 0; i < 7; i++) {
						if (rpidta.container[i] & 0x0F) scheduler_enqueuePattern(rpidta.container[i] & 0x0F);
						else break;
						if (rpidta.container[i] & 0xF0) scheduler_enqueuePattern(rpidta.container[i] & 0xF0);
						else break;
					}
					break;
				case TYPE_SCHEDULE_SNACK:
					if (!recvScheduleMode) break;
					scheduler_setSnack(rpidta.container[0]);
					break;
				case TYPE_SCHEDULE_SPEED:
					if (!recvScheduleMode) break;
					scheduler_setSpd(rpidta.container[0]);
					break;
				case TYPE_SYS:
					switch (rpidta.container[0]) {
					case 0x01: // start manual drive
						manualDrive();
						break;
					case 0xFF: // initialize whole system
						// not yet implemented
						//core_restart();
						break;
					}
					break;
				case TYPE_SCHEDULE_DURATION:
					if (!recvScheduleMode) break;
					i32 = 0;
					pu8 = &i32;
					for (int i = 0; i < 4; i++) {
						*(pu8++) = rpidta.container[i];
					}
					if (scheduler_setDuration(i32) == ERR) {
						// error
					}
					break;
				case TYPE_SCHEDULE_START:
					recvScheduleMode = TRUE;
					break;
				case TYPE_SCHEDULE_END:
					recvScheduleMode = FALSE;
				}

			}
		}
		else { // do something else

		}
		// check if schedule is set
		// check for schedule. process schedule if time has been elapsed
		if (flagTimeElapsed) {
			flagTimeElapsed = FALSE; // reset flag first
			flagAutorun = TRUE;
			autoDrive();
			flagAutorun = FALSE;
		}
	}
}

void core_setHandle(TIM_HandleTypeDef* ph) {

}

void core_start() {
	if (initState) coreMain(); // skip initialization

	// initialization
	periph_init();
	opman_init();
	rpi_init();
	scheduler_init();
	l298n_init();
	sg90_init();
	initState = TRUE;

	// start core
	if (secTimEna == FALSE) { // enable timer if timer is off
		HAL_TIM_Base_Start_IT(pTimHandle);
		secTimEna = TRUE;
	}
	coreMain();
}

void core_callOp(uint8_t opcode) {
	switch (opcode) {
	case OP_SNACK_RET_MOTOR:
		sg90_setAngle(SG90_MOTOR_B, SNACK_ANG_RDY);
		break;
	case OP_RPI_PIN_IO_SEND_RESET:
		rpi_opmanTimeoutHandler();
		break;
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		rpi_RxCpltCallbackHandler();
		//flagRxCplt = TRUE;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance == TIM10) { // 1s sys tim
		if(scheduler_TimCallbackHandler()) flagTimeElapsed = TRUE;
		else flagTimeElapsed = FALSE;
	}
	else if (htim->Instance == TIM11) { // 1ms sys tim(for postponed ops)
		opman_callbackHandler();
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

}
