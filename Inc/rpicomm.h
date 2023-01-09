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

/* definitions */
// header
#define TYPE_NO 0
#define TYPE_CONN 1
#define TYPE_SCHEDULE_TIME 2
#define TYPE_SCHEDULE_PATTERN 4
#define TYPE_SCHEDULE_SNACK 8
#define TYPE_SCHEDULE_MODE 16
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
int rpi_getSerialDta(struct SerialDta dest); // return zero if data is unavailable
uint8_t rpi_getPinDta();
