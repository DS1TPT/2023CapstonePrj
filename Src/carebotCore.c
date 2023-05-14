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
#include "buzzer.h"
#include "sg90.h"

#if defined _TEST_MODE_SEND_VIA_STLINK_SWO
const _Bool isDebugModeDef = TRUE;
#include <stdio.h>
#elif defined _TEST_MODE_SEND_VIA_UART
const _Bool isDebugModeDef = TRUE;
#else
const _Bool isDebugModeDef = FALSE;
#endif

// system variables
//static uint8_t flagTimeElapsed = FALSE;
//static uint8_t flagHibernate = FALSE;
static _Bool initState = FALSE;
static _Bool secTimEna = FALSE;

static uint8_t pendedOpcodeMem = 0; // bit-masked
static core_statRetTypeDef (*arrRegdPendingOpHandlerFunc[8])();
static core_statRetTypeDef (*arrRegdSecTimIntrHandlerFunc[8])();
static core_statRetTypeDef (*arrRegdUartIntrHandlerFunc[8])(UART_HandleTypeDef*);
static volatile uint16_t timeMem[8] = { 0, };
static _Bool timEna = FALSE;

static TIM_HandleTypeDef* pSecTimHandle = NULL;
static TIM_HandleTypeDef* pMillisecTimHandle = NULL;
static UART_HandleTypeDef* pDbgUartHandle = NULL;

/* basic functions */

static unsigned getBitPos(uint8_t u8) {
	unsigned pos = 0;
	uint8_t dta = u8;
	while(1) {
		if (dta == 0x01) break;
		dta = dta >> 1;
		pos++;
	}
	return pos;
}

/* application support functions */

_Bool core_dtaStruct_queueU8isEmpty(struct dtaStructQueueU8 *structQueue) {
	return (structQueue->index == -1);
}

_Bool core_dtaStruct_queueU8isFull(struct dtaStructQueueU8 *structQueue) {
	return (structQueue->index == DTA_STRUCT_QUEUE_SIZE - 1);
}

void core_dtaStruct_queueU8init(struct dtaStructQueueU8 *structQueue) {
	structQueue->index = -1;
	for (int i = 0; i < DTA_STRUCT_QUEUE_SIZE; i++) {
		structQueue->queue[i] = 0;
	}
}

core_statRetTypeDef core_dtaStruct_enqueueU8(struct dtaStructQueueU8 *structQueue, uint8_t data) {
	if (structQueue->index == DTA_STRUCT_QUEUE_SIZE - 1) return ERR;
	structQueue->queue[++(structQueue->index)] = data;
	return OK;
}

core_statRetTypeDef core_dtaStruct_dequeueU8(struct dtaStructQueueU8 *structQueue, uint8_t *pDest) {
	if (structQueue->index == -1) {
		*pDest = 0;
		return ERR;
	}
	*pDest = structQueue->queue[structQueue->index];
	structQueue->queue[structQueue->index--] = 0;
	return OK;
}

_Bool core_dtaStruct_stackU8isEmpty(struct dtaStructStackU8 *structStack) {
	return (structStack->index == -1);
}

_Bool core_dtaStruct_stackU8isFull(struct dtaStructStackU8 *structStack) {
	return (structStack->index == DTA_STRUCT_STACK_SIZE - 1);
}

void core_dtaStruct_stackU8init(struct dtaStructStackU8 *structStack) {
	structStack->index = -1;
	for (int i = 0; i < DTA_STRUCT_STACK_SIZE; i++) {
		structStack->stack[i] = 0;
	}
}

core_statRetTypeDef core_dtaStruct_pushU8(struct dtaStructStackU8 *structStack, uint8_t data) {
	if (structStack->index == DTA_STRUCT_STACK_SIZE - 1) return ERR;
	structStack->stack[++(structStack->index)] = data;
	return OK;
}

core_statRetTypeDef core_dtaStruct_popU8(struct dtaStructStackU8 *structStack, uint8_t *pDest) {
	if (structStack->index == -1) {
		*pDest = 0;
		return ERR;
	}
	*pDest = structStack->stack[structStack->index];
	structStack->stack[structStack->index--] = 0;
	return OK;
}


/* delayed operation support functions */

core_statRetTypeDef core_call_pendingOpRegister(uint8_t *opcodeDest, core_statRetTypeDef(*pHandlerFunc)()) {
	for (int i = 0; i < 8; i++) {
		if (arrRegdPendingOpHandlerFunc[i] == NULL) {
			arrRegdPendingOpHandlerFunc[i] = pHandlerFunc;
			return OK;
		}
	}
	return ERR;
}

core_statRetTypeDef core_call_pendingOpUnregister(uint8_t opcode) {
	if (arrRegdPendingOpHandlerFunc[opcode] == NULL) return ERR;
	else {
		arrRegdPendingOpHandlerFunc[opcode] = NULL;
		return OK;
	}
}

core_statRetTypeDef core_call_pendingOpAdd(uint8_t opcode, uint16_t milliseconds) {
	if (!opcode) return ERR;
	// add pending operation, which will be executed after n milliseconds
	if (timEna == FALSE) { // enable timer if timer is off
		HAL_TIM_Base_Start_IT(pMillisecTimHandle);
		timEna = TRUE;
	}
	if (pendedOpcodeMem & opcode) return ERR; // already have same operation postponed
	else if (arrRegdPendingOpHandlerFunc[opcode] == NULL) return ERR; // opcode not registered
	else {
		pendedOpcodeMem = pendedOpcodeMem & opcode;
		timeMem[getBitPos(opcode)] = milliseconds;
	}
    return OK;
}

core_statRetTypeDef core_call_pendingOpTimeReset(uint8_t opcode, uint16_t milliseconds) {
    if (!opcode) return ERR;
    // check if opcode is not set
    if (timEna == FALSE) return ERR;
    if (!(pendedOpcodeMem & opcode)) return ERR; // commanded opcode is not set
    else if (arrRegdPendingOpHandlerFunc[opcode] == NULL) return ERR; // opcode not registered
    else if (timeMem[getBitPos(opcode)] <= 0) return ERR; // commanded time had been elapsed already

    // reset time
    timeMem[getBitPos(opcode)] = milliseconds;
    return OK;

}

core_statRetTypeDef core_call_pendingOpExeImmediate(uint8_t opcode) {
    if (!opcode) return ERR;
    if (timEna == FALSE) return ERR;
    if ((pendedOpcodeMem & opcode) == 0) return ERR;
    else if (arrRegdPendingOpHandlerFunc[opcode] == NULL) return ERR; // opcode not registered
    else if (timeMem[getBitPos(opcode) <= 0]) return ERR;

    timeMem[getBitPos(opcode)] = 0;
    pendedOpcodeMem = pendedOpcodeMem & ~opcode;
    arrRegdPendingOpHandlerFunc[opcode]();
    return OK;
}

void core_call_pendingOpCancel(uint8_t opcode) {
	if (!opcode) return;
	timeMem[getBitPos(opcode)] = 0;
	pendedOpcodeMem = pendedOpcodeMem & ~opcode;
}

#if defined _TEST_MODE_SEND_VIA_STLINK_SWO
int _write(int file, char *ptr, int len) {
	for (int i= 0; i < len; i++) {
		ITM_SendChar(*ptr++);
	}
	return len;
}

core_statRetTypeDef core_dbgTx(char *sz) {
	uint16_t size = 0;
	uint8_t* pu8 = (uint8_t*)sz;
	if (sz == NULL || *sz == 0) return ERR;
	while (1) { // get length
		if (*(pu8++) == 0) break;
		else size++;
	}
	if (!size) return ERR;

	int retval = printf(sz);
	if (retval != 0) return OK;
	else return ERR;
}
#elif defined _TEST_MODE_SEND_VIA_UART
core_statRetTypeDef core_dbgTx(char *sz) {
	uint16_t size = 0;
	uint32_t timeout = 0;
	uint8_t* pu8 = (uint8_t*)sz;
	if (sz == NULL || *sz == 0) return ERR;
	while (1) { // get length
		if (*(pu8++) == 0) break;
		else size++;
	}
	if (!size) return ERR;

	timeout = (uint32_t)(size / (pDbgUartHandle->Init.BaudRate / 1000) + 10);
	if (timeout <= 20) timeout = 20;

	HAL_StatusTypeDef retval = HAL_UART_Transmit(pDbgUartHandle, (uint8_t*)sz, size, timeout);
	if (retval == HAL_OK) return OK;
	else return ERR;
}
#else
core_statRetTypeDef core_dbgTx(char *sz) {
	return OK;
}
#endif

core_statRetTypeDef core_call_secTimIntrRegister(core_statRetTypeDef(*pHandlerFunc)()) {
	for (int i = 0; i < 8; i++) {
		if (arrRegdSecTimIntrHandlerFunc[i] == pHandlerFunc) return ERR; // function already registered
		else if (arrRegdSecTimIntrHandlerFunc[i] == NULL) {
			arrRegdSecTimIntrHandlerFunc[i] = pHandlerFunc;
			return OK;
		}
	}
	return ERR; // array is full
}

core_statRetTypeDef core_call_secTimIntrUnregister(core_statRetTypeDef(*pHandlerFunc)()) {
	for (int i = 0; i < 8; i++) {
		if (arrRegdSecTimIntrHandlerFunc[i] == pHandlerFunc) {
			arrRegdSecTimIntrHandlerFunc[i] = NULL;
			return OK;
		}
	}
	return ERR; // function not found
}

void core_call_delayms(uint32_t ms) {
	HAL_Delay(ms);
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
	buzzer_init();
	initState = TRUE;

	pendedOpcodeMem = 0;
	for (int i = 0; i < 8; i++) {
		arrRegdPendingOpHandlerFunc[i] = NULL;
	}

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


static void millisecTimCallbackHandler() {
	for (int i = 0; i < 8; i++) {
		if (timeMem[i] > 0) {
			timeMem[i]--; // decrement time
			if (!timeMem[i]) {
				pendedOpcodeMem = pendedOpcodeMem & ~(0x01 << i);
#ifdef _TEST_MODE_ENABLED
				core_statRetTypeDef retval = arrRegdPendingOpHandlerFunc[i]();
				if (retval != OK) {
					core_dbgTx("\r\n?PENDING OPERATION HANDLER FUNCTION RETURNED NON-OK VALUE TO CORE\r\n");
					while (1) {

					}
				}
#else
				arrRegdPendingOpHandlerFunc[i]();
#endif
			}
		}
	}
}

core_statRetTypeDef core_call_uartHandlerRegister(core_statRetTypeDef(*pHandlerFunc)(UART_HandleTypeDef *huart)) {
	for (int i = 0; i < 8; i++) {
		if (arrRegdUartIntrHandlerFunc[i] == pHandlerFunc) return ERR; // function already registered
		else if (arrRegdUartIntrHandlerFunc[i] == NULL) {
			arrRegdUartIntrHandlerFunc[i] = pHandlerFunc;
			return OK;
		}
	}
	return ERR; // array is full
}

core_statRetTypeDef core_call_uartHandlerUnregister(core_statRetTypeDef(*pHandlerFunc)(UART_HandleTypeDef *huart)) {
	for (int i = 0; i < 8; i++) {
		if (arrRegdUartIntrHandlerFunc[i] == pHandlerFunc) {
			arrRegdUartIntrHandlerFunc[i] = NULL;
			return OK;
		}
	}
	return ERR; // function not found
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	for (int i = 0; i < 8; i++) {
		if (arrRegdUartIntrHandlerFunc[i] != NULL) arrRegdUartIntrHandlerFunc[i](huart);
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance == pSecTimHandle->Instance) { // 1s sys tim
		for (int i = 0; i < 8; i++) {
			if (arrRegdSecTimIntrHandlerFunc[i] != NULL) arrRegdSecTimIntrHandlerFunc[i]();
		}
	}
	else if (htim->Instance == pMillisecTimHandle->Instance) { // 1ms sys tim(for postponed ops)
		millisecTimCallbackHandler();
	}
}
