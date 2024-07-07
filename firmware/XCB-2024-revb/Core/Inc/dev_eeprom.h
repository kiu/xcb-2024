/*
 * dev_eeprom.h
 *
 *  Created on: Jun 2, 2024
 *      Author: kiu
 */

#ifndef INC_DEV_EEPROM_H_
#define INC_DEV_EEPROM_H_

#include <stdbool.h>
#include "stm32f0xx_hal.h"

#if defined(DEBUG)
#include "stdio.h"
#endif

#define EE_PAGE_A 30
#define EE_PAGE_B 31

#define EE_VALID 0x42
#define EE_LEN 9 // Words: 1 Config + 7 Unique + 1 Treasure

#define EE_LOC_0_VALID 0x00
#define EE_LOC_0_ID 0x01
#define EE_LOC_0_HIGHSCORE 0x02
#define EE_LOC_0_SENSOR 0x03

uint8_t eeIDGet();

bool eeIDUnlock(uint8_t id);
uint8_t eeIDUniqueCount();
uint16_t eeIDTreasureGet();

bool eeHighscoreSet(uint8_t score);
uint8_t eeHighscoreGet();

uint8_t eeSensorGet();
bool eeSensorSet(uint8_t cfg);

void eeWipe(bool keep_id);

uint32_t eeSeed();

void eeInit();

#endif /* INC_DEV_EEPROM_H_ */
