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

#define OPMAN_TIM TIM11
#define OPMAN_TIM_HANDLE &htim11

void opman_init();
void opman_addPendingOp(uint8_t opcode, uint16_t milliseconds); // opcode 0 will be ignored
void opman_canclePendingOp(uint8_t opcode);
void opman_callbackHandler();


#endif
