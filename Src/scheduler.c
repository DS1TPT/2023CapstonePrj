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

struct time {
	int32_t sec;
	int isSet;
};

struct patternQueue qp;
struct time t;
uint8_t speed; // 0 ~ 4.

void scheduler_init() {
	qp.count = 0;
	for (int i = 0; i < 70; i++) {
		qp.queue[i] = 0;
	}
	t.sec = 0;
	t.isSet = FALSE;
	spd = 2; // initial value is normal
}

void scheduler_setSpd(uint8_t spd) {
	speed = spd;
}

int scheduler_enqueuePattern(uint8_t code) {
	if (qp.count == 70) return ERR;
	for (int i = 0; i < count - 1; i++)
		qp.queue[i + 1] = qp.queue[i];
	qp.queue[i] = code;
	qp.count++;
	return OK;
}

int scheduler_dequeuePattern() {
	if (!qp.count) return ERR;

	uint8_t tmp;
	tmp = qp.queue[count - 1];
	qp.queue[(count--) - 1] = 0;
	return OK;
}

int scheduler_setTime(int32_t sec) {
	if (t.isSet) return 1;
}

int scheduler_TimCallbackHandler() {
	if (t.isSet)
		if (!(--t.sec)) return 1;
	return 0;
}

/* ADD THIS TO MAIN.C
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance == TIM10)
		if(scheduler_TimCallbackHandler()) flagTimeElapsed = TRUE;
		else flagTimeElapsed = FALSE;
}
*/
