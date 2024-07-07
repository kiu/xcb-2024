/*
 * dev_lcd.c
 *
 *  Created on: Jun 2, 2024
 *      Author: kiu
 */

#include "dev_lcd.h"

#include "main.h"

static const uint8_t lcd_hunt_map[14] = { 8, 12, 7, 13, 9, 11, 6, 10, 19, 15, 20, 14, 18, 16 };

static const uint8_t lcd_chars[(0x004F + 0x0001) * 0x0003] = {
//   04      08      12
// 03  05  07  09  11  13
//   02      06      10
// 24  22  20  18  16  14
//   23      19      15
//       21      17

//        43210987    65432109    87654321
//      0b00000000, 0b00000000, 0b00000000,

		0b00000000, 0b11111100, 0b00000000, // 00 xx0
		0b00000000, 0b00110000, 0b00000000, // 01 xx1
		0b00000000, 0b11011010, 0b00000000, // 02 xx2
		0b00000000, 0b01111010, 0b00000000, // 03 xx3
		0b00000000, 0b00110110, 0b00000000, // 04 xx4
		0b00000000, 0b01101110, 0b00000000, // 05 xx5
		0b00000000, 0b11101110, 0b00000000, // 06 xx6
		0b00000000, 0b00111000, 0b00000000, // 07 xx7
		0b00000000, 0b11111110, 0b00000000, // 08 xx8
		0b00000000, 0b01111110, 0b00000000, // 09 xx9
		0b00000000, 0b10111110, 0b00000000, // 0A xxA
		0b00000000, 0b11100110, 0b00000000, // 0B xxb
		0b00000000, 0b11001100, 0b00000000, // 0C xxC
		0b00000000, 0b11110010, 0b00000000, // 0D xxd
		0b00000000, 0b11001110, 0b00000000, // 0E xxE
		0b00000000, 0b10001110, 0b00000000, // 0F xxF

		0b00001110, 0b00000001, 0b11000000, // 10 x0x
		0b00000010, 0b00000001, 0b00000000, // 11 x1x
		0b00001100, 0b00000001, 0b10100000, // 12 x2x
		0b00000110, 0b00000001, 0b10100000, // 13 x3x
		0b00000010, 0b00000001, 0b01100000, // 14 x4x
		0b00000110, 0b00000000, 0b11100000, // 15 x5x
		0b00001110, 0b00000000, 0b11100000, // 16 x6x
		0b00000010, 0b00000001, 0b10000000, // 17 x7x
		0b00001110, 0b00000001, 0b11100000, // 18 x8x
		0b00000110, 0b00000001, 0b11100000, // 19 x9x
		0b00001010, 0b00000001, 0b11100000, // 1A xAx
		0b00001110, 0b00000000, 0b01100000, // 1B xbx
		0b00001100, 0b00000000, 0b11000000, // 1C xCx
		0b00001110, 0b00000001, 0b00100000, // 1D xdx
		0b00001100, 0b00000000, 0b11100000, // 1E xEx
		0b00001000, 0b00000000, 0b11100000, // 1F xFx

		0b11100000, 0b00000000, 0b00011100, // 20 0xx
		0b00100000, 0b00000000, 0b00010000, // 21 1xx
		0b11000000, 0b00000000, 0b00011010, // 22 2xx
		0b01100000, 0b00000000, 0b00011010, // 23 3xx
		0b00100000, 0b00000000, 0b00010110, // 24 4xx
		0b01100000, 0b00000000, 0b00001110, // 25 5xx
		0b11100000, 0b00000000, 0b00001110, // 26 6xx
		0b00100000, 0b00000000, 0b00011000, // 27 7xx
		0b11100000, 0b00000000, 0b00011110, // 28 8xx
		0b01100000, 0b00000000, 0b00011110, // 29 9xx
		0b10100000, 0b00000000, 0b00011110, // 2A Axx
		0b11100000, 0b00000000, 0b00000110, // 2B bxx
		0b11000000, 0b00000000, 0b00001100, // 2C Cxx
		0b11100000, 0b00000000, 0b00010010, // 2D dxx
		0b11000000, 0b00000000, 0b00001110, // 2E Exx
		0b10000000, 0b00000000, 0b00001110, // 2F Fxx

		0b00010000, 0b00000000, 0b00000000, // 30 x.xx
		0b00000001, 0b00000000, 0b00000000, // 31 xx.x
		0b00000000, 0b00000000, 0b00000010, // 32 -xx
		0b00000000, 0b00000000, 0b00100000, // 33 x-x
		0b00000000, 0b00000010, 0b00000000, // 34 xx-

		0b11001110, 0b10111101, 0b11001100, // 35 CON
		0b10001100, 0b01110110, 0b01011110, // 36 PLY
		0b01101100, 0b10111100, 0b11101110, // 37 SEN

		0b10100000, 0b00000000, 0b00010110, // 38 Hxx
		0b11100000, 0b00000000, 0b00010100, // 39 Uxx
		0b10000000, 0b00000000, 0b00011110, // 3A Pxx
		0b01100000, 0b00000000, 0b00001110, // 3B Sxx
		0b00100000, 0b00000000, 0b00010010, // 3C |xx

		0b00000000, 0b00000000, 0b00001100, // 3D x^x TL
		0b00000000, 0b00011000, 0b00000000, // 3E xx^ TR
		0b00000000, 0b01100000, 0b00000000, // 3F xx^ BR
		0b11000000, 0b00000000, 0b00000000, // 40 x^x BL

		0b00001100, 0b00000000, 0b11011110, // 41 °Cx
		0b00000000, 0b10100110, 0b00000000, // 42 xxh
		0b00001100, 0b10100110, 0b11011110, // 43 °Ch

		0b11001110, 0b10000010, 0b00001100, // 44 Cur
		0b11001110, 0b00000000, 0b00100100, // 45 Lo
		0b10101000, 0b00000000, 0b00010110, // 46 Hi

		0b00001010, 0b00011110, 0b00111110, // 47 ono
		0b00001110, 0b00011110, 0b00011110, // 48 ouo

		0b00000000, 0b00000010, 0b00100010, // 49 ---

		0b10100000, 0b11000100, 0b00111100, // 4A N-L

		0b11101000, 0b11110011, 0b11110100, // 4B UPd

		0b11101010, 0b10000101, 0b11010100, // 4C UNI
		0b10101110, 0b10111101, 0b01010110, // 4D HUN
		0b01101100, 0b11111100, 0b11001110, // 4E SCO
		0b11000110, 0b11001101, 0b01101100, // 4F CYC
		};

// ----------------------------------------------------------------------------

static uint32_t lcd_last = 0;

// ----------------------------------------------------------------------------

uint32_t lcdChar(uint8_t loc) {
	uint32_t buffer = 0;
	buffer |= ((uint32_t) lcd_chars[(loc * 3) + 2]) << 0;
	buffer |= ((uint32_t) lcd_chars[(loc * 3) + 1]) << 8;
	buffer |= ((uint32_t) lcd_chars[(loc * 3) + 0]) << 16;
	return buffer;
}

uint32_t lcdDigits_1(uint8_t base, int16_t value, uint8_t dot1, uint8_t dot2, uint8_t minus) {
	uint32_t lcd = 0;
	uint8_t fail = 0;

	if (value < 0) {
		value *= -1;
		if (minus) {
			fail |= 1;
		}
	}
	fail |= value >= base;

	if (fail) {
		return lcdChar(0x34);
	}

	lcd |= lcdChar(value);

	if (dot1) {
		lcd |= lcdChar(0x30);
	}
	if (dot2) {
		lcd |= lcdChar(0x31);
	}

	return lcd;
}

uint32_t lcdDigits_2(uint8_t base, int16_t value, uint8_t dot1, uint8_t dot2, uint8_t minus) {
	uint32_t lcd = 0;
	uint8_t fail = 0;
	uint8_t isminus = 0;
	if (value < 0) {
		isminus = 1;
		value *= -1;
		if (minus) {
			fail |= value >= base;
		}
	}
	fail |= value >= base * base;

	if (fail) {
		lcd |= lcdChar(0x33);
		lcd |= lcdChar(0x34);
		return lcd;
	}

	int16_t d = value / base;
	if (isminus && minus && d == 0) {
		lcd |= lcdChar(0x33);
	} else {
		lcd |= lcdChar(0x10 + d);
	}

	value = value - (d * base);
	lcd |= lcdDigits_1(base, value, 0, 0, 0);

	if (dot1) {
		lcd |= lcdChar(0x30);
	}
	if (dot2) {
		lcd |= lcdChar(0x31);
	}

	return lcd;
}

uint32_t lcdDigits_3(uint8_t base, int16_t value, uint8_t dot1, uint8_t dot2, uint8_t minus) {
	uint32_t lcd = 0;
	uint8_t fail = 0;
	uint8_t isminus = 0;

	if (value < 0) {
		isminus = 1;
		value *= -1;
		if (minus) {
			fail |= value >= base * base;
		}
	}

	fail |= value >= base * base * base;

	if (fail) {
		lcd |= lcdChar(0x49);
		return lcd;
	}

	int16_t d = value / (base * base);
	if (isminus && minus && d == 0) {
		lcd |= lcdChar(0x32);
	} else {
		lcd |= lcdChar(0x20 + d);
	}

	value = value - (d * base * base);
	lcd |= lcdDigits_2(base, value, 0, 0, 0);

	if (dot1) {
		lcd |= lcdChar(0x30);
	}
	if (dot2) {
		lcd |= lcdChar(0x31);
	}

	return lcd;
}

uint32_t lcdMap(uint16_t unlocks) {
	uint32_t lcd = 0;
	for (uint8_t i = 0; i < 14; i++) {
		if (unlocks & (((uint16_t) 0x0001) << i)) {
			lcd |= ((uint32_t) 0x00000001) << (lcd_hunt_map[i] - 1);
		}
	}
	return lcd;
}

// ----------------------------------------------------------------------------

void lcdWriteBit(uint8_t bit) {
	HAL_GPIO_WritePin(LCD_WRITE_GPIO_Port, LCD_WRITE_Pin, 0x00);
	HAL_GPIO_WritePin(LCD_DATA_GPIO_Port, LCD_DATA_Pin, bit);
	HAL_GPIO_WritePin(LCD_WRITE_GPIO_Port, LCD_WRITE_Pin, 0x01);
}

void lcdWriteCmd(uint16_t data) {
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, 0x01);
	HAL_GPIO_WritePin(LCD_WRITE_GPIO_Port, LCD_WRITE_Pin, 0x01);
	HAL_GPIO_WritePin(LCD_DATA_GPIO_Port, LCD_DATA_Pin, 0x01);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, 0x00);
	for (uint8_t i = 0; i < 12; i++) {
		lcdWriteBit((data >> (11 - i)) & 0x01);
	}
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, 0x01);
}

void lcdWriteData(uint32_t data) {
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, 0x01);
	HAL_GPIO_WritePin(LCD_WRITE_GPIO_Port, LCD_WRITE_Pin, 0x01);
	HAL_GPIO_WritePin(LCD_DATA_GPIO_Port, LCD_DATA_Pin, 0x01);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, 0x00);

	lcdWriteBit(0x01);
	lcdWriteBit(0x00);
	lcdWriteBit(0x01);

	lcdWriteBit(0x00);
	lcdWriteBit(0x00);
	lcdWriteBit(0x00);
	lcdWriteBit(0x00);
	lcdWriteBit(0x00);
	lcdWriteBit(0x00);

	for (uint8_t i = 0; i < 32; i++) {
		lcdWriteBit((data >> i) & 0x01);
		lcdWriteBit(0x00);
		lcdWriteBit(0x00);
		lcdWriteBit(0x00);
	}

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, 0x01);
}

// ----------------------------------------------------------------------------

void lcdShow(uint32_t data) {
	lcd_last = data;
	lcdWriteData(data);
}

void lcdAdd(uint32_t data) {
	lcdShow(lcd_last | data);
}

void lcdRemove(uint32_t data) {
	lcdShow(lcd_last & ~data);
}

void lcdDisable() {
	lcdShow(0);
	lcdWriteCmd(0b100000000100); // LCD OFF  	1 0 0 0000-0010-X
}

void lcdInit() {
	lcdWriteCmd(0b100000000010); // SYS EN 		1 0 0 0000-0001-X
//	lcdWriteCmd(0b100001000000); // BIAS 1/2  	1 0 0 0010-abX0-X
	lcdWriteCmd(0b100001000010); // BIAS 1/3  	1 0 0 0010-abX1-X
	lcdWriteCmd(0b100000000110); // LCD ON   	1 0 0 0000-0011-X
	lcdWriteCmd(0b100101000000); //	F1 			1 0 0 101X-X000-X
	lcdWriteCmd(0b100000011000); // CLR TIMER 	1 0 0 0000-11XX-X
	lcdWriteCmd(0b100000001100); // TIMER EN 	1 0 0 0000-0110-X
	lcdWriteCmd(0b100100010000); // IRQ	EN		1 0 0 100X-1XXX-X
	lcdShow(0);
}
