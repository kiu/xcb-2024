/*
 * dev_lcd.h
 *
 *  Created on: Jun 2, 2024
 *      Author: kiu
 */

#ifndef INC_DEV_LCD_H_
#define INC_DEV_LCD_H_

#include <stdbool.h>
#include "stm32f0xx_hal.h"

#if defined(DEBUG)
#include "stdio.h"
#endif

uint32_t lcdDigits_1(uint8_t base, int16_t value, uint8_t dot1, uint8_t dot2, uint8_t minus);
uint32_t lcdDigits_2(uint8_t base, int16_t value, uint8_t dot1, uint8_t dot2, uint8_t minus);
uint32_t lcdDigits_3(uint8_t base, int16_t value, uint8_t dot1, uint8_t dot2, uint8_t minus);
uint32_t lcdChar(uint8_t loc);
uint32_t lcdMap(uint16_t unlocks);

void lcdShow(uint32_t data);
void lcdAdd(uint32_t data);
void lcdRemove(uint32_t data);

void lcdInit();

#endif /* INC_DEV_LCD_H_ */
