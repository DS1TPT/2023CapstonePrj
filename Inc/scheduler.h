/**
  *********************************************************************************************
  * NAME OF THE FILE : scheduler.c
  * BRIEF INFORMATION: manage auto-play schedule
  *
  * Copyright (c) 2023 Lee Geon-goo.
  * All rights reserved.
  *
  * This file is part of catCareBot.
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
//void scheduler_setHandle(TIM_HandleTypeDef* ph);
void scheduler_init();
void scheduler_setSpd(uint8_t spd); // set speed of robot
int scheduler_setTime(int32_t sec); // set time and start timer. returns OK on success, ERR on failure
int scheduler_setDuration(int32_t sec); // set play time
void scheduler_setSnack(uint8_t num); // set how many times the user want to give treat
int scheduler_enqueuePattern(uint8_t code); // returns ERR if queue is full, returns OK otherwise
int scheduler_dequeuePattern(); // returns ERR if queue is empty, returns OK otherwise
int scheduler_TimCallbackHandler(); // returns 0 if time is not elapsed or time is not set. returns 1 if elapsed
int scheduler_isSet(); // check if schedule is set and complete, returns 0 if schedule is not set
uint8_t scheduler_getSpd(); // return speed to core
uint8_t scheduler_getSnack(); // return how many times the user want to give treat to core
int32_t scheduler_getInterval(); // return interval time between patterns

#endif
