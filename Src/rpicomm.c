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

#include "carebotCore.h"
#include "rpicomm.h"

static UART_HandleTypeDef* pUartHandle = NULL;

static struct SerialDta UARTdta;
static uint8_t rxBuf[DTA_LEN + 1] = { 0, };
//static uint8_t txBuf[8] = { 0, };

static uint8_t opcode = 0;

void rpi_setHandle(UART_HandleTypeDef* ph) {
	pUartHandle = ph;
}

int rpi_getSerialDta(struct SerialDta* pDest) {
	if (UARTdta.available) { // has new received data
		*pDest = UARTdta; // copy from internal var to dest var
		UARTdta.available = 0; // mark unavailable
		return 1;
	}
	else return 0;
}

_Bool rpi_foundCat() {
	if (HAL_GPIO_ReadPin(RPI_PIN_IN_PORT, RPI_PIN_IN_FOUNDCAT) == GPIO_PIN_SET) return TRUE;
	else return FALSE;

}

int rpi_serialDtaAvailable() { // returns zero if not available
	if (UARTdta.available) return 1;
	else return 0;
}

/*
int rpi_tcpipRespond(uint8_t isErr) { // send RESP pkt to client app. returns OK on success
	uint8_t buf[8] = { 0, };
	buf[0] = 0xFF;
	if (!isErr) buf[1] = 0xFF;
	uint32_t txRes = HAL_UART_Transmit(pUartHandle, buf, 8, 20);
	if (txRes == HAL_OK) return OK;
	else return ERR;
}
*/

void rpi_sendPin(int code) {
	core_call_pendingOpCancel(opcode);
	switch (code) {
	case RPI_PINCODE_O_SCHEDULE_EXE:
		HAL_GPIO_WritePin(RPI_PIN_OUT_PORT, RPI_PIN_OUT_SCHEDULE_EXE, GPIO_PIN_RESET);
		break;
		/*
	case RPI_PINCODE_O_SCHEDULE_END:
		HAL_GPIO_WritePin(RPI_PIN_OUT_PORT, RPI_PIN_OUT_SCHEDULE_END, GPIO_PIN_SET);
		break;
		*/
	case RPI_PINCODE_O_FIND_CAT_TIMEOUT:
		HAL_GPIO_WritePin(RPI_PIN_OUT_PORT, RPI_PIN_OUT_FIND_CAT_TIMEOUT, GPIO_PIN_RESET);
		break;
	}
	core_call_pendingOpAdd(opcode, RPI_PIN_SEND_WAITING_TIME);
}

core_statRetTypeDef rpi_pendingOpTimeoutHandler() {
	HAL_GPIO_WritePin(RPI_PIN_OUT_PORT, RPI_PIN_OUT_SCHEDULE_EXE, GPIO_PIN_SET);
	//HAL_GPIO_WritePin(RPI_PIN_OUT_PORT, RPI_PIN_OUT_SCHEDULE_END, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RPI_PIN_OUT_PORT, RPI_PIN_OUT_FIND_CAT_TIMEOUT, GPIO_PIN_SET);
	return OK;
}

static core_statRetTypeDef rpi_RxCpltCallbackHandler(UART_HandleTypeDef *huart) {
	//if (huart->Instance != pUartHandle->Instance) return ERR;
	//if (huart->Instance != USART2) return ERR;
	UARTdta.available = 1; // mark available
	UARTdta.type = rxBuf[0]; // copy data from buffer to internal var
	for (int i = 0; i < DTA_LEN - 1; i++)
		UARTdta.container[i] = rxBuf[i + 1];
	UARTdta.container[7] = 0;
	for (int i = 0; i < DTA_LEN + 1; i++) // clr buf
		rxBuf[i] = 0;
	HAL_UART_Receive_IT(pUartHandle, rxBuf, DTA_LEN); // restart rx
	return OK;
}

void rpi_init() {
	// register pending op handler
	core_statRetTypeDef retval = core_call_pendingOpRegister(&opcode, &rpi_pendingOpTimeoutHandler);
	if (retval != OK) {
#ifdef _TEST_MODE_ENABLED
		core_dbgTx("\r\n?FAILED TO REGISTER RPI PENDING OP HANDLER FUNCTION OF RPICOMM\r\n");
		while (1) {

		}
#endif
	}
	// register UART intr handler
	retval = core_call_uartHandlerRegister(&rpi_RxCpltCallbackHandler);
	if (retval != OK) {
#ifdef _TEST_MODE_ENABLED
		core_dbgTx("\r\n?FAILED TO REGISTER RPI UART INTR HANDLER FUNCTION OF RPICOMM\r\n");
		while (1) {

		}
#endif
	}

	// send pin data once, start and then timeout, set all the pins to HIGH(rpi conf: pull up to init
	HAL_GPIO_WritePin(RPI_PIN_OUT_PORT, RPI_PIN_OUT_FIND_CAT_TIMEOUT, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RPI_PIN_OUT_PORT, RPI_PIN_OUT_SCHEDULE_EXE, GPIO_PIN_RESET);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(RPI_PIN_OUT_PORT, RPI_PIN_OUT_SCHEDULE_EXE, GPIO_PIN_SET);
	HAL_Delay(500);
	HAL_GPIO_WritePin(RPI_PIN_OUT_PORT, RPI_PIN_OUT_FIND_CAT_TIMEOUT, GPIO_PIN_RESET);
	HAL_Delay(500);
	HAL_GPIO_WritePin(RPI_PIN_OUT_PORT, RPI_PIN_OUT_FIND_CAT_TIMEOUT, GPIO_PIN_SET);


	UARTdta.available = 0;
	for (int i = 0; i < 9; i++)
		rxBuf[i] = 0;
	//pinDta = 0;
	HAL_UART_Receive_IT(pUartHandle, rxBuf, DTA_LEN);
}
