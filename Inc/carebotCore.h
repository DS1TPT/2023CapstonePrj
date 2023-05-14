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

/*
 * delayed operation handler functions, and second timer interrupt handler functions MUST be declared in following type:
 * core_statRetTypeDef functionName()
 * When test mode is enabled, the core will halt firmware execution if a handler function returns non-OK value.
 * Max number of handler functions for delayed opration and second timer interrupt is 8.
 */

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

/* for tesing. defining this will make robot to send program execution status via Serial. */
//#define _TEST_MODE_ENABLED
// un-comment this to use UART
//#define _TEST_MODE_SEND_VIA_UART
// un-comment this to use ST-LINK SWO
//#define _TEST_MODE_SEND_VIA_STLINK_SWO

/* definitions */
#define DTA_STRUCT_QUEUE_SIZE 128
#define DTA_STRUCT_STACK_SIZE 128

typedef enum {
	OK = 0x00U,
	ERR = 0xFFU
} core_statRetTypeDef;

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
_Bool core_dtaStruct_queueU8isEmpty(struct dtaStructQueueU8 *structQueue);
_Bool core_dtaStruct_queueU8isFull(struct dtaStructQueueU8 *structQueue);
void core_dtaStruct_queueU8init(struct dtaStructQueueU8 *structQueue);
core_statRetTypeDef core_dtaStruct_enqueueU8(struct dtaStructQueueU8 *structQueue, uint8_t data);
core_statRetTypeDef core_dtaStruct_dequeueU8(struct dtaStructQueueU8 *structQueue, uint8_t *pDest);
_Bool core_dtaStruct_stackU8isEmpty(struct dtaStructStackU8 *structStack);
_Bool core_dtaStruct_stackU8isEmpty(struct dtaStructStackU8 *structStack);
void core_dtaStruct_stackU8init(struct dtaStructStackU8 *structStack);
core_statRetTypeDef core_dtaStruct_pushU8(struct dtaStructStackU8 *structStack, uint8_t data);
core_statRetTypeDef core_dtaStruct_popU8(struct dtaStructStackU8 *structStack, uint8_t *pDest);

// delayed operation support
core_statRetTypeDef core_call_pendingOpRegister(uint8_t *opcodeDest, core_statRetTypeDef(*pHandlerFunc)());
core_statRetTypeDef core_call_pendingOpUnregister(uint8_t opcode);
core_statRetTypeDef core_call_pendingOpAdd(uint8_t opcode, uint16_t milliseconds); // opcode 0 will be ignored. returns ERR on error, OK on success
core_statRetTypeDef core_call_pendingOpTimeReset(uint8_t opcode, uint16_t milliseconds); // re-configure waiting time of an already-postponed operation
core_statRetTypeDef core_call_pendingOpExeImmediate(uint8_t opcode); // immediately execute postponed operation and remove from pending list
void core_call_pendingOpCancel(uint8_t opcode);

// time support
core_statRetTypeDef core_call_secTimIntrRegister(core_statRetTypeDef(*pHandlerFunc)());
core_statRetTypeDef core_call_secTimIntrUnregister(core_statRetTypeDef(*pHandlerFunc)());
void core_call_delayms(uint32_t ms);

// misc support
core_statRetTypeDef core_call_uartHandlerRegister(core_statRetTypeDef(*pHandlerFunc)(UART_HandleTypeDef *huart));
core_statRetTypeDef core_call_uartHandlerUnregister(core_statRetTypeDef(*pHandlerFunc)(UART_HandleTypeDef *huart));
core_statRetTypeDef core_dbgTx(char *sz);

//void core_callbackHandler();

#endif
