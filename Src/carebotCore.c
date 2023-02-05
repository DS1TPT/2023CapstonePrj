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
#include "opman.h"
#include "rpicomm.h"
#include "scheduler.h"
#include "l298n.h"
#include "sg90.h"

struct SerialDta rpidta;

// opcode definitions
#define OP_SNACK_RET_MOTOR 0x01

// system properties
const uint8_t MAN_ROT_SPD = 25;
const uint8_t MAN_DRV_SPD = 50;
const uint8_t DEF_ANG_A = 45; // default angle of toy motor, WRITE RETRACTED ANGLE
const uint8_t DEF_ANG_B = 30; // default angle of snack motor
const uint16_t OP_SNACK_RET_MOTOR_WAITING_TIME = 500;

// derived properties
const uint8_t TOY_ANG_DRAW = DEF_ANG_A + 90; // max angle(draw) of toy motor
const uint8_t TOY_ANG_RETRACT = DEF_ANG_A;
const uint8_t SNACK_ANG_RDY = DEF_ANG_B;
const uint8_t SNACK_ANG_GIVE = DEF_ANG_B + 90;

// system variables
static uint8_t flagTimeElapsed = FALSE;
static uint8_t flagHibernate = FALSE;
static uint8_t recvScheduleMode = FALSE;
static uint8_t initState = FALSE;
static uint8_t secTimEna = FALSE;

static void manualDrive() {
	// enable motor first
	l298n_enable();
	sg90_enable(SG90_MOTOR_A, DEF_ANGLE_A);
	sg90_enable(SG90_MOTOR_B, DEF_ANGLE_B);
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
						l298n_setSpeed(L298N_MOTOR_A, MAN_ROT_SPD);
						l298n_setSpeed(L298N_MOTOR_B, MAN_ROT_SPD);
						break;
					case 0x02: // right
						l298n_setRotation(L298N_MOTOR_A, L298N_CCW);
						l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
						l298n_setSpeed(L298N_MOTOR_A, MAN_ROT_SPD);
						l298n_setSpeed(L298N_MOTOR_B, MAN_ROT_SPD);
						break;
					case 0x04: // forward
						l298n_setRotation(L298N_MOTOR_A, L298N_CCW);
						l298n_setRotation(L298N_MOTOR_B, L298N_CW);
						l298n_setSpeed(L298N_MOTOR_A, MAN_DRV_SPD);
						l298n_setSpeed(L298N_MOTOR_B, MAN_DRV_SPD);
						break;
					case 0x08: // reverse
						l298n_setRotation(L298N_MOTOR_A, L298N_CW);
						l298n_setRotation(L298N_MOTOR_B, L298N_CCW);
						l298n_setSpeed(L298N_MOTOR_A, MAN_DRV_SPD);
						l298n_setSpeed(L298N_MOTOR_B, MAN_DRV_SPD);
						break;
					case 0x10: // snack
						sg90_setAngle(SG90_MOTOR_B, SNACK_ANG_GIVE);
						opman_addPendingOp(OP_SNACK_RET_MOTOR, OP_SNACK_RET_MOTOR_WAITING_TIME);
						break;
					case 0x20: // hide toy
						sg90_setAngle(SG90_MOTOR_A, TOY_ANG_RETRACT);
						break;
					case 0x40: // draw toy
						sg90_setAngle(SG90_MOTOR_A, TOY_ANG_DRAW);
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

static void autoDrive() {
	// enable motor
	l298n_enable();
	sg90_enable(SG90_MOTOR_A, DEF_ANGLE_A);
	sg90_enable(SG90_MOTOR_B, DEF_ANGLE_B);

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

static void coreMain() {
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
	opman_init();
	rpicomm_init();
	scheduler_init();
	l298n_init();
	sg90_init();
	initState = TRUE;

	// start core
	if (secTimEna == FALSE) { // enable timer if timer is off
		HAL_TIM_Base_Start_IT(CORE_SEC_TIM_HANDLE);
		secTimEna = TRUE;
	}
	coreMain();
}

void core_callOp(uint8_t opcode) {
	switch (opcode) {
	case OP_SNACK_RET_MOTOR:
		sg90_setAngle(SG90_MOTOR_B, SNACK_ANG_RDY);
		break;
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		rpi_RxCpltCallbackHandler();
		//flagRxCplt = TRUE;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance == TIM10) { // 1s sys tim
		if(scheduler_TimCallbackHandler()) flagTimeElapsed = TRUE;
		else flagTimeElapsed = FALSE;
	}
	else if (htim->Instance == TIM11) { // 1ms sys tim(for postponed ops)
		opman_callbackHandler();
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

}
