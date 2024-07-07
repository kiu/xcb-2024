/*
 * dev_buzz.h
 *
 *  Created on: Jun 2, 2024
 *      Author: kiu
 */

#ifndef INC_DEV_BUZZ_H_
#define INC_DEV_BUZZ_H_

#include <stdbool.h>

#include "stm32f0xx_hal.h"

#if defined(DEBUG)
#include "stdio.h"
#endif

void buzzNote(uint8_t note, uint8_t octave, uint16_t duration, uint16_t delay);

void buzzTouch();

void buzzInit(TIM_HandleTypeDef htim);

#endif /* INC_DEV_BUZZ_H_ */
