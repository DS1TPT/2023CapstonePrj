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
#define OP_RPI_PIN_IO_SEND_WAITING_TIME 1000
#define RPI_PIN_I_FOUNDCAT 0x01

#define RPI_PIN_O_SOUND_SEEK 0x01

// pkt header
#define TYPE_NO 0
#define TYPE_CONN 1
#define TYPE_SCHEDULE_TIME 2
#define TYPE_SCHEDULE_PATTERN 4
#define TYPE_SCHEDULE_SNACK 8
#define TYPE_SCHEDULE_SPEED 0x10
#define TYPE_SYS 0x20
#define TYPE_MANUAL_CTRL 0x40
#define TYPE_SCHEDULE_DURATION 0x80
#define TYPE_SCHEDULE_START 0xAA
#define TYPE_SCHEDULE_END 0x55
#define TYPE_RESP 0xFF

// pin code
#define RPI_PIN_IN_PORT GPIOA
#define RPI_PIN_IN_PIN_FOUNDCAT GPIO_PIN_0

/* exported struct */
struct SerialDta {
	uint8_t available;
	uint8_t type;
	uint8_t container[8];
};

/* exported vars */


/* exported func prototypes */
void rpi_init();
int rpi_getSerialDta(struct SerialDta dest); // returns zero if no data is available, even though flag will be set by callback handler...
uint8_t rpi_getPinDta();
void rpi_sendPin(int code);
int rpi_serialDtaAvailable(); // returns zero if not available
int rpi_tcpipRespond(uint8_t isErr); // send RESP pkt to client app. returns 0 on success

void rpi_RxCpltCallbackHandler();
void rpi_opmanTimeoutHandler();

#endif
