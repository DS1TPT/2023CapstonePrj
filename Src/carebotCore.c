/**
  *********************************************************************************************
  * NAME OF THE FILE : carebotCore.c
  * BRIEF INFORMATION: robot program core
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

#include "carebotCore.h"
#include "rpicomm.h"
#include "scheduler.h"
#include "l298n.h"
#include "sg90.h"

struct SerialDta rpidta;

uint8_t flagTimeElapsed = FALSE;
uint8_t flagHibernate = FALSE;

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
					break;
				case TYPE_SCHEDULE_PATTERN:
					break;
				case TYPE_SCHEDULE_SNACK:
					break;
				case TYPE_SCHEDULE_SPEED:
					break;
				case TYPE_SYS:
					break;
				case TYPE_MANUAL_CTRL:
					break;
				}

			}
		}
		else { // do something else

		}
		// check if schedule is set
		// check for schedule. process schedule if time has been elapsed
		if (flagTimeElapsed) {
			flagTimeElapsed = FALSE; // reset flag first

		}
	}
}

void core_start() {
	// initialization
	rpicomm_init();
	scheduler_init();
	l298n_init();
	sg90_init();

	// start core exe
	coreMain();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		rpi_RxCpltCallbackHandler();
		flagRxCplt = TRUE;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance == TIM10)
		if(scheduler_TimCallbackHandler()) flagTimeElapsed = TRUE;
		else flagTimeElapsed = FALSE;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

}
