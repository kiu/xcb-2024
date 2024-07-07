/*
 * mode_conf.h
 *
 *  Created on: Jun 2, 2024
 *      Author: kiu
 */

#ifndef INC_MODE_CONF_H_
#define INC_MODE_CONF_H_

#include "main.h"
#include <stdbool.h>
#include "stm32f0xx_hal.h"

#if defined(DEBUG)
#include "stdio.h"
#endif

#define CONF_CYCLE_S 3
#define CONF_CYCLE_TX_S 30
#define CONF_CYCLE_TX_OTHER_S 15
#define CONF_CYCLE_UNLOCK_S 50
#define CONF_RX_BUF 10

bool confAllowSleep();

void confIRTX_htim_enc();
void confIRRX_htim_dec();
void confIRRX_gpio_dec();

void confButton(button_t btn);
void confTick();
void confStart();
void confStop();
void confInit(TIM_HandleTypeDef htim_pwm, TIM_HandleTypeDef htim_enc, TIM_HandleTypeDef htim_dec);

#endif /* INC_MODE_CONF_H_ */
