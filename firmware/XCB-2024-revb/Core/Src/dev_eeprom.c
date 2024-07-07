/*
 * dev_eeprom.c
 *
 *  Created on: Jun 2, 2024
 *      Author: kiu
 */

#include "dev_eeprom.h"
#include "main.h"

static uint32_t eeMemory[EE_LEN];

// ----------------------------------------------------------------------------

uint8_t eeByteGet(uint8_t mem, uint8_t b8) {
	uint32_t mask = 0x000000FF << (b8 * 8);
	return (eeMemory[mem] & mask) >> (b8 * 8);
}

void eeByteSet(uint8_t mem, uint8_t b8, uint8_t insert) {
	uint32_t tmp = eeMemory[mem] & (~(0x000000FF << (b8 * 8)));
	eeMemory[mem] = tmp | ((uint32_t) insert) << (b8 * 8);
}

// ----------------------------------------------------------------------------

void eePageLoad(uint8_t page) {
#if defined(DEBUG)
	printf("E: Load page: 0x%02X ", page);
#endif

// https://controllerstech.com/flash-programming-in-stm32/
	for (uint8_t i = 0; i < EE_LEN; i++) {
		eeMemory[i] = *(__IO uint32_t*) ((page * 1024) + i * 4);
	}

#if defined(DEBUG)
	for (uint8_t i = 0; i < EE_LEN; i++) {
		printf("%08X ", (unsigned int) eeMemory[i]);
	}
	printf("\n");
#endif
}

void eePageStore(uint8_t page) {
#if defined(DEBUG)
	printf("E: Store page: 0x%02X ", page);
	for (uint8_t i = 0; i < EE_LEN; i++) {
		printf("%08X ", (unsigned int) eeMemory[i]);
	}
	printf("\n");
#endif

// https://controllerstech.com/flash-programming-in-stm32/
	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PAGEError;

	HAL_FLASH_Unlock();

	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = page * 1024;
	EraseInitStruct.NbPages = 1;
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
#if defined(DEBUG)
		printf("E: Store: Error while erasing page 0x%02X: 0x%08X\n", page, (unsigned int) HAL_FLASH_GetError());
#endif
		return;
	}

	for (uint8_t i = 0; i < EE_LEN; i++) {
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (page * 1024) + i * 4, eeMemory[i]) != HAL_OK) {
#if defined(DEBUG)
			printf("E: Store: Error while writing page 0x%02X word 0x%02X: 0x%08X\n", page, i,
					(unsigned int) HAL_FLASH_GetError());
#endif
			return;
		}
	}
	HAL_FLASH_Lock();
}

void eeStore() {
	eePageStore(EE_PAGE_B);
	eePageStore(EE_PAGE_A);
}

void eeWipe(bool keep_id) {
	uint8_t id = eeByteGet(0, EE_LOC_0_ID);

	for (uint8_t i = 0; i < EE_LEN; i++) {
		eeMemory[i] = 0;
	}
	eeByteSet(0, EE_LOC_0_VALID, EE_VALID);

	if (keep_id) {
		eeByteSet(0, EE_LOC_0_ID, id);
	}

	eeStore();
}

// ----------------------------------------------------------------------------

uint8_t eeIDGet() {
	return eeByteGet(0, EE_LOC_0_ID);
}

bool eeHighscoreSet(uint8_t score) {
#if defined(DEBUG)
	printf("E: Updating highscore: %d\n", score);
#endif

	if (eeByteGet(0, EE_LOC_0_HIGHSCORE) == score) {
#if defined(DEBUG)
		printf("E: Updating highscore ignored\n");
#endif
		return false;
	}

	eeByteSet(0, EE_LOC_0_HIGHSCORE, score);
	eeStore();
	return true;
}

uint8_t eeHighscoreGet() {
	return eeByteGet(0, EE_LOC_0_HIGHSCORE);
}

bool eeIDUnlock(uint8_t id) {
#if defined(DEBUG)
	printf("E: Updating unlock: 0x%02X\n", id);
#endif

	bool valid = false;
	valid |= (id >= ID_UNLOCK_START && id <= ID_UNLOCK_END);
	valid |= (id >= ID_TREASURE_START && id <= ID_TREASURE_END);
	if (!valid) {
#if defined(DEBUG)
		printf("E: Updating unlock ignored, out of range\n");
#endif
		return false;
	}

	uint8_t offset = id / 32;
	uint32_t mask = (0x00000001 << (id - (offset * 32)));
	bool current = eeMemory[offset + 1] & mask;
	if (current) {
#if defined(DEBUG)
		printf("E: Updating unlock ignored, already set\n");
#endif
		return false;
	}

	eeMemory[offset + 1] |= mask;

	eeStore();
	return true;
}

uint16_t eeIDTreasureGet() {
	return eeMemory[8] & 0x00FFFF;
}

uint8_t eeIDUniqueCount() {
	uint8_t count = 0;

	for (uint8_t m = 1; m < 8; m++) {
		for (uint8_t i = 0; i < 32; i++) {
			if (eeMemory[m] & (1 << i)) {
				count++;
			}
		}
	}

	return count;
}

uint8_t eeSensorGet() {
	return eeByteGet(0, EE_LOC_0_SENSOR);
}

bool eeSensorSet(uint8_t cfg) {
#if defined(DEBUG)
	printf("E: Updating sensor: %d\n", cfg);
#endif

	if (eeByteGet(0, EE_LOC_0_SENSOR) == cfg) {
#if defined(DEBUG)
		printf("E: Updating sensor ignored\n");
#endif
		return false;
	}

	eeByteSet(0, EE_LOC_0_SENSOR, cfg);
	eeStore();
	return true;
}

// ----------------------------------------------------------------------------

uint32_t eeSeed() {
	uint32_t seed = 0x42220623;
	for (uint8_t i = 0; i < EE_LEN; i++) {
		seed ^= eeMemory[i];
	}
	return seed;
}

void eeInit() {
	eePageLoad(EE_PAGE_A);
	bool ee_a = eeByteGet(0, EE_LOC_0_VALID) == EE_VALID;

	eePageLoad(EE_PAGE_B);
	bool ee_b = eeByteGet(0, EE_LOC_0_VALID) == EE_VALID;

	if (ee_a && ee_b) {
		//eePageLoad(EE_PAGE_B);
	} else if (ee_a && !ee_b) {
		eePageLoad(EE_PAGE_A);
	} else if (!ee_a && ee_b) {
		//eePageLoad(EE_PAGE_B);
	} else {
		eeWipe(false);
	}

#if defined(DEBUG)
	printf("E: ID: 0x%02X\n", eeIDGet());
	printf("E: Unique (count): %d\n", eeIDUniqueCount());
	printf("E: Treasure (map): 0x%02X\n", eeIDTreasureGet());
	printf("E: Highscore: %d\n", eeHighscoreGet());
	printf("E: Sensor (cfg): %d\n", eeSensorGet());
#endif
}
