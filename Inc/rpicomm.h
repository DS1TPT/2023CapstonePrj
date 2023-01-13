/**
  *********************************************************************************************
  * NAME OF THE FILE : rpicomm.h
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

/* check source file for header and container info. */

#ifndef RPICOMM_H
#define RPICOMM_H

/* definitions */
#define TYPE_NO 0
#define TYPE_CONN 1
#define TYPE_SCHEDULE_TIME 2
#define TYPE_SCHEDULE_PATTERN 4
#define TYPE_SCHEDULE_SNACK 8
#define TYPE_SCHEDULE_SPEED 16
#define TYPE_SYS 32
#define TYPE_MANUAL_CTRL 64

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
int rpi_getSerialDta(struct SerialDta dest); // returns zero if no data is available
uint8_t rpi_getPinDta();
int rpi_tcpipAvailable(); // returns zero if not available
int rpi_tcpipRespond(); // send RESP pkt to client app. returns 0 on success

#endif
