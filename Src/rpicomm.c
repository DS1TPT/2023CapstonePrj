/**
  *********************************************************************************************
  * NAME OF THE FILE : rpicomm.c
  * BRIEF INFORMATION: for communications between stm32f411re nucleo board
  * 			  	   and raspberry pi via GPIO pins and UART.
  *
  * Copyright (c) 2023 Lee Geon-goo.
  * All rights reserved.
  *
  * This file is part of catCareBot.
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

struct SerialDta UARTdta;
uint8_t rxBuf[9] = { 0, };
uint8_t txBuf[8] = { 0, };

int rpi_getSerialDta(struct SerialDta dest) {
	if (UARTdta.available) { // has new received data
		dest = UARTdta; // copy from internal var to dest var
		UARTdta.available = 0; // mark unavailable
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
	UARTdta.available = 0;
	for (int i = 0; i < 9; i++)
		rxBuf[i] = 0;
	pinDta = 0;
	HAL_UART_Receive_IT(&huart1, &rxBuf, 9);
}

void rpi_RxCpltCallbackHandler() {
	UARTdta.available = 1; // mark available
	UARTdta.type = rxBuf[0]; // copy data from buffer to internal var
	for (int i = 0; i < 8; i++)
		UARTdta.container[i] = rxBuf[i + 1];
	for (int i = 0; i < 9; i++) // clr buf
		rxBuf[i] = 0;
	HAL_UART_Receive_IT(&huart1, &rxBuf, 9); // restart rx
}

int rpi_serialDtaAvailable() { // returns zero if not available
	if (UARTdta.available) return 1;
	else return 0;
}
int rpi_tcpipRespond(uint8_t isErr) { // send RESP pkt to client app. returns OK on success
	uint8_t buf[8] = { 0, };
	buf[0] = 0xFF;
	if (!isErr) buf[1] = 0xFF;
	uint32_t txRes = HAL_UART_Transmit(&huart1, buf, 8, 20);
	if (txRes == HAL_OK) return OK;
	else return ERR;
}

/* ADD THIS TO MAIN.C
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		rpi_RxCpltCallbackHandler();
	}
}
*/
