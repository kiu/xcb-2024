/*
 * mode_play.h
 *
 *  Created on: Jun 2, 2024
 *      Author: kiu
 */

#ifndef INC_MODE_PLAY_H_
#define INC_MODE_PLAY_H_

#include "main.h"
#include <stdbool.h>
#include "stm32f0xx_hal.h"

#if defined(DEBUG)
#include "stdio.h"
#endif

/*
 https://www.waitingforfriday.com/?p=586

 Frequencies
 TL Red		252 Hz	B3  247.942 Hz	C4 261.626 Hz
 TR Blue	415 Hz 	G#4 415.305 Hz	G4 391.995 Hz
 BR Yellow	310 Hz 	D#4 311.127 Hz	E4 329.628 Hz
 BL Green	209 Hz 	G#3 207.652 Hz	G3 195.998 Hz
 Losing		42 Hz

 Timing Tones
 Losing	tone	1500 ms
 Seq 01-05	tone 420 ms
 Seq 06-13	tone 320 ms
 Seq 14-31	tone 220 ms

 Timing pause
 Seq tones	 	  50 ms
 Seq Next		 800 ms
 Timeout 		3000 ms

 Victory
 Last color repeat	6
 Start delay 800 ms
 1	tone 20ms pause 20ms
 1+ tone 70ms pause 20ms
 */

// TL, TR, BR, BL, Lost // B3 G#4 D#4 G#3
//static const uint8_t PLAY_OCTAVES[5] = { 0, 1, 1, 0, 0 };
//static const uint8_t PLAY_NOTES[5] = { 9, 8, 3, 8, 0 };

// TL, TR, BR, BL, Lost // C4 G4 E4 G3
static const uint8_t PLAY_OCTAVES[5] = { 1, 1, 1, 0, 0 };
static const uint8_t PLAY_NOTES[5] = { 0, 7, 4, 7, 0 };

static const uint8_t PLAY_SEQ_LEN = 5;
static const uint8_t PLAY_SEQ_THRESHOLD[5] = { 0, 5, 13, 31, 63 };
static const uint16_t PLAY_SEQ_SPEED[5] = { 420, 320, 220, 120, 80 };

#define PLAY_SEQ_MAX 99

#define PLAY_SEQ_PAUSE_MS 50
#define PLAY_SEQ_NEXT_MS 800

#define PLAY_VICTORY_TONE_LENGTH_0_MS 20
#define PLAY_VICTORY_TONE_LENGTH_12345_MS 70
#define PLAY_VICTORY_TONE_PAUSE_MS 20

#define PLAY_LOSE_TONE_LENGTH_MS 1500

#define PLAY_TIMEOUT_S 4

void playButton(button_t btn);
void playTick();
void playStart();
void playStop();

#endif /* INC_MODE_PLAY_H_ */
