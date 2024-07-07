/*
 * mode_play.c
 *
 *  Created on: Jun 2, 2024
 *      Author: kiu
 */

#include "mode_play.h"

#include "dev_buzz.h"
#include "dev_eeprom.h"
#include "dev_lcd.h"

#include <stdlib.h>

static uint8_t play_sequence[PLAY_SEQ_MAX];
static uint8_t play_sequence_current = 0;
static uint8_t play_sequence_max = 0;

static uint8_t play_timeout = 0xFF;

static uint16_t play_speed = 0;

static uint32_t play_seed = 0;

typedef enum {
	PLAY_INIT, PLAY_MENU, PLAY_RUN,
} play_mode_t;

static play_mode_t play_mode = PLAY_INIT;

typedef enum {
	PLAY_LCD_HIGHSCORE,
	PLAY_LCD_SCORE,
	PLAY_LCD_MENU,
	PLAY_LCD_TL,
	PLAY_LCD_TR,
	PLAY_LCD_BR,
	PLAY_LCD_BL,
	PLAY_LCD_FAIL,
	PLAY_LCD_WIN,
	PLAY_LCD_EMPTY,
} play_lcd_t;

// ----------------------------------------------------------------------------

void playLCD(play_lcd_t lcd) {

	if (lcd == PLAY_LCD_HIGHSCORE) {
		uint32_t lcd = 0;
		lcd |= lcdChar(0x38); // Hxx
		lcd |= lcdDigits_2(10, eeHighscoreGet(), 0, 0, 0);
		lcdShow(lcd); // H88
	}

	if (lcd == PLAY_LCD_SCORE) {
		uint32_t lcd = 0;
		lcd |= lcdChar(0x3B); // Sxx
		lcd |= lcdDigits_2(10, play_sequence_max, 0, 0, 0);
		lcdShow(lcd); // S88
	}

	if (lcd == PLAY_LCD_MENU) {
		lcdShow(lcdChar(0x4A)); // NXL
	}

	if (lcd == PLAY_LCD_TL) {
		lcdShow(lcdChar(0x3D));
	}
	if (lcd == PLAY_LCD_TR) {
		lcdShow(lcdChar(0x3E));
	}
	if (lcd == PLAY_LCD_BR) {
		lcdShow(lcdChar(0x3F));
	}
	if (lcd == PLAY_LCD_BL) {
		lcdShow(lcdChar(0x40));
	}

	if (lcd == PLAY_LCD_FAIL) {
		lcdShow(lcdChar(0x47)); // ono
	}

	if (lcd == PLAY_LCD_WIN) {
		lcdShow(lcdChar(0x48)); // ouo
	}

	if (lcd == PLAY_LCD_EMPTY) {
		lcdShow(0);
	}
}

void playSeed() {
	play_seed = HAL_GetTick() ^ HAL_GetUIDw0() ^ eeSeed() ^ HAL_GetUIDw1() ^ HAL_GetUIDw2();

#if defined(DEBUG)
	printf("P: New Seed 0x%08X\n", (unsigned int) play_seed);
#endif

	srand(play_seed);

	for (uint8_t i = 0; i < PLAY_SEQ_MAX; i++) {
		play_sequence[i] = rand() % 4;
	}
}

// ----------------------------------------------------------------------------

void playStartScreen() {
#if defined(DEBUG)
	printf("P: Start screen\n");
#endif

	play_mode = PLAY_MENU;

	playLCD(PLAY_LCD_HIGHSCORE);
	HAL_Delay(1500);

	playLCD(PLAY_LCD_MENU);

	buttonClear();
}

void playShowSequence() {
	uint16_t speed_org = play_speed;
	uint16_t speed_new = 0;
	for (uint8_t i = 0; i < PLAY_SEQ_LEN; i++) {
		if (play_sequence_max >= PLAY_SEQ_THRESHOLD[i]) {
			speed_new = PLAY_SEQ_SPEED[i];
		}
	}
	if (speed_new != speed_org) {
		play_speed = speed_new;
#if defined(DEBUG)
		printf("P: Changed Speed: %d (ms)\n", play_speed);
#endif
	}

#if defined(DEBUG)
	printf("P: Sequence [%d]: ", play_sequence_max + 1);
#endif
	for (uint8_t i = 0; i < play_sequence_max + 1; i++) {
		uint8_t c = play_sequence[i];
#if defined(DEBUG)
		printf("%d ", c);
#endif
		playLCD(PLAY_LCD_TL + c);
		//buzzARR(PLAY_BUTTON_TONES[c], play_speed, 0);
		buzzNote(PLAY_NOTES[c], PLAY_OCTAVES[c], play_speed, 0);
		playLCD(PLAY_LCD_EMPTY);
		HAL_Delay(PLAY_SEQ_PAUSE_MS);
	}
#if defined(DEBUG)
	printf("\n");
#endif

	buttonClear();
	play_timeout = PLAY_TIMEOUT_S;
}

void playVictoryIfNeeded(uint8_t btn) {
	for (uint8_t i = 1; i < PLAY_SEQ_LEN; i++) {
		if (play_sequence_max == PLAY_SEQ_THRESHOLD[i]) {
#if defined(DEBUG)
			printf("P: Sequence Victory\n");
#endif
			HAL_Delay(PLAY_SEQ_NEXT_MS);
			buzzNote(PLAY_NOTES[btn], PLAY_OCTAVES[btn], PLAY_VICTORY_TONE_LENGTH_0_MS, 0);
			HAL_Delay(PLAY_VICTORY_TONE_PAUSE_MS);
			for (uint8_t i = 1; i < 6; i++) {
				buzzNote(PLAY_NOTES[btn], PLAY_OCTAVES[btn], PLAY_VICTORY_TONE_LENGTH_12345_MS, 0);
				HAL_Delay(PLAY_VICTORY_TONE_PAUSE_MS);
			}
		}
	}
}

void playLost() {
#if defined(DEBUG)
	printf("P: You lost\n");
#endif

	playLCD(PLAY_LCD_FAIL);
	buzzNote(PLAY_NOTES[4], PLAY_OCTAVES[4], PLAY_LOSE_TONE_LENGTH_MS, 0);

	playLCD(PLAY_LCD_SCORE);
	HAL_Delay(1500);

	playStartScreen();
}

// ----------------------------------------------------------------------------

void playButton(button_t btn) {

	if (play_mode == PLAY_MENU) {
		playLCD(PLAY_LCD_EMPTY);
		buzzTouch();

		play_mode = PLAY_RUN;
		play_sequence_current = 0;
		play_sequence_max = 0;

		if (btn == BUTTON_TL || btn == BUTTON_BL) {
#if defined(DEBUG)
			printf("P: Start New Sequence\n");
#endif
			playSeed();
		}

		if (btn == BUTTON_TR || btn == BUTTON_BR) {
#if defined(DEBUG)
			printf("P: Start Last Sequence\n");
#endif
		}

		HAL_Delay(PLAY_SEQ_NEXT_MS);
		playShowSequence();
		return;
	}

	if (play_mode == PLAY_RUN) {
		buttonClear();

		if (play_sequence[play_sequence_current] != btn) {
			playLost();
			return;
		} else {
			playLCD(PLAY_LCD_TL + btn);
			buzzNote(PLAY_NOTES[btn], PLAY_OCTAVES[btn], play_speed, 0);
			playLCD(PLAY_LCD_EMPTY);
			HAL_Delay(PLAY_SEQ_PAUSE_MS);
		}

		play_sequence_current++;

		if (play_sequence_current == play_sequence_max + 1) {
			play_sequence_current = 0;
			play_sequence_max++;

			if (play_sequence_max > eeHighscoreGet()) {
				eeHighscoreSet(play_sequence_max);
			}

			if (play_sequence_max == PLAY_SEQ_MAX) {
#if defined(DEBUG)
				printf("P: Sequence max reached!\n");
#endif
				playLCD(PLAY_LCD_WIN);
				HAL_Delay(1500);
				playLCD(PLAY_LCD_SCORE);
				HAL_Delay(1500);
				playStartScreen();
				return;
			}

			playVictoryIfNeeded(btn);

			HAL_Delay(PLAY_SEQ_NEXT_MS);
			playShowSequence();
		} else {
			play_timeout = PLAY_TIMEOUT_S;
		}
	}
}

void playTick() {
	if (play_mode == PLAY_RUN) {
		if (play_timeout > 0) {
			play_timeout--;
		} else {
#if defined(DEBUG)
			printf("P: Timeout (%d s)\n", PLAY_TIMEOUT_S);
#endif
			playLost();
		}
	}
}

void playStart() {
	playSeed();
	playStartScreen();
}

void playStop() {
}
