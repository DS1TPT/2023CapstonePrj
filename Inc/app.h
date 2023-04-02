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

#define OP_SNACK_RET_MOTOR 0x01
#define OP_RPI_PIN_IO_SEND_RESET 0x02

void app_start(UART_HandleTypeDef* pHandleDbgUART); // start application. call this function in core
void app_opTimeout(uint8_t opcode);
void app_secTimCallbackHandler();

#endif
