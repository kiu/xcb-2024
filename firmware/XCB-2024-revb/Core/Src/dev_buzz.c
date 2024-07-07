/*
 * dev_buzz.c
 *
 *  Created on: Jun 2, 2024
 *      Author: kiu
 */

#include "dev_buzz.h"

// see generate_note_table.py

static const uint16_t notes[] = { 61155, // C  00 130.81
		57722, // C# 01 138.59
		54482, // D  02 146.83
		51424, // D# 03 155.56
		48538, // E  04 164.81
		45814, // F  05 174.61
		43242, // F# 06 185.0
		40815, // G  07 196.0
		38524, // G# 08 207.65
		36362, // A  09 220.0
		34321, // A# 10 233.08
		32395, // H  11 246.94
		30577, // C  12 261.63
		28860, // C# 13 277.18
		27240, // D  14 293.66
		25711, // D# 15 311.13
		24268, // E  16 329.63
		22906, // F  17 349.23
		21620, // F# 18 369.99
		20407, // G  19 392.0
		19261, // G# 20 415.3
		18180, // A  21 440.0
		17160, // A# 22 466.16
		16197, // H  23 493.88
		15288, // C  24 523.25
		14429, // C# 25 554.37
		13619, // D  26 587.33
		12855, // D# 27 622.25
		12133, // E  28 659.26
		11452, // F  29 698.46
		10809, // F# 30 739.99
		10203, // G  31 783.99
		9630,  // G# 32 830.61
		9089,  // A  33 880.0
		8579,  // A# 34 932.33
		8098,  // H  35 987.77
		7643,  // C  36 1046.5
		7214,  // C# 37 1108.73
		6809,  // D  38 1174.66
		6427,  // D# 39 1244.51
		6066,  // E  40 1318.51
		5725,  // F  41 1396.91
		5404,  // F# 42 1479.98
		5101,  // G  43 1567.98
		4814,  // G# 44 1661.22
		4544,  // A  45 1760.0
		4289,  // A# 46 1864.66
		4048,  // H  47 1975.53
		3821,  // C  48 2093.0
		3606,  // C# 49 2217.46
		3404,  // D  50 2349.32
		3213,  // D# 51 2489.02
		3032,  // E  52 2637.02
		2862,  // F  53 2793.83
		2701,  // F# 54 2959.96
		2550,  // G  55 3135.96
		2406,  // G# 56 3322.44
		2271,  // A  57 3520.0
		2144,  // A# 58 3729.31
		};
// ----------------------------------------------------------------------------

static TIM_HandleTypeDef buzz_htim;

// ----------------------------------------------------------------------------

// https://deepbluembedded.com/stm32-timer-calculator/
void buzzARR(uint16_t arr, uint16_t duration, uint16_t delay) {
	buzz_htim.Instance->ARR = arr;
	buzz_htim.Instance->CCR1 = arr / 2;
	buzz_htim.Instance->CNT = 0;

	HAL_TIM_PWM_Start(&buzz_htim, TIM_CHANNEL_1);
	HAL_Delay(duration);
	HAL_TIM_PWM_Stop_IT(&buzz_htim, TIM_CHANNEL_1);
	HAL_Delay(delay);
}

// ----------------------------------------------------------------------------

void buzzNote(uint8_t note, uint8_t octave, uint16_t duration, uint16_t delay) {
	buzzARR(notes[note + (12 * octave)], duration, delay);
}

/*
void buzzMorse(const char msg[], uint8_t len, uint16_t freq_dot, uint16_t freq_dash, uint8_t duration) {
	//const char msg[] = "-..-"; // X
	//buzzMorse(msg, sizeof(msg), 6067, 7644, 50);

	const uint8_t dot_len = duration;
	for (uint8_t i = 0; i < len; i++) {
		if (msg[i] == '.') {
			buzzARR(freq_dot, dot_len, dot_len);
		}
		if (msg[i] == '-') {
			buzzARR(freq_dash, dot_len * 3, dot_len);
		}
		if (msg[i] == ' ') {
			HAL_Delay(dot_len * 2);
		}
	}
}
*/

// ----------------------------------------------------------------------------

void buzzTouch() {
	buzzARR(2600, 50, 10);
}

void buzzInit(TIM_HandleTypeDef htim) {
	buzz_htim = htim;

	// Mario Coin / Gameboy Power-On
	buzzNote(11, 2, 100, 0);
	buzzNote(4, 3, 500, 0);
}
