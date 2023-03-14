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

// core will control every hardware after the main calls start function.

#include "main.h"

#define CORE_SEC_TIM TIM10

void core_setHandle(TIM_HandleTypeDef* ph); // pass timer with 1000ms interval
void core_start(); // this should be called only once by main.c
void core_callOp(uint8_t opcode); // opman will call this

#endif
