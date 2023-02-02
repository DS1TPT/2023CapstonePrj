/**
  *********************************************************************************************
  * NAME OF THE FILE : scheduler.c
  * BRIEF INFORMATION: auto-play schedule manager
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

#include "scheduler.h"

struct PatternQueue {
	unsigned count;
	uint8_t queue[70];
};

struct Time {
	int32_t sec;
	int isSet;
};

struct PatternQueue QP;
struct Time T;
uint8_t speed; // 0 ~ 4.

void scheduler_init() {
	QP.count = 0;
	for (int i = 0; i < 70; i++) {
		QP.queue[i] = 0;
	}
	T.sec = 0;
	T.isSet = FALSE;
	spd = 2; // initial value is normal
}

void scheduler_setSpd(uint8_t spd) {
	speed = spd;
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
	if (T.isSet) return 1;
	else {
		T.isSet = TRUE;
		T.sec = sec;
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

/* ADD THIS TO MAIN.C
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance == TIM10)
		if(scheduler_TimCallbackHandler()) flagTimeElapsed = TRUE;
		else flagTimeElapsed = FALSE;
}
*/
