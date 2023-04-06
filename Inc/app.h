/**
  *********************************************************************************************
  * NAME OF THE FILE : app.h
  * BRIEF INFORMATION: robot application
  *
  * Copyright (c) 2023 Lee Geon-goo.
  * All rights reserved.
  *
  * This file is part of catCareBot.
  *
  *********************************************************************************************
  */

#ifndef INC_APP
#define INC_APP

#include "main.h"

#define _ASCII_NUMBER_FOR_PATTERN_CODE

void app_start(); // start application. call this function in core
void app_opTimeout(uint8_t opcode);

#endif
