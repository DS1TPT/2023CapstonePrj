/**
  *********************************************************************************************
  * NAME OF THE FILE : carebotCore.h
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

#ifndef CAREBOTCORE_H_
#define CAREBOTCORE_H_

/*
 * core will control every hardware after the main calls start function.
 */

#include "main.h"

void core_start(); // this should be called only once by main.c

#endif
