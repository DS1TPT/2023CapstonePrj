/**
  *********************************************************************************************
  * NAME OF THE FILE : scheduler.c
  * BRIEF INFORMATION: auto-play schedule manager
  *
  * Copyright (c) 2023 Lee Geon-goo.
  * All rights reserved.
  *
  * This file is part of catCareBot.
  *
  *********************************************************************************************
  */

#include "scheduler.h"

struct PatternQueue {
	unsigned count;
	uint8_t queue[70];
};

struct Time {
	int32_t wait;
	int32_t duration;
	int isSetDuration;
	int isSetWait;
};

static struct PatternQueue QP;
static struct Time T;
static uint8_t speed = 0; // 0 ~ 4.
static uint8_t snackNum = 0;

void scheduler_init() {
	QP.count = 0;
	for (int i = 0; i < 70; i++) {
		QP.queue[i] = 0;
	}
	T.wait = 0;
	T.isSetWait = FALSE;
	T.duration = 0;
	T.isSetDuration = 0;
	speed = 2; // initial value is normal
	snackNum = 0;
}

void scheduler_setSpd(uint8_t spd) {
	speed = spd;
}

void scheduler_setSnack(uint8_t num) {
	snackNum = num;
}

int scheduler_enqueuePattern(uint8_t code) {
	if (QP.count == 70) return ERR;
	for (int i = 0; i < count - 1; i++)
		QP.queue[i + 1] = QP.queue[i];
	QP.queue[i] = code;
	QP.count++;
	return OK;
}

int scheduler_dequeuePattern() {
	if (!QP.count) return ERR;

	uint8_t tmp;
	tmp = QP.queue[count - 1];
	QP.queue[(count--) - 1] = 0;
	return OK;
}

int scheduler_setTime(int32_t sec) {
	if (T.isSetWait) return ERR;
	else {
		T.isSetWait = TRUE;
		T.wait = sec;
		return OK;
	}
}

int scheduler_setDuration(int32_t sec) {
	if (T.isSetDuration) return ERR;
	else {
		T.isSetDuration = TRUE;
		T.duration = sec;
		return OK;
	}
}

int scheduler_TimCallbackHandler() {
	if (T.isSet)
		if (!(--T.sec)) return 1;
	return 0;
}

int scheduler_isSet() {
	if (!T.isSet || !QP.count) return 0;
	else return 1;
}

uint8_t scheduler_getSpd() {
	return speed;
}

uint8_t scheduler_getSnack() {
	return snackNum;
}

int32_t scheduler_getInterval() {
	return (T.duration / QP.count);
}

/* ADD THIS TO MAIN.C
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance == TIM10)
		if(scheduler_TimCallbackHandler()) flagTimeElapsed = TRUE;
		else flagTimeElapsed = FALSE;
}
*/
