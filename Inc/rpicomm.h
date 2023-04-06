/**
  *********************************************************************************************
  * NAME OF THE FILE : rpicomm.h
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

/* check source file for header and container info. */

#ifndef RPICOMM_H
#define RPICOMM_H

/* definitions */

// pinIO code and config
#define RPI_PIN_SEND_WAITING_TIME 1000
#define DTA_LEN 8

#define RPI_PINCODE_I_FOUNDCAT 0x01
#define RPI_PINCODE_O_SCHEDULE_EXE 0x01
#define RPI_PINCODE_O_SCHEDULE_END 0x02
#define RPI_PINCODE_O_FIND_CAT_TIMEOUT 0x04

// Data type header definitions
#define TYPE_SCHEDULE_TIME 'T'
#define TYPE_SCHEDULE_PATTERN 'P'
#define TYPE_SCHEDULE_SNACK_INTERVAL 'N'
#define TYPE_SCHEDULE_SPEED 'V'
#define TYPE_SYS '!'
#define TYPE_MANUAL_CTRL 'M'
#define TYPE_SCHEDULE_DURATION 'D'
#define TYPE_SCHEDULE_START '<'
#define TYPE_SCHEDULE_END '>'
//#define TYPE_RESP 0xFF

// pin code
#define RPI_PIN_IN_PORT GPIOA
#define RPI_PIN_IN_FOUNDCAT GPIO_PIN_0
#define RPI_PIN_OUT_PORT GPIOC // Reserved: C6 ~ C9
#define RPI_PIN_OUT_SCHEDULE_EXE GPIO_PIN_6
#define RPI_PIN_OUT_SCHEDULE_END GPIO_PIN_7
#define RPI_PIN_OUT_FIND_CAT_TIMEOUT GPIO_PIN_8

/* exported struct */
struct SerialDta {
	uint8_t available;
	uint8_t type;
	uint8_t container[8];
};

/* exported vars */


/* exported func prototypes */
void rpi_setHandle(UART_HandleTypeDef* ph);
void rpi_init();
int rpi_getSerialDta(struct SerialDta* pDest); // returns zero if no data is available, even though flag will be set by callback handler...
uint8_t rpi_getPinDta();
void rpi_sendPin(int code);
int rpi_serialDtaAvailable(); // returns zero if not available
//int rpi_tcpipRespond(uint8_t isErr); // send RESP pkt to client app. returns 0 on success

void rpi_RxCpltCallbackHandler();
void rpi_msTimeoutHandler();

#endif
