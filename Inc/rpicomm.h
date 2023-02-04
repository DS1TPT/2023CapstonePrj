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
#define TYPE_NO 0
#define TYPE_CONN 1
#define TYPE_SCHEDULE_TIME 2
#define TYPE_SCHEDULE_PATTERN 4
#define TYPE_SCHEDULE_SNACK 8
#define TYPE_SCHEDULE_SPEED 0x10
#define TYPE_SYS 0x20
#define TYPE_MANUAL_CTRL 0x40
#define TYPE_SCHEDULE_START 0xAA
#define TYPE_SCHEDULE_END 0x55
#define TYPE_RESP 0xFF

/* exported struct */
struct SerialDta {
	uint8_t available;
	uint8_t type;
	uint8_t container[8];
};

/* exported vars */


/* exported func prototypes */
void rpi_init();
void rpi_RxCpltCallbackHandler();
int rpi_getSerialDta(struct SerialDta dest); // returns zero if no data is available, even though flag will be set by callback handler...
uint8_t rpi_getPinDta();
int rpi_serialDtaAvailable(); // returns zero if not available
int rpi_tcpipRespond(uint8_t isErr); // send RESP pkt to client app. returns 0 on success

#endif
