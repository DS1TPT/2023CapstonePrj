/**
  *********************************************************************************************
  * NAME OF THE FILE : opman.c
  * BRIEF INFORMATION: pending operation manager
  *                    this manager does NOT allow to postpone same operation multiple times
  *
  * Copyright (c) 2023 Lee Geon-goo.
  * All rights reserved.
  *
  * This file is part of catCareBot.
  *
  *********************************************************************************************
  */

#include "carebotCore.h"
#include "opman.h"

static uint8_t opcodeMem = 0;
static uint16_t timeMem[8] = { 0, };
static uint8_t timEna = FALSE;

static unsigned getBitPos(uint8_t u8) {
	unsigned pos = 0;
	while(1) {
		if (u8 == 0x01) break;
		u8 = u8 >> 1;
		pos++;
	}
	return pos;
}

void opman_init() {
	opcodeMem = 0;
	for(int i = 0; i < 8; i++)
		timeMem[i] = 0;
	timEna = FALSE;
}

void opman_addPendingOp(uint8_t opcode, uint16_t milliseconds) {
	if (!opcode) return;
	// add pending operation, which will be executed after n milliseconds
	if (timEna == FALSE) { // enable timer if timer is off
		HAL_TIM_Base_Start_IT(OPMAN_TIM_HANDLE);
		timEna = TRUE;
	}
	if (opcodeMem & opcode) return; // already have same operation postponed
	else {
		opcodeMem = opcodeMem & opcode;
		timeMem[getBitPos(opcode)] = milliseconds;
	}
}

void opman_canclePendingOp(uint8_t opcode) {
	if (!opcode) return;
	timeMem[getBitPos(opcode)] = 0;
	opcodeMem = opcodeMem & ~opcode;
}

void opman_callbackHandler() {
	for (int i = 0; i < 8; i++) {
		if (timeMem[i] > 0) {
			timeMem[i]--; // decrement time
			if (!timeMem[i]) {
				opcodeMem = opcodeMem & ~(0x01 << i);
				core_callOp(0x01 << i);
			}
		}
	}
}
