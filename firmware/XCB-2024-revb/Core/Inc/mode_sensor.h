/*
 * mode_sensor.h
 *
 *  Created on: Jun 2, 2024
 *      Author: kiu
 */

#ifndef INC_MODE_SENSOR_H_
#define INC_MODE_SENSOR_H_

#include "main.h"
#include <stdbool.h>
#include "stm32f0xx_hal.h"

#if defined(DEBUG)
#include "stdio.h"
#endif

#define SENSOR_CYCLE_S 3

void sensorButton(button_t btn);
void sensorTick();
void sensorStart();
void sensorStop();
void sensorInit(I2C_HandleTypeDef hi2c1);

#endif /* INC_MODE_SENSOR_H_ */
