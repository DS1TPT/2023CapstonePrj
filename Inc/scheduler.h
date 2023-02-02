/**
  *********************************************************************************************
  * NAME OF THE FILE : scheduler.c
  * BRIEF INFORMATION: auto-play schedule manager
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

#ifndef SCHEDULER_H
#define SCHEDULER_H

/* includes */
#include "main.h"
//#include "rpicomm.h"

/* exported struct */

/* exported vars */

/* exported func prototypes */
void scheduler_init();
void scheduler_setSpd(uint8_t spd); // set speed of robot
int scheduler_setTime(int32_t sec); // set time and start timer. returns OK on success, ERR on failure
int scheduler_enqueuePattern(uint8_t code); // returns ERR if queue is full, returns OK otherwise
int scheduler_dequeuePattern(); // returns ERR if queue is empty, returns OK otherwise
int scheduler_TimCallbackHandler(); // returns 0 if time is not elapsed or time is not set. returns 1 if elapsed
int scheduler_isSet(); // check if schedule is set and complete, returns 0 if schedule is not set

#endif
