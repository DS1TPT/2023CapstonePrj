/**
  *********************************************************************************************
  * NAME OF THE FILE : app.c
  * BRIEF INFORMATION: robot application
  *
  * Copyright (c) 2023 Lee Geon-goo.
  * All rights reserved.
  *
  * This file is part of catCareBot.
  *
  *********************************************************************************************
  */

#include "app.h"
#include "carebotCore.h"
#include "carebotPeripherals.h"
#include "rpicomm.h"
#include "l298n.h"
#include "sg90.h"
#include "buzzer.h"

struct SerialDta rpidta;

#define PATTERN_EXE_MODE_AUTO 0
#define PATTERN_EXE_MODE_MAN 1

#define AUTOPLAY_STATUS_BEGIN 0
#define AUTOPLAY_STATUS_DO 1
#define AUTOPLAY_STATUS_END 2

/* TEST MODE can be disabled by commenting some lines at: carebotCore.h */

// for audible execution: (AUTODRIVE MODE AND COMM ONLY) notify what's going on using beep.
#define _AUDIBLE_EXECUTION_ENABLED

// system properties (editable)
const uint8_t MAN_ROT_SPD = 44; // RANGE: 38~48, EVEN NUMBER. AFFECTS AUTO SPEED
const uint8_t MAN_DRV_SPD = 48; // RANGE: 38~48, EVEN NUMBER. AFFECTS AUTO SPEED
const uint8_t SPD_ADDEND = 3; // THIS NUMBER MUST NOT EXCEED: 100 - MANUAL SPEED * 2
const uint8_t SPD_SUBTRAHEND = 6; // THIS NUMBER MUST BE LESS THAN: MANUAL SPEED / 4
const uint8_t DEF_ANG_A = 30; // default angle of snack motor
//const uint8_t DEF_ANG_B = ; // reserve servo b
const uint16_t OP_SNACK_RET_MOTOR_WAITING_TIME = 500; // in milliseconds
const int32_t CAT_SEARCH_WAIT_TIME = 20; // in seconds
const int32_t VIB_WAIT_TIME = 600; // in seconds
const int32_t PATTERN_WAIT_AND_FLEE_WAIT_TIME = 20; // RANGE: 1 ~ 60, in seconds

// derived properties (NOT EDITABLE)

const uint8_t AUTO_DEF_ROT_SPD = MAN_ROT_SPD;
const uint8_t AUTO_DEF_DRV_SPD = MAN_DRV_SPD;
// auto minimum speed calculation formula below is deprecated since it was not able to run motor;
//const uint8_t AUTO_MIN_ROT_SPD = (uint8_t)((float)AUTO_DEF_ROT_SPD / 2.0) - (((float)AUTO_DEF_ROT_SPD / 2.0 > 0) ? 0 : 1);
//const uint8_t AUTO_MIN_DRV_SPD = (uint8_t)((float)AUTO_DEF_DRV_SPD / 2.0) - (((float)AUTO_DEF_ROT_SPD / 2.0 > 0) ? 0 : 1);
// actual minimum motor speed should be around 35~40 to run
// therefore, i manually set minimum value, and it won't be affected by user settings.
const uint8_t AUTO_MIN_ROT_SPD = 38;
const uint8_t AUTO_MIN_DRV_SPD = 38;
const uint8_t SPD_OVERSHOOT_ADDEND = ((AUTO_DEF_ROT_SPD >= 50 || AUTO_DEF_DRV_SPD >= 50) ? 0 : (AUTO_DEF_ROT_SPD > AUTO_DEF_DRV_SPD) ? (100 - AUTO_DEF_DRV_SPD * 2) : (100 - AUTO_DEF_ROT_SPD * 2));

const uint8_t SNACK_ANG_RDY = DEF_ANG_A;
const uint8_t SNACK_ANG_GIVE = DEF_ANG_A + 90;


// system variables
static volatile uint8_t flagSkdTimeElapsed = FALSE;
static volatile uint8_t flagVibWaitTimeout = FALSE;
static volatile uint8_t flagCatSearchTimeout = FALSE;
static volatile uint8_t flagAutorun = FALSE;
static volatile uint8_t recvScheduleMode = FALSE;
static volatile uint8_t initState = FALSE;
static volatile uint8_t autoplayStatus = AUTOPLAY_STATUS_BEGIN;

static struct dtaStructQueueU8 patternQueue;
static uint8_t speed = 0; // 0 ~ 2.
static uint8_t rotSpd = AUTO_DEF_ROT_SPD * 2;
static uint8_t drvSpd = AUTO_DEF_DRV_SPD * 2;
static volatile int32_t skdWaitTime = 0;
static volatile int32_t vibWaitTime = 0;
static volatile int32_t catSearchWaitTime = 0;
static volatile int32_t skdDuration = 0;
static volatile _Bool sndRptOutputStat = FALSE; // sound repeat: output on or off
static int skdSpd = 0;
static int skdSnackIntv = 0;
static volatile _Bool skdIsSet = FALSE;
static volatile _Bool catSearchIsSet = FALSE;
static volatile _Bool vibWaitIsSet = FALSE;
static volatile _Bool sndRptIsSet = FALSE;
static _Bool isAutoplayCancelled = FALSE;

static uint8_t opcodePendingOp = 0;

/* basic functions */
int32_t atoi32(uint8_t* str) {
    int32_t result, positive;

    result = 0;
    positive = 1;
    while (( 9 <= *str && *str <= 13) || *str == ' ')
        str++;
    if (*str == '+' || *str == '-') {
        if (*str == '-')
            positive = -1;
        str++;
    }
    while ('0' <= *str && *str <= '9') {
        result *= 10;
        result += (*str - '0') * positive;
        str++;
    }
    return result;
}

/* schedule related functions */

/* play related functions */
static void exePattern(int code, int mode) {
#ifdef _TEST_MODE_ENABLED
	core_dbgTx("BEGIN PATTERN ");
#endif
	int32_t interval = 0; // seconds
	int32_t rptNum = 1;
	int32_t rptTime = 1;
	int32_t cnt = 0;
	if (mode == PATTERN_EXE_MODE_AUTO) {
		if (autoplayStatus == AUTOPLAY_STATUS_BEGIN) { // to avoid hard fault: div by 0. to avoid some logical bugs
			interval = skdDuration / patternQueue.index;
			if (!flagAutorun) interval = 1;
			autoplayStatus = AUTOPLAY_STATUS_DO;
		}
		core_call_delayms(300); // give a slight delay between patterns
	}
	else if (mode == PATTERN_EXE_MODE_MAN) {
		rotSpd = AUTO_DEF_ROT_SPD * 2;
		drvSpd = AUTO_DEF_DRV_SPD * 2;
		interval = 1;
		cnt = 0;
	}

#ifdef _AUDIBLE_EXECUTION_ENABLED
	buzzer_mute();
	buzzer_setTone(toneA4);
	buzzer_setDuty(50);
	for (int audibleExeForCountVar = 0; audibleExeForCountVar < code; audibleExeForCountVar++) {
		buzzer_unmute();
		core_call_delayms(250);
		buzzer_mute();
		core_call_delayms(250);
	}
#endif

	switch (code) {
	case 1: // Waltz(S-shaped route zig-zaging)
		rptNum = interval / 3;
		if (rptNum < 2) rptNum = 1; // execute at least one time
		l298n_setRotation(L298N_MOTOR_A, L298N_CCW); // initial rotation
		l298n_setRotation(L298N_MOTOR_B, L298N_CW);
		l298n_setSpeed(L298N_MOTOR_A, AUTO_DEF_ROT_SPD); // rotation speed will not be affected by speed multiplier
		l298n_setSpeed(L298N_MOTOR_B, AUTO_MIN_ROT_SPD);
		core_call_delayms(500);
		for (int32_t i32 = 0; i32 < rptNum; i32++) {
			// forward
			l298n_setSpeed(L298N_MOTOR_A, drvSpd);
			l298n_setSpeed(L298N_MOTOR_B, drvSpd);
			core_call_delayms(300);
			l298n_setSpeed(L298N_MOTOR_A, AUTO_MIN_ROT_SPD); // rotation speed will not be affected by speed multiplier
			l298n_setSpeed(L298N_MOTOR_B, AUTO_DEF_ROT_SPD);
			core_call_delayms(1000);
			l298n_setSpeed(L298N_MOTOR_A, drvSpd);
			l298n_setSpeed(L298N_MOTOR_B, drvSpd);
			core_call_delayms(300);
			l298n_setSpeed(L298N_MOTOR_A, AUTO_DEF_ROT_SPD); // rotation speed will not be affected by speed multiplier
			l298n_setSpeed(L298N_MOTOR_B, AUTO_MIN_ROT_SPD);
			core_call_delayms(1000);
		}
		break;
	case 2: // loop of Sudden accel., decel.
		rptNum = interval / 20;
		if (rptNum < 2) rptNum = 1; // execute at least one time
		for (int32_t i32 = 0; i32 < rptNum; i32++) {
			// forward
			l298n_setRotation(L298N_MOTOR_A, L298N_CCW);
			l298n_setRotation(L298N_MOTOR_B, L298N_CW);
			for (int i = 0; i < 4; i++) {
				l298n_setSpeed(L298N_MOTOR_A, drvSpd + SPD_OVERSHOOT_ADDEND);
				l298n_setSpeed(L298N_MOTOR_B, drvSpd + SPD_OVERSHOOT_ADDEND);
				core_call_delayms(800);
				l298n_setSpeed(L298N_MOTOR_A, drvSpd);
				l298n_setSpeed(L298N_MOTOR_B, drvSpd);
				core_call_delayms(700);
				l298n_setSpeed(L298N_MOTOR_A, 0);
				l298n_setSpeed(L298N_MOTOR_B, 0);
				core_call_delayms(1000);
			}
			// backward
			l298n_setRotation(L298N_MOTOR_A, L298N_CW);
			l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
			for (int i = 0; i < 4; i++) {
				l298n_setSpeed(L298N_MOTOR_A, drvSpd + SPD_OVERSHOOT_ADDEND);
				l298n_setSpeed(L298N_MOTOR_B, drvSpd + SPD_OVERSHOOT_ADDEND);
				core_call_delayms(800);
				l298n_setSpeed(L298N_MOTOR_A, drvSpd);
				l298n_setSpeed(L298N_MOTOR_B, drvSpd);
				core_call_delayms(700);
				l298n_setSpeed(L298N_MOTOR_A, 0);
				l298n_setSpeed(L298N_MOTOR_B, 0);
				core_call_delayms(1000);
			}

		}
		break;
	case 3: // crawling, left wheel forwards a little bit, right goes next, then left goes again...
		rptNum = interval / 10;
		if (rptNum < 2) rptNum = 1; /// execute at least one time
		for (int32_t i32 = 0; i32 < rptNum; i32++) {
			for (int i = 0; i < 5; i++) {
				l298n_setRotation(L298N_MOTOR_A, L298N_CCW);
				l298n_setRotation(L298N_MOTOR_B, L298N_CW);
				l298n_setSpeed(L298N_MOTOR_A, drvSpd);
				l298n_setSpeed(L298N_MOTOR_B, AUTO_MIN_ROT_SPD);
				core_call_delayms(1000);
				l298n_setRotation(L298N_MOTOR_A, L298N_CCW);
				l298n_setRotation(L298N_MOTOR_B, L298N_CW);
				l298n_setSpeed(L298N_MOTOR_A, AUTO_MIN_ROT_SPD);
				l298n_setSpeed(L298N_MOTOR_B, drvSpd);
				core_call_delayms(1000);
			}
			for (int i = 0; i < 5; i++) {
				l298n_setRotation(L298N_MOTOR_A, L298N_CW);
				l298n_setRotation(L298N_MOTOR_B, L298N_STOP);
				l298n_setSpeed(L298N_MOTOR_A, rotSpd);
				core_call_delayms(1000);
				l298n_setRotation(L298N_MOTOR_A, L298N_STOP);
				l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
				l298n_setSpeed(L298N_MOTOR_B, rotSpd);
				core_call_delayms(1000);
			}
		}
		break;
	case 4: // draw circle fast
		rptTime = interval;
		if (rptTime < 2) rptTime = 10; // ensure execution
		l298n_setRotation(L298N_MOTOR_A, L298N_CCW); // right
		l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
		l298n_setSpeed(L298N_MOTOR_A, drvSpd + SPD_ADDEND);
		l298n_setSpeed(L298N_MOTOR_B, rotSpd);
		core_call_delayms(rptTime * 1000);
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
			core_call_delayms(150);
			l298n_setSpeed(L298N_MOTOR_A, rotSpd);
			l298n_setSpeed(L298N_MOTOR_B, rotSpd);
			core_call_delayms(150);
			l298n_setSpeed(L298N_MOTOR_A, 0);
			l298n_setSpeed(L298N_MOTOR_B, 0);
			core_call_delayms(100);
			l298n_setRotation(L298N_MOTOR_A, L298N_CW); // left
			l298n_setRotation(L298N_MOTOR_B, L298N_CW);
			l298n_setSpeed(L298N_MOTOR_A, rotSpd + SPD_OVERSHOOT_ADDEND);
			l298n_setSpeed(L298N_MOTOR_B, rotSpd + SPD_OVERSHOOT_ADDEND);
			core_call_delayms(150);
			l298n_setSpeed(L298N_MOTOR_A, rotSpd);
			l298n_setSpeed(L298N_MOTOR_B, rotSpd);
			core_call_delayms(150);
			l298n_setSpeed(L298N_MOTOR_A, 0);
			l298n_setSpeed(L298N_MOTOR_B, 0);
			core_call_delayms(100);
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
			core_call_delayms(7000);
			l298n_setRotation(L298N_MOTOR_A, L298N_CCW); // forward
			l298n_setRotation(L298N_MOTOR_B, L298N_CW);
			l298n_setSpeed(L298N_MOTOR_A, drvSpd);
			l298n_setSpeed(L298N_MOTOR_B, drvSpd);
			core_call_delayms(5000);
			l298n_setRotation(L298N_MOTOR_A, L298N_CW); // left
			l298n_setRotation(L298N_MOTOR_B, L298N_CW);
			l298n_setSpeed(L298N_MOTOR_A, rotSpd);
			l298n_setSpeed(L298N_MOTOR_B, rotSpd);
			core_call_delayms(7000);
			l298n_setRotation(L298N_MOTOR_A, L298N_CW); // backward
			l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
			l298n_setSpeed(L298N_MOTOR_A, drvSpd);
			l298n_setSpeed(L298N_MOTOR_B, drvSpd);
			core_call_delayms(5000);
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
				core_call_delayms(500);
				l298n_setSpeed(L298N_MOTOR_A, drvSpd);
				l298n_setSpeed(L298N_MOTOR_B, drvSpd);
				core_call_delayms(1000);
				l298n_setSpeed(L298N_MOTOR_A, 0);
				l298n_setSpeed(L298N_MOTOR_B, 0);
				break;
			}
			core_call_delayms(100);
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
				core_call_delayms(150);
				l298n_setSpeed(L298N_MOTOR_A, rotSpd);
				l298n_setSpeed(L298N_MOTOR_B, rotSpd);
				core_call_delayms(150);
				l298n_setSpeed(L298N_MOTOR_A, 0);
				l298n_setSpeed(L298N_MOTOR_B, 0);
				core_call_delayms(100);
				l298n_setRotation(L298N_MOTOR_A, L298N_CCW); // right
				l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
				l298n_setSpeed(L298N_MOTOR_A, rotSpd + SPD_OVERSHOOT_ADDEND / 2);
				l298n_setSpeed(L298N_MOTOR_B, rotSpd + SPD_OVERSHOOT_ADDEND / 2);
				core_call_delayms(150);
				l298n_setSpeed(L298N_MOTOR_A, rotSpd);
				l298n_setSpeed(L298N_MOTOR_B, rotSpd);
				core_call_delayms(150);
				l298n_setSpeed(L298N_MOTOR_A, 0);
				l298n_setSpeed(L298N_MOTOR_B, 0);
				core_call_delayms(100);
			}
			l298n_setRotation(L298N_MOTOR_A, L298N_CCW); // forward
			l298n_setRotation(L298N_MOTOR_B, L298N_CW);
			l298n_setSpeed(L298N_MOTOR_A, drvSpd + SPD_OVERSHOOT_ADDEND / 2);
			l298n_setSpeed(L298N_MOTOR_B, drvSpd + SPD_OVERSHOOT_ADDEND / 2);
			core_call_delayms(200);
			l298n_setSpeed(L298N_MOTOR_A, drvSpd);
			l298n_setSpeed(L298N_MOTOR_B, drvSpd);
			core_call_delayms(300);
			l298n_setSpeed(L298N_MOTOR_A, 0);
			l298n_setSpeed(L298N_MOTOR_B, 0);
			core_call_delayms(200);
			for (int i = 0; i < 5; i++) { // shake again
				l298n_setRotation(L298N_MOTOR_A, L298N_CW); // left
				l298n_setRotation(L298N_MOTOR_B, L298N_CW);
				l298n_setSpeed(L298N_MOTOR_A, rotSpd + SPD_OVERSHOOT_ADDEND / 2);
				l298n_setSpeed(L298N_MOTOR_B, rotSpd + SPD_OVERSHOOT_ADDEND / 2);
				core_call_delayms(150);
				l298n_setSpeed(L298N_MOTOR_A, rotSpd);
				l298n_setSpeed(L298N_MOTOR_B, rotSpd);
				core_call_delayms(150);
				l298n_setSpeed(L298N_MOTOR_A, 0);
				l298n_setSpeed(L298N_MOTOR_B, 0);
				core_call_delayms(100);
				l298n_setRotation(L298N_MOTOR_A, L298N_CCW); // right
				l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
				l298n_setSpeed(L298N_MOTOR_A, rotSpd + SPD_OVERSHOOT_ADDEND / 2);
				l298n_setSpeed(L298N_MOTOR_B, rotSpd + SPD_OVERSHOOT_ADDEND / 2);
				core_call_delayms(150);
				l298n_setSpeed(L298N_MOTOR_A, rotSpd);
				l298n_setSpeed(L298N_MOTOR_B, rotSpd);
				core_call_delayms(150);
				l298n_setSpeed(L298N_MOTOR_A, 0);
				l298n_setSpeed(L298N_MOTOR_B, 0);
				core_call_delayms(100);
			}
		}
		break;
	case 9: // stand still, move toy left and right like the robot is fishing horizontally
		rptNum = interval / 2;
		if (rptNum < 4) rptNum = 3; // execute at least 3 times
		core_call_delayms(400);
		for (int32_t i32 = 0; i32 < rptNum; i32++) {
			// implementation here
			l298n_setRotation(L298N_MOTOR_A, L298N_CW); // left slow
			l298n_setRotation(L298N_MOTOR_B, L298N_CW);
			l298n_setSpeed(L298N_MOTOR_A, rotSpd);
			l298n_setSpeed(L298N_MOTOR_B, rotSpd);
			core_call_delayms(1000);
			l298n_setRotation(L298N_MOTOR_A, L298N_STOP);
			l298n_setRotation(L298N_MOTOR_B, L298N_STOP);
			core_call_delayms(500);
			l298n_setRotation(L298N_MOTOR_A, L298N_CW); // right fast
			l298n_setRotation(L298N_MOTOR_B, L298N_CW);
			l298n_setSpeed(L298N_MOTOR_A, rotSpd);
			l298n_setSpeed(L298N_MOTOR_B, rotSpd);
			core_call_delayms(500);
			l298n_setRotation(L298N_MOTOR_A, L298N_STOP);
			l298n_setRotation(L298N_MOTOR_B, L298N_STOP);
			core_call_delayms(500);
		}
		break;
	}
	l298n_setRotation(L298N_MOTOR_A, L298N_STOP); // stop motor rotation after each pattern exe
	l298n_setRotation(L298N_MOTOR_B, L298N_STOP);
#ifdef _TEST_MODE_ENABLED
	core_dbgTx("END PATTERN\r\n");
#endif
}

static void manualDrive() {
#ifdef _TEST_MODE_ENABLED
	core_dbgTx("BEGIN MANUAL MODE\r\n");
#endif
	// enable motor first
	l298n_enable();
	sg90_enable(SG90_MOTOR_A, DEF_ANG_A);
	while (1) {
		if (rpi_serialDtaAvailable()) {
			if (rpi_getSerialDta(&rpidta)) {
#ifdef _TEST_MODE_ENABLED
				uint8_t buf[9] = { 0, };
				buf[0] = rpidta.type;
				for (int i = 0; i < 7; i++) {
					buf[i + 1] = rpidta.container[i];
				}
				buf[8] = 0;
				core_dbgTx((char*)buf);
				core_dbgTx("\r\n");
#endif
				if (rpidta.type == TYPE_MANUAL_CTRL && rpidta.container[0] == '0') {
					switch (rpidta.container[1]) {
					case '0': // stop
						l298n_setRotation(L298N_MOTOR_A, L298N_STOP);
						l298n_setRotation(L298N_MOTOR_B, L298N_STOP);
						break;
					case '3': // left
						l298n_setRotation(L298N_MOTOR_A, L298N_CW);
						l298n_setRotation(L298N_MOTOR_B, L298N_CW);
						l298n_setSpeed(L298N_MOTOR_A, MAN_ROT_SPD);
						l298n_setSpeed(L298N_MOTOR_B, MAN_ROT_SPD);
						break;
					case '4': // right
						l298n_setRotation(L298N_MOTOR_A, L298N_CCW);
						l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
						l298n_setSpeed(L298N_MOTOR_A, MAN_ROT_SPD);
						l298n_setSpeed(L298N_MOTOR_B, MAN_ROT_SPD);
						break;
					case '1': // forward
						l298n_setRotation(L298N_MOTOR_A, L298N_CCW);
						l298n_setRotation(L298N_MOTOR_B, L298N_CW);
						l298n_setSpeed(L298N_MOTOR_A, MAN_DRV_SPD);
						l298n_setSpeed(L298N_MOTOR_B, MAN_DRV_SPD);
						break;
					case '2': // reverse
						l298n_setRotation(L298N_MOTOR_A, L298N_CW);
						l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
						l298n_setSpeed(L298N_MOTOR_A, MAN_DRV_SPD);
						l298n_setSpeed(L298N_MOTOR_B, MAN_DRV_SPD);
						break;
					}
				}
				else if (rpidta.type == TYPE_MANUAL_CTRL && rpidta.container[0] != '0') {
					if (rpidta.container[0] == 'P') {
#ifdef _TEST_MODE_ENABLED
						core_dbgTx("RECEIVED PATTERN CODE!\r\n");
#endif
						exePattern((rpidta.container[1] - 0x30), PATTERN_EXE_MODE_MAN);
					}
				}
				else if (rpidta.type == TYPE_SYS && rpidta.container[0] == '2') {
					// stop manual drive
					l298n_disable();
					sg90_disable(SG90_MOTOR_A);
#ifdef _TEST_MODE_ENABLED
					core_dbgTx("END MANUAL MODE\r\n");
#endif
					return;
				}
			}
		}
	}
}

static void autoDrive() {
	//uint8_t rpiPinDta = 0;
	uint8_t patternCode, patternCodePrev;
	int snackIntvCnt;
	const int* snackIntvVal = &skdSnackIntv;

	autoplayStatus = AUTOPLAY_STATUS_BEGIN;
	patternCode = 0;
	patternCodePrev = 0;
	snackIntvCnt = 0;

	// enable motor
	l298n_enable();
	sg90_enable(SG90_MOTOR_A, DEF_ANG_A);

	// skip searching if the schedule was cancelled previously
	if (isAutoplayCancelled) {
		isAutoplayCancelled = FALSE;
		goto lbl_autoDrive_play;
	}

	// set cat searching flag
	catSearchWaitTime = CAT_SEARCH_WAIT_TIME;
	catSearchIsSet = TRUE;
	flagCatSearchTimeout = FALSE;

	// call & find cat
	rpi_sendPin(RPI_PINCODE_O_SCHEDULE_EXE);
	core_call_delayms(1000);

	l298n_setRotation(L298N_MOTOR_A, L298N_CW); // rotate left slowly, to find cat
	l298n_setRotation(L298N_MOTOR_B, L298N_CW);
	l298n_setSpeed(L298N_MOTOR_A, AUTO_MIN_ROT_SPD);
	l298n_setSpeed(L298N_MOTOR_B, AUTO_MIN_ROT_SPD);
	rpi_foundCat(); // clears age-old flag
	while (1) {
		if (rpi_foundCat()) { // cat is found
			// do something
			sndRptIsSet = FALSE;
			sndRptOutputStat = FALSE;
			buzzer_mute();
			core_call_delayms(250);
			buzzer_setTone(toneC6);
			buzzer_setDuty(50);
			buzzer_unmute();
			core_call_delayms(300);
			buzzer_mute();
			break;
		}
		if (flagCatSearchTimeout) { // couldn't find cat, start wait-calling mode
			l298n_setRotation(L298N_MOTOR_A, L298N_STOP); // stop first
			l298n_setRotation(L298N_MOTOR_B, L298N_STOP);
			// set tone
			buzzer_setTone(toneF6);
			buzzer_setDuty(25);
			// set sound on/off to true
			buzzer_unmute();
			sndRptOutputStat = TRUE;
			sndRptIsSet = TRUE;
			// set timeout time and marker
			vibWaitTime = VIB_WAIT_TIME;
			vibWaitIsSet = TRUE;
			while (1) {
				// check for vibration every 100ms
				if (periph_isVibration() == TRUE) { // detected vibration
					vibWaitIsSet = FALSE;
					vibWaitTime = 0;
					sndRptIsSet = FALSE;
					sndRptOutputStat = FALSE;
					buzzer_mute();
					break;
				}
				// check for timeout
				if (flagVibWaitTimeout == TRUE) {
					// notify autoplay is cancelled, and make robot silent.
					vibWaitIsSet = FALSE;
					vibWaitTime = 0;
					buzzer_setTone(toneA4);
					buzzer_setDuty(50);
					for (int i = 0; i < 5; i++) {
						buzzer_unmute();
						core_call_delayms(500);
						buzzer_mute();
						core_call_delayms(500);
					}
					isAutoplayCancelled = TRUE; // mark cancelled
					l298n_disable(); // disable motors
					sg90_disable(SG90_MOTOR_A);
					sndRptIsSet = FALSE;
					sndRptOutputStat = FALSE;
					buzzer_mute();
					return;
				}
				core_call_delayms(25);
			}
			break;
		}
	}

	// play
	lbl_autoDrive_play:

	snackIntvCnt = 0;
	while (1) {
		// get pattern code and move robot according to dequeued code
		patternCodePrev = patternCode;
		if (core_dtaStruct_queueU8isEmpty(&patternQueue)) break;
		if (core_dtaStruct_dequeueU8(&patternQueue, &patternCode) == ERR) break; // empty
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
				exePattern(4, PATTERN_EXE_MODE_AUTO);
				break;
			case 2: // loop of Sudden accel., decel.
				patternCode = 7;
				exePattern(7, PATTERN_EXE_MODE_AUTO);
				break;
			case 3: // crawling, left wheel forwards a little bit, right goes next, then left goes again...
				patternCode = 2;
				exePattern(2, PATTERN_EXE_MODE_AUTO);
				break;
			case 4: // draw circle fast
				patternCode = 9;
				exePattern(9, PATTERN_EXE_MODE_AUTO);
				break;
			case 5: // shake the toy left and right but doesn't go anywhere
				patternCode = 6;
				exePattern(6, PATTERN_EXE_MODE_AUTO);
				break;
			case 6: // rotate, go to somewhere else, then rotate again
				patternCode = 1;
				exePattern(1, PATTERN_EXE_MODE_AUTO);
				break;
			case 7: // wait until something reaches in front of IR sensor, then flee backwards
				patternCode = 5;
				exePattern(5, PATTERN_EXE_MODE_AUTO);
				break;
			case 8: // shake the toy left and right, flee to somewhere else, then shake the toy again
				patternCode = 3;
				exePattern(3, PATTERN_EXE_MODE_AUTO);
				break;
			case 9: // stand still, move toy up and down like the robot is fishing
				patternCode = 8;
				exePattern(8, PATTERN_EXE_MODE_AUTO);
				break;
			case 0: // if first scheduled pattern is auto decide, do code 5(shake)
				patternCode = 5;
				exePattern(5, PATTERN_EXE_MODE_AUTO);
				break;
			}
		}
		else {
			exePattern(patternCode, PATTERN_EXE_MODE_AUTO);
		}
		if (++snackIntvCnt >= *snackIntvVal) { // give snack
			snackIntvCnt = 0;
			buzzer_setTone(toneFS6);
			buzzer_setDuty(50);
			buzzer_unmute();
			core_call_delayms(400);
			buzzer_mute();
			l298n_setRotation(L298N_MOTOR_A, L298N_CCW); // forwards to create inertia
			l298n_setRotation(L298N_MOTOR_B, L298N_CW);
			l298n_setSpeed(L298N_MOTOR_A, L298N_MAX_SPD);
			l298n_setSpeed(L298N_MOTOR_B, L298N_MAX_SPD);
			core_call_delayms(400);
			sg90_setAngle(SG90_MOTOR_A, SNACK_ANG_GIVE);
			core_call_pendingOpAdd(opcodePendingOp, OP_SNACK_RET_MOTOR_WAITING_TIME);
			l298n_setRotation(L298N_MOTOR_A, L298N_CW); // backwards, fast speed to use inertia of snack
			l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
			l298n_setSpeed(L298N_MOTOR_A, L298N_MAX_SPD);
			l298n_setSpeed(L298N_MOTOR_B, L298N_MAX_SPD);
			core_call_delayms(800);
			l298n_setRotation(L298N_MOTOR_A, L298N_STOP);
			l298n_setRotation(L298N_MOTOR_B, L298N_STOP);
			core_call_delayms(1000);
		}
	}

	// disable servo
	sg90_disable(SG90_MOTOR_A);

	// move away from cat(park near a wall)
	// for safety, if robot couldn't find an object with ir prox snsr for more than 15 sec,
	// abort wall-searching and park
	l298n_setRotation(L298N_MOTOR_A, L298N_CCW); // forward, slow
	l298n_setRotation(L298N_MOTOR_B, L298N_CW);
	l298n_setSpeed(L298N_MOTOR_A, AUTO_MIN_DRV_SPD);
	l298n_setSpeed(L298N_MOTOR_B, AUTO_MIN_DRV_SPD);

	unsigned parkPeriodCnt;
	parkPeriodCnt = 0;
	while (1) {
		if (periph_irSnsrChk(IR_SNSR_MODE_OP) == IR_SNSR_NEAR || parkPeriodCnt >= 150) {
			l298n_setRotation(L298N_MOTOR_A, L298N_STOP);
			l298n_setRotation(L298N_MOTOR_B, L298N_STOP);
			break;
		}
		core_call_delayms(100);
		parkPeriodCnt++;
	}

	// after parking, turn off motor
	l298n_disable();
	autoplayStatus = AUTOPLAY_STATUS_END;
	buzzer_setTone(toneE6);
	buzzer_setDuty(50);
	buzzer_unmute();
	core_call_delayms(250);
	buzzer_setTone(toneG6);
	core_call_delayms(250);
	buzzer_setTone(toneC7);
	core_call_delayms(250);
	buzzer_mute();
}

/* main */

static void appMain() {
	//int32_t i32 = 0;
#ifdef _TEST_MODE_ENABLED
	core_dbgTx("\r\nAPPLICATION START\r\n");
#endif
	// check for rpi data
	while (1) {
		if (rpi_serialDtaAvailable()) { // process data if available
			if (rpi_getSerialDta(&rpidta)) { // get data

#ifdef _TEST_MODE_ENABLED
				uint8_t buf[9] = { 0, };
				buf[0] = rpidta.type;
				for (int i = 0; i < 7; i++) {
					buf[i + 1] = rpidta.container[i];
				}
				buf[8] = 0;
				core_dbgTx((char*)buf);
				core_dbgTx("\r\n");
#endif

				switch (rpidta.type) {
				case TYPE_SCHEDULE_TIME:
					if (!recvScheduleMode) break;
					skdWaitTime = atoi32(rpidta.container);
#ifdef _AUDIBLE_EXECUTION_ENABLED
					buzzer_mute();
					buzzer_setTone(toneG6);
					buzzer_setDuty(50);
					buzzer_unmute();
					core_call_delayms(250);
					buzzer_mute();
					core_call_delayms(250);
#endif
					break;
				case TYPE_SCHEDULE_PATTERN:
					if (!recvScheduleMode) break;
					for (int i = 0; i < 7; i++) {
						if (rpidta.container[i]) {
							if (rpidta.container[i] != '.') core_dtaStruct_enqueueU8(&patternQueue, rpidta.container[i] - 0x30);
						}
						else break;
					}
#ifdef _AUDIBLE_EXECUTION_ENABLED
					buzzer_mute();
					buzzer_setTone(toneA6);
					buzzer_setDuty(50);
					buzzer_unmute();
					core_call_delayms(250);
					buzzer_mute();
					core_call_delayms(250);
#endif
					break;
				case TYPE_SCHEDULE_SNACK_INTERVAL:
					if (!recvScheduleMode) break;
					skdSnackIntv = rpidta.container[0];
#ifdef _AUDIBLE_EXECUTION_ENABLED
					buzzer_mute();
					buzzer_setTone(toneB6);
					buzzer_setDuty(50);
					buzzer_unmute();
					core_call_delayms(250);
					buzzer_mute();
					core_call_delayms(250);
#endif
					break;
				case TYPE_SCHEDULE_SPEED:
					if (!recvScheduleMode) break;
					skdSpd = rpidta.container[0];
					if (skdSpd < 0) skdSpd = 0;
					else if (skdSpd > 2) skdSpd = 2;

					if (skdSpd) {
						rotSpd = AUTO_DEF_ROT_SPD * skdSpd;
						drvSpd = AUTO_DEF_DRV_SPD * skdSpd;
					}
					else {
						rotSpd = AUTO_MIN_ROT_SPD;
						drvSpd = AUTO_MIN_DRV_SPD;
					}

#ifdef _AUDIBLE_EXECUTION_ENABLED
					buzzer_mute();
					buzzer_setTone(toneC7);
					buzzer_setDuty(50);
					buzzer_unmute();
					core_call_delayms(250);
					buzzer_mute();
					core_call_delayms(250);
#endif
					break;
				case TYPE_SYS:
#ifdef _TEST_MODE_ENABLED
					core_dbgTx("SYS CMD: ");
#endif
					switch (rpidta.container[0]) {
					case '1': // start manual drive
						manualDrive();
						break;
					case '9': // initialize whole system
						// not yet implemented
						//core_restart();
						break;
					}
					break;
				case TYPE_SCHEDULE_DURATION:
					if (!recvScheduleMode) break;
					skdDuration = atoi32(rpidta.container);
					break;
				case TYPE_SCHEDULE_START:
					skdDuration = 1; // if no input, play only once

					recvScheduleMode = TRUE;
					isAutoplayCancelled = FALSE; // reset autoplay cancel status to FALSE, since new schedule is being input.
#ifdef _AUDIBLE_EXECUTION_ENABLED
					buzzer_mute();
					buzzer_setTone(toneE6);
					buzzer_setDuty(50);
					buzzer_unmute();
					core_call_delayms(250);
					buzzer_mute();
					core_call_delayms(250);
#endif
					break;
				case TYPE_SCHEDULE_END:
					recvScheduleMode = FALSE;
#ifdef _AUDIBLE_EXECUTION_ENABLED
					buzzer_mute();
					buzzer_setTone(toneE6);
					buzzer_setDuty(50);
					for (int audibleExeForCountVar = 0; audibleExeForCountVar < 3; audibleExeForCountVar++) {
						buzzer_unmute();
						core_call_delayms(200);
						buzzer_mute();
						core_call_delayms(200);
					}
#endif
					skdIsSet = TRUE;
				}
			}
		}
		else { // if no data is available, do something else

		}
		// check if schedule is set
		// check for schedule. process schedule if time has been elapsed
		if (flagSkdTimeElapsed) {
			flagSkdTimeElapsed = FALSE; // reset flag first
			skdIsSet = FALSE;
			flagAutorun = TRUE;

			for (int i = 0; i < 2; i++) {
				buzzer_setTone(toneC6);
				buzzer_setDuty(50);
				buzzer_unmute();
				core_call_delayms(500);
				buzzer_setTone(toneE6);
				core_call_delayms(500);
				buzzer_setTone(toneG6);
				core_call_delayms(2000);
			}

			buzzer_mute();
			autoDrive();
			flagAutorun = FALSE;
		}
		// check if autoplay is cancelled
		if (isAutoplayCancelled) {
			// re-run autoplay if vibration
#ifdef _AUDIBLE_EXECUTION_ENABLED
			buzzer_mute();
			buzzer_setTone(toneA4);
			buzzer_setDuty(50);
			for (int audibleExeForCountVar = 0; audibleExeForCountVar < 5; audibleExeForCountVar++) {
				buzzer_unmute();
				core_call_delayms(500);
				buzzer_mute();
				core_call_delayms(500);
			}
#endif
			if (periph_isVibration()) {
				autoDrive();
			}
			else core_call_delayms(50);
		}
	}
}

static core_statRetTypeDef app_pendingOpTimeoutHandler() {
	sg90_setAngle(SG90_MOTOR_A, SNACK_ANG_RDY);
	return OK;

}

core_statRetTypeDef app_secTimCallbackHandler() {
	if (skdIsSet) {
		if (--skdWaitTime <= 0) {
			flagSkdTimeElapsed = TRUE;
			skdWaitTime = 0;
		}
		else flagSkdTimeElapsed = FALSE;
	}
	if (catSearchIsSet) {
		if (--catSearchWaitTime <= 0) {
			flagCatSearchTimeout = TRUE;
			catSearchWaitTime = 0;
		}
		else flagCatSearchTimeout = FALSE;
	}
	if (vibWaitIsSet) {
		if (--vibWaitTime <= 0) {
			flagVibWaitTimeout = TRUE;
			vibWaitTime = 0;
		}
		else flagVibWaitTimeout = FALSE;
	}
	if (sndRptIsSet) {
		if (sndRptOutputStat == TRUE) {
			buzzer_mute();
			sndRptOutputStat = FALSE;
		}
		else {
			buzzer_unmute();
			sndRptOutputStat = TRUE;
		}
	}
	return OK;
}

void app_start() {
	if (initState == TRUE) {
#ifdef _TEST_MODE_ENABLED
		core_dbgTx("\r\n?DETECTED APP RESTART\r\n");
		while (1) {

		}
#endif
	}

	uint8_t *pu8 = &opcodePendingOp;
	core_statRetTypeDef(*phf)() = &app_pendingOpTimeoutHandler;
#ifdef _TEST_MODE_ENABLED
	if (core_call_pendingOpRegister(pu8, phf) != OK) {
		core_dbgTx("\r\n?FAILED TO REGISTER PENDING OP HANDLER FUNCTION OF APP\r\n");
		while (1) {

		}
	}
	if (core_call_secTimIntrRegister(&app_secTimCallbackHandler) != OK) {
		core_dbgTx("\r\n?FAILED TO REGISTER SECOND TIMER INTR HANDLER FUNCTION OF APP\r\n");
		while (1) {

		}
	}
#else
	core_call_pendingOpRegister(pu8, phf);
	core_call_secTimIntrRegister(&app_secTimCallbackHandler);
#endif
	core_dtaStruct_queueU8init(&patternQueue);
	speed = 2; // initial value is normal
	skdSpd = 0;
	skdDuration = 0;
	skdSnackIntv = 0;
	skdIsSet = FALSE;
	initState = TRUE;

	for (int i = 0; i < 20; i++) { // call ir sensor func and rpi pin recv 20 times to avoid error
		rpi_foundCat();
		periph_isVibration();
		core_call_delayms(20);
	}

	core_call_delayms(300);
	buzzer_setTone(toneA5); // notify boot success
	buzzer_setDuty(50);
	buzzer_unmute();
	core_call_delayms(250);
	buzzer_mute();

	/*
	// motor speed test: PASS
	l298n_enable();
	l298n_setRotation(L298N_MOTOR_A, L298N_CW);
	l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
	uint8_t spd = 9;
	uint8_t beep = 1;
	while(1) {
		l298n_setSpeed(L298N_MOTOR_A, spd);
		l298n_setSpeed(L298N_MOTOR_B, spd);
		for(int i = 0; i < beep; i++) {
			buzzer_unmute();
			core_call_delayms(200);
			buzzer_mute();
		}
		core_call_delayms(2000);
		if (spd > 90) spd = 9;
		else spd += 10;
		if (beep > 8) beep = 1;
		else beep++;
	}
	l298n_disable();
	*/

	/*
	// laser test: PASS
	while (1) {
		periph_laser_on();
		core_call_delayms(500);
		periph_laser_off();
		core_call_delayms(1500);
	}
	*/

	/*
	// prox snsr test: PASS
	while (1) {
		core_call_delayms(1000);
		if (periph_irSnsrChk(IR_SNSR_MODE_FIND) == IR_SNSR_NEAR) {
			for (int i = 0; i < 5; i++) {
				buzzer_unmute();
				core_call_delayms(200);
				buzzer_mute();
			}
		}
		if (periph_irSnsrChk(IR_SNSR_MODE_FIND) == IR_SNSR_ERR) {
			buzzer_unmute();
			core_call_delayms(5000);
			buzzer_mute();
			while (1) {

			}
		}
	}
	*/

	// pin comm test: PASS
	/*
	while (1) {
		if (rpi_foundCat() == TRUE) {
			for (int i = 0; i < 5; i++) {
				buzzer_unmute();
				core_call_delayms(200);
				buzzer_mute();
				core_call_delayms(200);
			}
		}
	}
	*/

	/*
	// servo test: DIDN'T TEST
	sg90_enable(SG90_MOTOR_A, SNACK_ANG_RDY);
	while (1) {
		core_call_delayms(2000);
		sg90_setAngle(SG90_MOTOR_A, SNACK_ANG_GIVE);
		core_call_delayms(2000);
		sg90_setAngle(SG90_MOTOR_A, SNACK_ANG_RDY);
	}
	*/

	appMain();
}
