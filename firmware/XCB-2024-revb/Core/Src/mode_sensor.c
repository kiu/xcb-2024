/*
 * mode_sensor.c
 *
 *  Created on: Jun 2, 2024
 *      Author: kiu
 */

#include "mode_sensor.h"

#include "dev_buzz.h"
#include "dev_eeprom.h"
#include "dev_lcd.h"

static I2C_HandleTypeDef sensor_hi2c1;

static int16_t sensor_temp_cur;
static int16_t sensor_temp_max;
static int16_t sensor_temp_min;

static int16_t sensor_humid_cur;
static int16_t sensor_humid_max;
static int16_t sensor_humid_min;

static bool sensor_i2c_reset = true;
static bool sensor_show_flip = false;
static uint8_t sensor_tick_count = 0;

typedef enum {
	SENSOR_SHOW_CYCLE, SENSOR_SHOW_TEMPERATURE, SENSOR_SHOW_HUMIDTY
} sensor_show_type_t;

static sensor_show_type_t sensor_show_type = SENSOR_SHOW_CYCLE;

typedef enum {
	SENSOR_SHOW_CURRENT, SENSOR_SHOW_MIN, SENSOR_SHOW_MAX,
} sensor_show_value_t;

static sensor_show_value_t sensor_show_value = SENSOR_SHOW_CURRENT;

// ----------------------------------------------------------------------------

void sensorClear() {
	sensor_temp_cur = 0x7FFF;
	sensor_temp_max = 0x7FFF;
	sensor_temp_min = 0x7FFF;
	sensor_humid_cur = 0x7FFF;
	sensor_humid_max = 0x7FFF;
	sensor_humid_min = 0x7FFF;
}

void sensorUpdateLCD() {
	if (sensor_show_type == SENSOR_SHOW_TEMPERATURE) {
		int16_t value = 0;
		if (sensor_show_value == SENSOR_SHOW_CURRENT) {
			value = sensor_temp_cur;
		}
		if (sensor_show_value == SENSOR_SHOW_MIN) {
			value = sensor_temp_min;
		}
		if (sensor_show_value == SENSOR_SHOW_MAX) {
			value = sensor_temp_max;
		}

		// 23.5 | -23
		if (value >= 0) {
			lcdShow(lcdDigits_3(10, value, 0, 1, 0));
		} else {
			lcdShow(lcdDigits_3(10, value / 10, 0, 0, 1));
		}
	}

	if (sensor_show_type == SENSOR_SHOW_HUMIDTY) {
		int16_t value = 0;
		if (sensor_show_value == SENSOR_SHOW_CURRENT) {
			value = sensor_humid_cur;
		}
		if (sensor_show_value == SENSOR_SHOW_MIN) {
			value = sensor_humid_min;
		}
		if (sensor_show_value == SENSOR_SHOW_MAX) {
			value = sensor_humid_max;
		}

		// 45.3
		lcdShow(lcdDigits_3(10, value, 0, 1, 0));
	}

	if (sensor_show_type == SENSOR_SHOW_CYCLE) {
		sensor_show_flip = !sensor_show_flip;

		// 23.5 | -23 | 45h
		if (sensor_show_flip) {
			int16_t value = 0;
			if (sensor_show_value == SENSOR_SHOW_CURRENT) {
				value = sensor_temp_cur;
			}
			if (sensor_show_value == SENSOR_SHOW_MIN) {
				value = sensor_temp_min;
			}
			if (sensor_show_value == SENSOR_SHOW_MAX) {
				value = sensor_temp_max;
			}

			if (value >= 0) {
				lcdShow(lcdDigits_3(10, value, 0, 1, 0));
			} else {
				lcdShow(lcdDigits_3(10, value / 10, 0, 0, 1));
			}
		} else {
			int16_t value = 0;
			if (sensor_show_value == SENSOR_SHOW_CURRENT) {
				value = sensor_humid_cur;
			}
			if (sensor_show_value == SENSOR_SHOW_MIN) {
				value = sensor_humid_min;
			}
			if (sensor_show_value == SENSOR_SHOW_MAX) {
				value = sensor_humid_max;
			}
			uint32_t lcd = 0;
			lcd |= lcdDigits_3(10, value / 10 * 10, 0, 0, 0);
			lcd &= ~lcdChar(0x00);
			lcd |= lcdChar(0x42);
			lcdShow(lcd);
		}
	}

}

void sensorButtonPressed(uint16_t lcd_char) {
	lcdShow(lcdChar(lcd_char));
	HAL_Delay(BUTTON_PRESS_DELAY);
	sensor_tick_count = SENSOR_CYCLE_S;
}

void sensorButton(button_t btn) {
	buzzTouch();

	if (btn == BUTTON_TL) {
		if (sensor_show_type == SENSOR_SHOW_TEMPERATURE) {
			sensor_show_type = SENSOR_SHOW_HUMIDTY;
			sensorButtonPressed(0x42); // xxh
		} else if (sensor_show_type == SENSOR_SHOW_HUMIDTY) {
			sensor_show_type = SENSOR_SHOW_CYCLE;
			sensorButtonPressed(0x43); // °Ch
		} else if (sensor_show_type == SENSOR_SHOW_CYCLE) {
			sensor_show_type = SENSOR_SHOW_TEMPERATURE;
			sensorButtonPressed(0x41); // xxh
		}

#if defined(DEBUG)
		printf("S: Changed type display: %d\n", sensor_show_type);
#endif
	}

	if (btn == BUTTON_TR) {
		if (sensor_show_value == SENSOR_SHOW_CURRENT) {
			sensor_show_value = SENSOR_SHOW_MIN;
			sensorButtonPressed(0x45); // Lo
		} else if (sensor_show_value == SENSOR_SHOW_MIN) {
			sensor_show_value = SENSOR_SHOW_MAX;
			sensorButtonPressed(0x46); // Hi
		} else if (sensor_show_value == SENSOR_SHOW_MAX) {
			sensor_show_value = SENSOR_SHOW_CURRENT;
			sensorButtonPressed(0x44); // Cur
		}

#if defined(DEBUG)
		printf("S: Changed value display: %d\n", sensor_show_value);
#endif
	}

	if (btn == BUTTON_BR) {
#if defined(DEBUG)
		printf("S: Clearing Values\n");
#endif
		sensorClear();
		sensorButtonPressed(0x49); // ---
	}

	if (btn == BUTTON_BL) {
#if defined(DEBUG)
		printf("S: Storing Config\n");
#endif
		eeSensorSet(sensor_show_type | (sensor_show_value << 4));
		sensorButtonPressed(0x4B); // UPd
	}

}

// -----------------------------------------------------------------------------------

void sensorI2CReset() {
	HAL_GPIO_WritePin(SENSOR_VCC_GPIO_Port, SENSOR_VCC_Pin, 0x00);
	HAL_Delay(500);
	HAL_GPIO_WritePin(SENSOR_VCC_GPIO_Port, SENSOR_VCC_Pin, 0x01);
	HAL_Delay(500);
}

HAL_StatusTypeDef sensorI2CMeasure() {
	uint8_t TXB[3];
	uint8_t RXB[6];
	HAL_StatusTypeDef res = HAL_OK;

	if (sensor_i2c_reset) {
		sensor_i2c_reset = false;
		sensorI2CReset();
	}

	// Init State
	TXB[0] = 0x71;
	res = HAL_I2C_Master_Transmit(&sensor_hi2c1, 0x70, TXB, 1, 1000);
#if defined(DEBUG)
	printf("S: TX [%02X] %02X\n", res, TXB[0]);
#endif
	if (res != HAL_OK) {
		return res;
	}

	// Read State
	res = HAL_I2C_Master_Receive(&sensor_hi2c1, 0x70, RXB, 1, 1000);
#if defined(DEBUG)
	printf("S: RX [%02X] %02X\n", res, RXB[0]);
#endif
	if (res != HAL_OK) {
		return res;
	}

	// Device needs special init sequence
	if ((RXB[0] & 0x18) != 0x18) {

		// -- 0x1b

		TXB[0] = 0x1b;
		TXB[1] = 0x00;
		TXB[2] = 0x00;

		res = HAL_I2C_Master_Transmit(&sensor_hi2c1, 0x70, TXB, 3, 1000);
#if defined(DEBUG)
		printf("S: TX [%02X] %02X %02X %02X\n", res, TXB[0], TXB[1], TXB[2]);
#endif
		if (res != HAL_OK) {
			return res;
		}
		HAL_Delay(5);
		res = HAL_I2C_Master_Receive(&sensor_hi2c1, 0x70, RXB, 3, 1000);
#if defined(DEBUG)
		printf("S: RX [%02X] %02X %02X %02X\n", res, RXB[0], RXB[1], RXB[2]);
#endif
		if (res != HAL_OK) {
			return res;
		}
		HAL_Delay(5);
		TXB[0] = 0xb0 | TXB[0];
		TXB[1] = RXB[1];
		TXB[2] = RXB[2];
		res = HAL_I2C_Master_Transmit(&sensor_hi2c1, 0x70, TXB, 3, 1000);
#if defined(DEBUG)
		printf("S: TX [%02X] %02X %02X %02X\n", res, TXB[0], TXB[1], TXB[2]);
#endif
		if (res != HAL_OK) {
			return res;
		}
		HAL_Delay(5);

		// -- 0x1c

		TXB[0] = 0x1c;
		TXB[1] = 0x00;
		TXB[2] = 0x00;

		res = HAL_I2C_Master_Transmit(&sensor_hi2c1, 0x70, TXB, 3, 1000);
#if defined(DEBUG)
		printf("S: TX [%02X] %02X %02X %02X\n", res, TXB[0], TXB[1], TXB[2]);
#endif
		if (res != HAL_OK) {
			return res;
		}
		HAL_Delay(5);
		res = HAL_I2C_Master_Receive(&sensor_hi2c1, 0x70, RXB, 3, 1000);
#if defined(DEBUG)
		printf("S: RX [%02X] %02X %02X %02X\n", res, RXB[0], RXB[1], RXB[2]);
#endif
		if (res != HAL_OK) {
			return res;
		}
		HAL_Delay(5);
		TXB[0] = 0xb0 | TXB[0];
		TXB[1] = RXB[1];
		TXB[2] = RXB[2];
		res = HAL_I2C_Master_Transmit(&sensor_hi2c1, 0x70, TXB, 3, 1000);
#if defined(DEBUG)
		printf("S: TX [%02X] %02X %02X %02X\n", res, TXB[0], TXB[1], TXB[2]);
#endif
		if (res != HAL_OK) {
			return res;
		}
		HAL_Delay(5);

		// -- 0x1e

		TXB[0] = 0x1e;
		TXB[1] = 0x00;
		TXB[2] = 0x00;

		res = HAL_I2C_Master_Transmit(&sensor_hi2c1, 0x70, TXB, 3, 1000);
#if defined(DEBUG)
		printf("S: TX [%02X] %02X %02X %02X\n", res, TXB[0], TXB[1], TXB[2]);
#endif
		if (res != HAL_OK) {
			return res;
		}
		HAL_Delay(5);
		res = HAL_I2C_Master_Receive(&sensor_hi2c1, 0x70, RXB, 3, 1000);
#if defined(DEBUG)
		printf("S: RX [%02X] %02X %02X %02X\n", res, RXB[0], RXB[1], RXB[2]);
#endif
		if (res != HAL_OK) {
			return res;
		}
		HAL_Delay(5);
		TXB[0] = 0xb0 | TXB[0];
		TXB[1] = RXB[1];
		TXB[2] = RXB[2];
		res = HAL_I2C_Master_Transmit(&sensor_hi2c1, 0x70, TXB, 3, 1000);
#if defined(DEBUG)
		printf("S: TX [%02X] %02X %02X %02X\n", res, TXB[0], TXB[1], TXB[2]);
#endif
		if (res != HAL_OK) {
			return res;
		}
		HAL_Delay(500);
	}

	// Request Measurement
	TXB[0] = 0xAC;
	TXB[1] = 0x33;
	TXB[2] = 0x00;
	res = HAL_I2C_Master_Transmit(&sensor_hi2c1, 0x70, TXB, 3, 1000);
#if defined(DEBUG)
	printf("S: TX [%02X] %02X %02X %02X\n", res, TXB[0], TXB[1], TXB[2]);
#endif
	if (res != HAL_OK) {
		return res;
	}
	HAL_Delay(100);

	// Measurement ready?
	RXB[0] = 0x80;
	uint8_t retry = 0;
	while ((RXB[0] & 0x80) == 0x80) {
		res = HAL_I2C_Master_Receive(&sensor_hi2c1, 0x70, RXB, 1, 1000);
#if defined(DEBUG)
		printf("S: RX [%02X] %02X\n", res, RXB[0]);
#endif
		if (res != HAL_OK) {
			return res;
		}
		HAL_Delay(2);
		retry++;
		if (retry > 100) {
			return HAL_ERROR;
		}
	}

	// Read Measurement
	res = HAL_I2C_Master_Receive(&sensor_hi2c1, 0x70, RXB, 6, 1000);
#if defined(DEBUG)
	printf("S: RX [%02X] %02X %02X %02X %02X %02X %02X\n", res, RXB[0], RXB[1], RXB[2], RXB[3], RXB[4], RXB[5]);
#endif
	if (res != HAL_OK) {
		return res;
	}

	uint32_t sensor_humid = 0;
	sensor_humid = ((sensor_humid | RXB[1]) << 8);
	sensor_humid = ((sensor_humid | RXB[2]) << 8);
	sensor_humid = ((sensor_humid | RXB[3]));
	sensor_humid = sensor_humid >> 4;
	int c1 = sensor_humid * 100 * 10 / 1024 / 1024;
	sensor_humid_cur = c1;
	if ((c1 < sensor_humid_min) | (sensor_humid_min == 0x7FFF)) {
		sensor_humid_min = c1;
	}
	if ((c1 > sensor_humid_max) | (sensor_humid_max == 0x7FFF)) {
		sensor_humid_max = c1;
	}
#if defined(DEBUG)
	printf("S: Measurement: %d %% (min %d %%) (max %d %%)\n", sensor_humid_cur, sensor_humid_min, sensor_humid_max);
#endif

	uint32_t sensor_temp = 0;
	sensor_temp = ((sensor_temp | RXB[3]) << 8);
	sensor_temp = ((sensor_temp | RXB[4]) << 8);
	sensor_temp = ((sensor_temp | RXB[5]));
	sensor_temp = sensor_temp & 0xfffff;
	int t1 = (sensor_temp * 200 * 10 / 1024 / 1024) - 500;
	sensor_temp_cur = t1;
	if ((t1 < sensor_temp_min) | (sensor_temp_min == 0x7FFF)) {
		sensor_temp_min = t1;
	}
	if ((t1 > sensor_temp_max) | (sensor_temp_max == 0x7FFF)) {
		sensor_temp_max = t1;
	}
#if defined(DEBUG)
	printf("S: Measurement: %d C (min %d C) (max %d C)\n", sensor_temp_cur, sensor_temp_min, sensor_temp_max);
#endif

	return HAL_OK;
}

// -----------------------------------------------------------------------------------

void sensorTick() {
	if (sensor_tick_count > 0) {
		sensor_tick_count--;
	} else {
		sensor_tick_count = SENSOR_CYCLE_S;
		if (sensorI2CMeasure() != HAL_OK) {
			sensor_i2c_reset = true;
			lcdShow(lcdChar(0x49)); // ---
			return;
		}
		sensorUpdateLCD();
	}
}

void sensorInit(I2C_HandleTypeDef hi2c1) {
	sensor_hi2c1 = hi2c1;
}

void sensorStart() {
	sensorClear();

	uint8_t cfg = eeSensorGet();
	sensor_show_type = cfg & 0x0F;
	sensor_show_value = (cfg & 0xF0) >> 4;
	sensor_i2c_reset = true;
	sensor_tick_count = 0;
}

void sensorStop() {
	HAL_GPIO_WritePin(SENSOR_VCC_GPIO_Port, SENSOR_VCC_Pin, 0x00);
}
