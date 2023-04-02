/**
  *********************************************************************************************
  * NAME OF THE FILE : carebotCore.h
  * BRIEF INFORMATION: robot program core
  *
  * Copyright (c) 2023 Lee Geon-goo.
  * All rights reserved.
  *
  * This file is part of catCareBot.
  *
  *********************************************************************************************
  */

#ifndef CAREBOTCORE_H
#define CAREBOTCORE_H

#include "main.h"

// for testing this to test connection(received uart data will be sent to debug uart port)
#define _TEST_MODE_ENABLED

/* definitions */
#define DTA_STRUCT_QUEUE_SIZE 128
#define DTA_STRUCT_STACK_SIZE 128

/* structures */
struct dtaStructQueueU8 {
	int index;
	uint8_t queue[DTA_STRUCT_QUEUE_SIZE];
};

struct dtaStructStackU8 {
	int index;
	uint16_t stack[DTA_STRUCT_STACK_SIZE];
};

/* exported functions */

// initialization related functions
void core_setHandleMillisec(TIM_HandleTypeDef* ph);
void core_setHandleSec(TIM_HandleTypeDef* ph); // pass timer with 1000ms interval
void core_setHandleDebugUART(UART_HandleTypeDef* ph); // pass uart handle to get debugging info
void core_start(); // this should be called only once by main.c

// application support functions
// data structures support
_Bool core_dtaStruct_queueU8isEmpty(dtaStructQueueU8 *structQueue);
_Bool core_dtaStruct_queueU8isFull(dtaStructQueueU8 *structQueue);
void core_dtaStruct_queueU8init(dtaStructQueueU8 *structQueue);
int core_dtaStruct_enqueueU8(dtaStructQueueU8 *structQueue, uint8_t data);
int core_dtaStruct_dequeueU8(dtaStructQueueU8 *structQueue, uint8_t *pDest);
_Bool core_dtaStruct_stackU8isEmpty(dtaStructStackU8 *structStack);
_Bool core_dtaStruct_stackU8isEmpty(dtaStructStackU8 *structStack);
void core_dtaStruct_stackU8init(dtaStructStackU8 *structStack);
int core_dtaStruct_pushU8(dtaStructStackU8 *structStack, uint8_t data);
int core_dtaStruct_popU8(dtaStructStackU8 *structStack, uint8_t *pDest);

// delayed operation support
void core_addPendingOp(uint8_t opcode, uint16_t milliseconds); // opcode 0 will be ignored
void core_canclePendingOp(uint8_t opcode);

void core_callbackHandler();

#endif
