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
 * SCHEDULE_SPEED: DWORD 00 00 00. Use SIGNED int32.
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
 * SCHEDULE_START: FF 00 FF 00 FF 00 FF
 * SCHEDULE_END: 00 FF 00 FF 00 FF 00
 * RESP: BYTE 00 00 00 00 00 00
 *
 * for more info, go to <https://github.com/DS1TPT/catCareBot> and check document pktDefs.pdf
 */

#include "main.h"
#include "rpicomm.h"

struct SerialDta uartdta;
uint8_t rxBuf[9] = { 0, };
uint8_t txBuf[8] = { 0, };

int rpi_getSerialDta(struct SerialDta dest) {
	if (uartdta.available) { // has new received data
		dest = uartdta; // copy from internal var to dest var
		uartdta.available = 0; // mark unavailable
		return 1;
	}
	else return 0;
}

uint8_t rpi_getPinDta() {
	uint8_t dta = 0;
	if (HAL_GPIO_ReadPin(RPI_SIG, FOUND_CAT) == GPIO_PIN_SET)
		dta |= 1;
	else dta &= ~0x01;
	// add more code as needed
	return dta;
}

void rpi_init() {
	uartdta.available = 0;
	for (int i = 0; i < 9; i++)
		rxBuf[i] = 0;
	pinDta = 0;
	HAL_UART_Receive_IT(&huart1, &rxBuf, 9);
}

void rpi_RxCpltCallbackHandler() {
	uartdta.available = 1; // mark available
	uartdta.type = rxBuf[0]; // copy data from buffer to internal var
	for (int i = 0; i < 8; i++)
		uartdta.container[i] = rxBuf[i + 1];
	for (int i = 0; i < 9; i++) // clr buf
		rxBuf[i] = 0;
	HAL_UART_Receive_IT(&huart1, &rxBuf, 9); // restart rx
}

int rpi_tcpipAvailable() { // returns zero if not available

}
int rpi_tcpipRespond() { // send RESP pkt to client app. returns 0 on success

}

/* ADD THIS TO MAIN.C
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		rpi_RxCpltCallbackHandler();
		flagRxCplt = TRUE;
	}
}
*/
