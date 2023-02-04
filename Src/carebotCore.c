/**
  *********************************************************************************************
  * NAME OF THE FILE : carebotCore.c
  * BRIEF INFORMATION: robot program core
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
#include "scheduler.h"
#include "l298n.h"
#include "sg90.h"

struct SerialDta rpidta;

uint8_t flagTimeElapsed = FALSE;
uint8_t flagHibernate = FALSE;
uint8_t recvScheduleMode = FALSE;
uint8_t initState = FALSE;

const uint8_t manRotSpd = 25;
const uint8_t manDrvSpd = 50;
const uint8_t defAngleA = 90; // default angle of toy motor
const uint8_t defAngleB = 90; // default angle of snack motor

void manualDrive() {
	// enable motor first
	l298n_enable();
	sg90_enable(SG90_MOTOR_A, defAngleA);
	sg90_enable(SG90_MOTOR_B, defAngleB);
	while (1) {
		if (rpi_serialDtaAvailable()) {
			if (rpi_getSerialDta(rpidta)) {
				if (rpidta.type == TYPE_MANUAL_CTRL) {
					switch (rpidta.container[0]) {
					case 0x00: // stop
						l298n_setRotation(L298N_MOTOR_A, L298N_STOP);
						l298n_setRotation(L298N_MOTOR_B, L298N_STOP);
						break;
					case 0x01: // left
						l298n_setRotation(L298N_MOTOR_A, L298N_CW);
						l298n_setRotation(L298N_MOTOR_B, L298N_CW);
						l298n_setSpeed(L298N_MOTOR_A, manRotSpd);
						l298n_setSpeed(L298N_MOTOR_B, manRotSpd);
						break;
					case 0x02: // right
						l298n_setRotation(L298N_MOTOR_A, L298N_CCW);
						l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
						l298n_setSpeed(L298N_MOTOR_A, manRotSpd);
						l298n_setSpeed(L298N_MOTOR_B, manRotSpd);
						break;
					case 0x04: // forward
						l298n_setRotation(L298N_MOTOR_A, L298N_CCW);
						l298n_setRotation(L298N_MOTOR_B, L298N_CW);
						l298n_setSpeed(L298N_MOTOR_A, manDrvSpd);
						l298n_setSpeed(L298N_MOTOR_B, manDrvSpd);
						break;
					case 0x08: // reverse
						l298n_setRotation(L298N_MOTOR_A, L298N_CW);
						l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
						l298n_setSpeed(L298N_MOTOR_A, manDrvSpd);
						l298n_setSpeed(L298N_MOTOR_B, manDrvSpd);
						break;
					case 0x10: // snack
						break;
					case 0x20: // hide toy
						break;
					case 0x40: // draw toy
						break;
					case 0x80: // undefined
						break;
					}
				}
				else if (rpidta.type == TYPE_SYS && rpidta.container[0] == 2) {
					// stop manual drive
					l298n_disable();
					sg90_disable(SG90_MOTOR_A);
					sg90_disable(SG90_MOTOR_B);
				}
			}
		}
	}
}

void autoDrive() {
	// enable motor
	l298n_enable();
	sg90_enable(SG90_MOTOR_A, defAngleA);
	sg90_enable(SG90_MOTOR_B, defAngleB);

	// find & call cat

	// draw toy

	// play

	// retract toy and disable servo

	sg90_disable(SG90_MOTOR_A);
	sg90_disable(SG90_MOTOR_B);

	// move away from cat(park near a wall)

	// after parking, turn off motor
	l298n_disable();
}

void coreMain() {
	// check for rpi data
	while (1) {
		if (rpi_serialDtaAvailable()) { // process data if available
			if (rpi_getSerialDta(rpidta)) { // get data
				switch (rpidta.type) {
				case TYPE_NO:
					break;
				case TYPE_CONN:
					break;
				case TYPE_SCHEDULE_TIME:
					if (!recvScheduleMode) break;
					int32_t i32 = 0;
					uint8_t pu8 = &i32;
					for (int i = 0; i < 4; i++) {
						*(pu8++) = rpidta.container[i];
					}
					if (scheduler_setTime(i32) == ERR) {
						// error
					}
					break;
				case TYPE_SCHEDULE_PATTERN:
					if (!recvScheduleMode) break;
					for (int i = 0; i < 7; i++) {
						if (rpidta.container[i] & 0x0F) scheduler_enqueuePattern(rpidta.container[i] & 0x0F);
						else break;
						if (rpidta.container[i] & 0xF0) scheduler_enqueuePattern(rpidta.container[i] & 0xF0);
						else break;
					}
					break;
				case TYPE_SCHEDULE_SNACK:
					if (!recvScheduleMode) break;
					scheduler_setSnack(rpidta.container[0]);
					break;
				case TYPE_SCHEDULE_SPEED:
					if (!recvScheduleMode) break;
					scheduler_setSpd(rpidta.container[0]);
					break;
				case TYPE_SYS:
					switch (rpidta.container[0]) {
					case 0x01: // start manual drive
						manualDrive();
						break;
					case 0xFF: // initialize whole system
						coreRestart();
						break;
					}
					break;
				case TYPE_SCHEDULE_START:
					recvScheduleMode = TRUE;
					break;
				case TYPE_SCHEDULE_STOP:
					recvScheduleMode = FALSE;
				}

			}
		}
		else { // do something else

		}
		// check if schedule is set
		// check for schedule. process schedule if time has been elapsed
		if (flagTimeElapsed) {
			flagTimeElapsed = FALSE; // reset flag first
			autoDrive();
		}
	}
}

void core_start() {
	if (initState) coreMain(); // skip initialization

	// initialization
	rpicomm_init();
	scheduler_init();
	l298n_init();
	sg90_init();
	initState = TRUE;

	// start core
	coreMain();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		rpi_RxCpltCallbackHandler();
		//flagRxCplt = TRUE;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance == TIM10)
		if(scheduler_TimCallbackHandler()) flagTimeElapsed = TRUE;
		else flagTimeElapsed = FALSE;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

}
