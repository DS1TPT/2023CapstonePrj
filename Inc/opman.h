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

#ifndef OPMAN_H
#define OPMAN_H

#include "main.h"

// opcode definitions
#define OP_SNACK_RET_MOTOR 0x01
#define OP_RPI_PIN_IO_SEND_RESET 0x02

#define OPMAN_TIM TIM11

void opman_setHandle(TIM_HandleTypeDef* ph);
void opman_init();
void opman_addPendingOp(uint8_t opcode, uint16_t milliseconds); // opcode 0 will be ignored
void opman_canclePendingOp(uint8_t opcode);
void opman_callbackHandler();


#endif
