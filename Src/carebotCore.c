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

#include "app.h"
#include "carebotCore.h"
#include "carebotPeripherals.h"
#include "rpicomm.h"
#include "l298n.h"
#include "sg90.h"

// system variables
//static uint8_t flagTimeElapsed = FALSE;
//static uint8_t flagHibernate = FALSE;
static uint8_t initState = FALSE;
static uint8_t secTimEna = FALSE;

static uint8_t opcodeMem = 0;
static uint16_t timeMem[8] = { 0, };
static uint8_t timEna = FALSE;

static TIM_HandleTypeDef* pSecTimHandle = NULL;
static TIM_HandleTypeDef* pMillisecTimHandle = NULL;
static UART_HandleTypeDef* pDbgUartHandle = NULL;

/* basic functions */

static unsigned getBitPos(uint8_t u8) {
	unsigned pos = 0;
	while(1) {
		if (u8 == 0x01) break;
		u8 = u8 >> 1;
		pos++;
	}
	return pos;
}

/* application support functions */

_Bool core_dtaStruct_queueU8isEmpty(dtaStructQueueU8 *structQueue) {
	return (structQueue->index == -1);
}

_Bool core_dtaStruct_queueU8isFull(dtaStructQueueU8 *structQueue) {
	return (structQueue->index == DTA_STRUCT_QUEUE_SIZE - 1);
}

void core_dtaStruct_queueU8init(dtaStructQueueU8 *structQueue) {
	structQueue->index = -1;
	for (int i = 0; i < DTA_STRUCT_QUEUE_SIZE; i++) {
		structQueue->queue[i] = 0;
	}
}

int core_dtaStruct_enqueueU8(dtaStructQueueU8 *structQueue, uint8_t data) {
	if (structQueue->index == DTA_STRUCT_QUEUE_SIZE - 1) return ERR;
	for (int i = 0; i < structQueue->index; i++) {
		structQueue->queue[i + 1] = structQueue->queue[i];
	}
	structQueue->queue[++(structQueue->index)] = data;
	return OK;
}

int core_dtaStruce_dequeueU8(dtaStructQueueU8 *structQueue, uint8_t *pDest) {
	if (structQueue->count == -1) {
		*pDest = 0;
		return ERR;
	}
	*pDest = structQueue->queue[structQueue->count];
	structQueue->queue[structQueue->count--] = 0;
	return OK;
}

_Bool core_dtaStruct_stackU8isEmpty(dtaStructStackU8 *structStack) {
	return (structStack->index == -1);
}

_Bool core_dtaStruct_stackU8isFull(dtaStructStackU8 *structStack) {
	return (structStack->index == DTA_STRUCT_STACK_SIZE - 1);
}

void core_dtaStruct_stackU8init(dtaStructStackU8 *structStack) {
	structStack->index = -1;
	for (int i = 0; i < DTA_STRUCT_STACK_SIZE; i++) {
		structStack->stack[i] = 0;
	}
}

int core_dtaStruct_pushU8(dtaStructStackU8 *structStack, uint8_t data) {
	if (structStack->index == DTA_STRUCT_STACK_SIZE - 1) return ERR;
	structStack->stack[++index] = data;
	return OK;
}

int core_dtaStruct_popU8(dtaStructStackU8 *structStack, uint8_t *pDest) {
	if (structStack->index == -1) {
		*pDest = 0;
		return ERR;
	}
	*pDest = structStack->stack[structStack->index];
	structStack->stack[structStack->index--] = 0;
	return OK;
}

/* delayed operation support functions */

void core_addPendingOp(uint8_t opcode, uint16_t milliseconds) {
	if (!opcode) return;
	// add pending operation, which will be executed after n milliseconds
	if (timEna == FALSE) { // enable timer if timer is off
		HAL_TIM_Base_Start_IT(pMillisecTimHandle);
		timEna = TRUE;
	}
	if (opcodeMem & opcode) return; // already have same operation postponed
	else {
		opcodeMem = opcodeMem & opcode;
		timeMem[getBitPos(opcode)] = milliseconds;
	}
}

void core_canclePendingOp(uint8_t opcode) {
	if (!opcode) return;
	timeMem[getBitPos(opcode)] = 0;
	opcodeMem = opcodeMem & ~opcode;
}

void core_setHandleDebugUART(UART_HandleTypeDef* ph) {
	pDbgUartHandle = ph;
}

void core_setHandleSec(TIM_HandleTypeDef* ph) {
	pSecTimHandle = ph;
}

void core_setHandleMillisec(TIM_HandleTypeDef* ph) {
	pMillisecTimHandle = ph;
}

void core_start() {
	if (initState) app_start(); // skip initialization
	// initialization
	periph_init();
	rpi_init();
	l298n_init();
	sg90_init();
	initState = TRUE;

	opcodeMem = 0;
	for(int i = 0; i < 8; i++)
		timeMem[i] = 0;
	timEna = FALSE;

	// start core
	if (secTimEna == FALSE) { // enable timer if timer is off
		HAL_TIM_Base_Start_IT(pSecTimHandle);
		secTimEna = TRUE;
	}
	app_start();
}


static void millisecTimcallbackHandler() {
	for (int i = 0; i < 8; i++) {
		if (timeMem[i] > 0) {
			timeMem[i]--; // decrement time
			if (!timeMem[i]) {
				opcodeMem = opcodeMem & ~(0x01 << i);
				app_opTimeout(0x01 << i);
			}
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		rpi_RxCpltCallbackHandler();
		//flagRxCplt = TRUE;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance == pSecTimHandle->Instance) { // 1s sys tim
		app_secTimCallbackHandler();
	}
	else if (htim->Instance == pMillisecTimHandle->Instance) { // 1ms sys tim(for postponed ops)
		millisecTimCallbackHandler();
	}
}
