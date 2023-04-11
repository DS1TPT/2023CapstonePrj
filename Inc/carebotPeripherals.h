/**
  *********************************************************************************************
  * NAME OF THE FILE : carebotPeripherals.h
  * BRIEF INFORMATION: peripheral device driver
  *
  * Copyright (c) 2023 Lee Geon-goo.
  * All rights reserved.
  *
  * This file is part of catCareBot.
  *
  *********************************************************************************************
  */

#ifndef CAREBOTPERIPHERALS_H
#define CAREBOTPERIPHERALS_H

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

/* definitions */
// FIXED VAL
#define IR_SNSR_FAR 0
#define IR_SNSR_NEAR 1
#define IR_SNSR_ERR -1
#define IR_SNSR_MODE_OP 1
#define IR_SNSR_MODE_FIND 2
#define IR_SNSR_MODE_SNACK 3
// config
#define LASER_PORT GPIOA
#define LASER_PIN GPIO_PIN_5
//#define LED_PORT GPIO
//#define LED_PIN GPIO_PIN_
#define IR_SNSR_POLL_TIMEOUT 10
#define IR_SNSR_TRIG_DIST_OP 20.0
#define IR_SNSR_TRIG_DIST_FIND 40.0
#define IR_SNSR_TRIG_DIST_SNACK 15.0

/* exported struct */

/* exported vars */

/* exported func prototypes */
void periph_setHandle(ADC_HandleTypeDef* ph);
void periph_init();
void periph_laser_on();
void periph_laser_off();
int periph_irSnsrChk(int mode);

#endif
