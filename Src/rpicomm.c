/**
  *********************************************************************************************
  * NAME OF THE FILE : rpicomm.c
  * BRIEF INFORMATION: for communications between stm32f411re nucleo board
  * 			  	   and raspberry pi via GPIO pins and UART.
  *
  * !ATTENTION!
  *
  * Copyright (c) 2023 Lee Geon-goo.
  * All rights reserved.
  *
  * This file is part of catCareBot.
  * catCareBot is free software: you can redistribute it and/or modify it under the
  * terms of the GNU General Public License as published by the Free Software Foundation,
  * either version 3 of the License, or (at your option) any later version.
  *
  * catCareBot is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
  * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  * See the GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License along with catCareBot.
  * If not, see <https://www.gnu.org/licenses/>.
  *
  *********************************************************************************************
  */

/*
 * dta type header 8b.
 * dta container 7B
 * container dta info below:
 * NO: any data in container will be ignored.
 * CONN:DIS\0\0\0\0: this will turn on sleep or stand-by mode.
 * 		if no schedule is set, stand-by mode will be active.
 * 		if schedule is set, sleep mode will be active.
 * 		this behavior is for keeping pattern stack active.
 * 		CON\0\0\0\0: this will set RUN mode.
 * SCHEDULE_TIME: DWORD 00 00 00. Use uint32.
 * SCHEDULE_PATTERN: pattern code, 4b per pattern.
 * 					therefore, a packet can contain up to 14 patterns.
 * 					NOTE: pattern stack can contain up to 70 patterns.
 * 						  this type of data can be sent 5 times in a row.
 * SCHEDULE_SNACK: number of times the owner want to give treat.
 * 				  format: BYTE 00 00 00 00 00 00 uint8
 * SCHEDULE_MODE: level of speed and aggressiveness: diffident - light - normal - active - energetic
 * 				  format: BYTE 00 00 00 00 00 00. uint8
 * 				  SYS: system commands. not defined this time
 * MANUAL_CTRL: format: BYTE 00 00 00 00 00 00. uint8
 * 0x80: undefined
 */

#include "main.h"
#include "rpicomm.h"

struct SerialDta uartdta;
uint8_t pindta = 0;
uint8_t rxBuf[9] = 0;

int rpi_getSerialDta(struct SerialDta dest) {
	if (uartdta.available) { // has new received data
		uartdta.type = rxBuf[0]; // copy from buf to internal var
		for (int i = 0; i < 8; i++)
			uartdta.container[i] = rxBuf[i + 1];
		dest = uartdta; // copy from internal buffer to dest
		uartdta.available = 0;
		for (int i = 0; i < 9; i++) // clr buf
			rxBuf[i] = 0;
		return 1;
	}
	else return 0;
}

void rpi_init() {
	uartdta.available = 0;
	for (int i = 0; i < 9; i++)
		rxBuf[i] = 0;
	pinDta = 0;
	HAL_UART_Receive_IT(&huart1, &rxBuf, 9);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		uartdta.available = 1;
		HAL_UART_Receive_IT(&huart1, &rxBuf, 9);
	}
}
