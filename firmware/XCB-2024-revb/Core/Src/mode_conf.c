/*
 * mode_conf.c
 *
 *  Created on: Jun 2, 2024
 *      Author: kiu
 */

#include "mode_conf.h"

#include "dev_buzz.h"
#include "dev_eeprom.h"
#include "dev_lcd.h"

typedef enum {
	SHOW_UNIQUE, SHOW_HUNT, SHOW_SCORE, SHOW_CYCLE
} show_t;

static show_t conf_show = SHOW_CYCLE;
static show_t conf_cycle = SHOW_SCORE;
static uint8_t conf_tick_display = 0;

static uint8_t conf_tick_tx = 0;

static uint8_t conf_unique = 0;
static uint16_t conf_hunt = 0;

volatile static bool conf_clear_dot1 = false;
volatile static bool conf_clear_dot2 = false;

static TIM_HandleTypeDef conf_htim_pwm;
static TIM_HandleTypeDef conf_htim_enc;
static TIM_HandleTypeDef conf_htim_dec;

static volatile bool conf_ir_tx_run = false;
static volatile uint8_t conf_ir_tx_idx = 0;
static uint32_t conf_ir_tx_buf = 0;

static volatile bool conf_ir_rx_en = false;
static volatile bool conf_ir_rx_run = false;
static volatile uint8_t conf_ir_rx_idx = 0;
static volatile uint32_t conf_ir_rx_buf = 0;
static volatile uint8_t conf_ir_rx_msg_idx = 0;
static volatile uint32_t conf_ir_rx_msg[CONF_RX_BUF];

static uint32_t conf_unlock_bank_a[8];
static uint32_t conf_unlock_bank_b[8];
static bool conf_unlock_bank_toggle = false;
static uint8_t conf_tick_unlock_bank = 0;

static bool conf_konami_enabled = false;
static uint8_t conf_konami_sequence[8];
static const uint8_t conf_konami[8] = { 0, 0, 2, 2, 3, 1, 3, 1 };

// ----------------------------------------------------------------------------
bool confAllowSleep() {
	return !conf_ir_rx_run;
}

void confUpdateLCD() {

	uint32_t lcd = 0;

	if (badgeTypeCheck(eeIDGet()) != BADGE_PARTICIPANT) {
		lcd |= lcdChar(0x39); // Uxx
		lcd |= lcdDigits_2(16, eeIDGet(), 0, 0, 0);
		lcdShow(lcd);
		return;
	}

	if (conf_show == SHOW_UNIQUE || (conf_show == SHOW_CYCLE && conf_cycle == SHOW_UNIQUE)) {
		if (!conf_konami_enabled) {
			lcd |= lcdDigits_3(10, conf_unique, 0, 0, 0);
		} else {
			lcd |= lcdDigits_3(10, 999, 0, 0, 0);
		}
	}
	if (conf_show == SHOW_HUNT || (conf_show == SHOW_CYCLE && conf_cycle == SHOW_HUNT)) {
		lcd |= lcdChar(0x38); // Hxx
		if (!conf_konami_enabled) {
			lcd |= lcdMap(eeIDTreasureGet());
		} else {
			lcd |= lcdMap(0b0000001111111111);
		}
	}
	if (conf_show == SHOW_SCORE || (conf_show == SHOW_CYCLE && conf_cycle == SHOW_SCORE)) {
		lcd |= lcdChar(0x3A); // Pxx
		if (!conf_konami_enabled) {
			lcd |= lcdDigits_2(10, eeHighscoreGet(), 0, 0, 0);
		} else {
			lcd |= lcdDigits_2(10, 99, 0, 0, 0);
		}
	}

	lcdShow(lcd);
}

void confButtonPressed(show_t show, uint8_t lcd_char) {
	lcdShow(lcdChar(lcd_char));
	buzzTouch();
	HAL_Delay(BUTTON_PRESS_DELAY * 2);
	conf_show = show;
	confUpdateLCD();
}

void confButton(button_t btn) {
	for (uint8_t i = 0; i < 7; i++) {
		conf_konami_sequence[i] = conf_konami_sequence[i + 1];
	}
	conf_konami_sequence[7] = btn;

	bool matched = true;
	for (uint8_t i = 0; i < 8; i++) {
		if (conf_konami_sequence[i] != conf_konami[i]) {
			matched = false;
			break;
		}
	}
	if (matched) {
		conf_konami_enabled = true;

		// Everytime We Touch
		// https://youtu.be/8SKWXf2yb5k?t=64
		const uint8_t o = 3;
		const uint16_t n = 160;
		const uint16_t p = 20;

		uint32_t lcd = 0;
		lcd |= (1 << 3);
		lcdShow(lcd);
		buzzNote(8, o, n, p); // G#
		buzzNote(8, o, n, p); // G#
		buzzNote(7, o, n, p); // G
		buzzNote(7, o, n, p); // G
		lcd |= (1 << 7);
		lcdShow(lcd);
		buzzNote(5, o, n, p); // F
		buzzNote(5, o, n * 3, p); // F
		buzzNote(5, o, n, p); // F
		buzzNote(7, o, n * 2, p); // G
		lcd |= (1 << 11);
		lcdShow(lcd);
		buzzNote(7, o, n, p); // G
		buzzNote(7, o, n * 2, p); // G
		buzzNote(8, o, n * 2, p); // G#
		buzzNote(8, o, n, p); // G#
		lcd |= (1 << 12);
		lcdShow(lcd);
		buzzNote(8, o, n, p); // G#
		buzzNote(7, o, n, p); // G
		buzzNote(7, o, n, p); // G
		buzzNote(5, o, n, p); // F
		lcd |= (1 << 13);
		lcdShow(lcd);
		buzzNote(5, o, n * 2, p); // F
		buzzNote(8, o, n, p); // G#
		buzzNote(10, o, n * 2, p); // A#
		buzzNote(8, o, n, p); // G#
		lcd |= (1 << 14);
		lcdShow(lcd);
		buzzNote(7, o, n, p); // G
		buzzNote(7, o, n * 2, p); // G
		buzzNote(5, o, n * 3, p); // F
		buzzNote(8, o, n, p); // G#
		lcd |= (1 << 18);
		lcdShow(lcd);
		buzzNote(7, o, n, p); // G
		buzzNote(7, o, n, p); // G
		buzzNote(5, o, n, p); // F
		buzzNote(5, o, n * 2, p); // F
		lcd |= (1 << 22);
		lcdShow(lcd);
		buzzNote(8, o, n * 2, p); // G#
		buzzNote(3, o, n * 3, p); // D#
		buzzNote(8, o, n, p); // G#
		buzzNote(7, o, n, p); // G
		lcd |= (1 << 23);
		lcdShow(lcd);
		buzzNote(5, o, n, p); // F
		buzzNote(7, o, n, p); // G
		buzzNote(3, o, n, p); // D#
		buzzNote(8, o, n * 4, p); // G#
		lcd |= (1 << 2);
		lcdShow(lcd);
		buzzNote(8, o, n * 2, p); // G#
		buzzNote(3, o, n, p); // D#
		buzzNote(3, o, n * 2, p); // D#
		buzzNote(0, o, n * 2, p); // C
		buzzNote(3, o, n * 4, p); // D#
		lcd |= (1 << 5);
		lcdShow(lcd);
		HAL_Delay(1000);
	}

	if (btn == BUTTON_TL) {
#if defined(DEBUG)
		printf("C: Show change: Unique\n");
#endif
		confButtonPressed(SHOW_UNIQUE, 0x4C);
	}
	if (btn == BUTTON_TR) {
#if defined(DEBUG)
		printf("C: Show change: Hunt\n");
#endif
		confButtonPressed(SHOW_HUNT, 0x4D);
	}
	if (btn == BUTTON_BR) {
#if defined(DEBUG)
		printf("C: Show change: Score\n");
#endif
		confButtonPressed(SHOW_SCORE, 0x4E);
	}
	if (btn == BUTTON_BL) {
#if defined(DEBUG)
		printf("C: Show change: Cycle\n");
#endif
		conf_tick_display = CONF_CYCLE_S;
		confButtonPressed(SHOW_CYCLE, 0x4F);
	}
}

void confUpdateUniqueCount() {
	conf_unique = eeIDUniqueCount();
}

void confUpdateHuntCount() {
	conf_hunt = eeIDTreasureGet();
}

bool confUnlockBank(uint8_t id) {
	uint8_t offset = id / 32;
	uint32_t mask = (0x00000001 << (id - (offset * 32)));

	bool exist_a = conf_unlock_bank_a[offset] & mask;
	bool exist_b = conf_unlock_bank_b[offset] & mask;

	if (conf_unlock_bank_toggle) {
		conf_unlock_bank_a[offset] |= mask;
	} else {
		conf_unlock_bank_b[offset] |= mask;
	}

	return exist_a || exist_b;
}

void confUnlock(uint8_t id) {

	if (id == ID_CMD_TEST) {
#if defined(DEBUG)
		printf("C: CMD TEST\n");
#endif
		conf_tick_tx = 0;
		lcdShow(lcdChar(0x33)); // x-x
		buzzNote(0, 4, 100, 60);
		buzzNote(0, 4, 100, 60);
		HAL_Delay(100);
		conf_tick_display = 8;
		return;
	}

	if (id == ID_CMD_WIPE) {
#if defined(DEBUG)
		printf("C: CMD WIPE\n");
#endif
		lcdShow(lcdChar(0x49)); // ---
		buzzNote(0, 4, 80, 30);
		buzzNote(2, 4, 80, 30);
		buzzNote(2, 4, 80, 30);
		buzzNote(0, 4, 80, 30);
		eeWipe(true);
		HAL_Delay(1000);
		HAL_NVIC_SystemReset();
		return;
	}

	badge_t badge_my = badgeTypeCheck(eeIDGet());
	badge_t badge_rx = badgeTypeCheck(id);

	if (badge_my == BADGE_TREASURE) {
		return;
	}

	if (badge_my == BADGE_OTHER) {
		uint32_t lcd = 0;
		lcd |= lcdChar(0x3C);
		lcd |= lcdDigits_2(16, id, 0, 0, 0);
		lcdShow(lcd);
		buzzNote(0, 3, 200, 40);
		buzzNote(0, 3, 200, 40);
		conf_tick_display = 8;
		return;
	}

	if (badge_rx == BADGE_TREASURE) {
		if (!eeIDUnlock(id)) {
			return;
		}

		confUpdateHuntCount();

#if defined(DEBUG)
		printf("C: Increasing Hunt to %d\n", conf_hunt);
#endif

		conf_cycle = SHOW_HUNT;
		if (conf_show == SHOW_CYCLE) {
			conf_tick_display = CONF_CYCLE_S;
		}

		confUpdateLCD();

		// Indiana Jones
		// https://www.youtube.com/watch?v=BCSdW6LfsZk

		const uint8_t o = 2;
		const uint16_t n = 180;
		const uint16_t p = 40;

		buzzNote(4, o, n, p); // E
		buzzNote(5, o, n, p); // F
		buzzNote(7, o, n, p); // G
		buzzNote(0, o + 1, n * 2, p); // C
		buzzNote(2, o, n, p); // D
		buzzNote(4, o, n, p); // E
		buzzNote(5, o, n * 3, p); // F
		buzzNote(7, o, n, p); // G
		buzzNote(9, o, n, p); // A
		buzzNote(10, o, n, p); // A#
		buzzNote(5, o + 1, n * 2, p); // F
		buzzNote(9, o, n, p); // A
		buzzNote(10, o, n, p); // A#
		buzzNote(0, o + 1, n * 2, p); // C
		buzzNote(2, o + 1, n * 2, p); // D
		buzzNote(4, o + 1, n * 2, p); // E
		buzzNote(4, o, n, p); // E
		buzzNote(5, o, n, p); // F
		buzzNote(7, o, n, p); // G
		buzzNote(0, o + 1, n * 3, p); // C
		buzzNote(2, o + 1, n, p); // D
		buzzNote(4, o + 1, n, p); // E
		buzzNote(5, o + 1, n * 4, p); // F

		return;
	}

	if (badge_rx == BADGE_PARTICIPANT) {

		if (!confUnlockBank(id)) {
#if defined(DEBUG)
			printf("C: Unlock 1 of 2 (ID: 0x%02X)\n", id);
#endif
			return;
		}

#if defined(DEBUG)
		printf("C: Unlock 2 of 2 (ID: 0x%02X)\n", id);
#endif

		if (!eeIDUnlock(id)) {
			return;
		}

		confUpdateUniqueCount();

#if defined(DEBUG)
		printf("C: Increasing Unique to %d\n", conf_unique);
#endif

		conf_cycle = SHOW_UNIQUE;
		if (conf_show == SHOW_CYCLE) {
			conf_tick_display = CONF_CYCLE_S;
		}

		confUpdateLCD();

		// Pokemon Level Up
		// https://www.youtube.com/watch?v=0c8qdqPTFjI
		const uint8_t o = 3;
		const uint16_t n = 140;
		const uint16_t p = 10;

		buzzNote(4, o, n, p); // E
		buzzNote(4, o, n, p); // E
		buzzNote(4, o, n, p); // E
		buzzNote(11, o, n * 3, p); // H
	}

}

// ----------------------------------------------------------------------------

void confIRRX_gpio_dec() {
	if (conf_ir_rx_run || !conf_ir_rx_en) {
		return;
	}

	conf_ir_rx_buf = 0;
	conf_ir_rx_idx = 0;

	conf_htim_dec.Instance->ARR = 4800;
	conf_htim_dec.Instance->CNT = 0xFFFF - 2400; // 1.5 * cycle
	HAL_TIM_Base_Start_IT(&conf_htim_dec);
	conf_ir_rx_run = true;
}

void confIRRX_htim_dec() {
	if (!conf_ir_rx_run) {
		return;
	}

	if (conf_ir_rx_idx < 32) {
		conf_ir_rx_buf = conf_ir_rx_buf << 1;
		conf_ir_rx_buf |= !HAL_GPIO_ReadPin(IR_RX_GPIO_Port, IR_RX_Pin);
	} else {
		HAL_TIM_Base_Stop_IT(&conf_htim_dec);
		if (conf_ir_rx_msg_idx < CONF_RX_BUF) {
			conf_ir_rx_msg[conf_ir_rx_msg_idx] = conf_ir_rx_buf;
			conf_ir_rx_msg_idx++;
		}
		conf_ir_rx_run = false;
	}

	conf_ir_rx_idx++;
}

void confIRRX_received(uint32_t msg) {
#if defined(DEBUG)
	printf("C: RX MSG: 0x%08X\n", (unsigned int) msg);
#endif

	uint16_t dec = 0;
	bool dec_err = false;

	for (uint8_t i = 0; i < 16; i++) {
		uint8_t b2 = (msg >> ((15 - i) * 2)) & 0b00000011;
		if (b2 == 0b00000010) {
			dec |= 0x0001 << (15 - i);
		}
		if (b2 == 0b00000001) {
		}
		if (b2 == 0b00000011) {
			dec_err = true;
#if defined(DEBUG)
			printf("C: RX ERR: 11 %d\n", i);
#endif
		}
		if (b2 == 0b00000000) {
			dec_err = true;
#if defined(DEBUG)
			printf("C: RX ERR: 00 %d\n", i);
#endif
		}
	}

	uint8_t id = dec >> 8;
	uint8_t rev = ~dec;
	if (id != rev) {
		dec_err = true;
#if defined(DEBUG)
		printf("C: RX ERR: 0x%02X ~0x%02X\n", id, rev);
#endif
	}

	if (!dec_err) {
#if defined(DEBUG)
		printf("C: RX ID: 0x%02X\n", id);
#endif

		if (id == eeIDGet()) {
			return;
		}

		lcdAdd(lcdChar(0x30));
		conf_clear_dot1 = true;
		confUnlock(id);
	}
}

// ----------------------------------------------------------------------------

uint32_t confIRTX_msg(uint32_t msg, uint8_t id) {
	for (uint8_t i = 0; i < 8; i++) {
		msg = msg << 2;
		if ((id >> (7 - i)) & 0b00000001) {
			msg |= 0b00000010;
		} else {
			msg |= 0b00000001;
		}
	}
	return msg;
}

void confIRTX() {
	uint8_t id = eeIDGet();
	uint32_t msg = 0;
	msg = confIRTX_msg(msg, id);
	msg = confIRTX_msg(msg, ~id);

#if defined(DEBUG)
	printf("C: TX MSG: 0x%08X (ID: 0x%02X)\n", (unsigned int) msg, id);
#endif

	conf_ir_tx_buf = msg;
	conf_ir_tx_idx = 0;

	conf_htim_pwm.Instance->ARR = 209; // 38khz
	conf_htim_pwm.Instance->CCR2 = 209 / 2; // 50% Duty
	conf_htim_pwm.Instance->CNT = 0;
	HAL_TIM_PWM_Start(&conf_htim_pwm, TIM_CHANNEL_2);
	HAL_Delay(1);
	HAL_TIM_PWM_Stop(&conf_htim_pwm, TIM_CHANNEL_2);

	conf_htim_enc.Instance->ARR = 4800;
	conf_htim_enc.Instance->CNT = 0;
	HAL_TIM_Base_Start_IT(&conf_htim_enc);

	conf_ir_tx_run = true;
}

void confIRTX_htim_enc() {
	if (!conf_ir_tx_run) {
		return;
	}

	if (conf_ir_tx_idx > 31) {
		HAL_TIM_PWM_Stop(&conf_htim_pwm, TIM_CHANNEL_2);
		HAL_TIM_Base_Stop_IT(&conf_htim_enc);
		conf_ir_tx_run = false;
		return;
	}

	if (((conf_ir_tx_buf >> (31 - conf_ir_tx_idx)) & 0x01)) {
		conf_htim_pwm.Instance->CNT = 0;
		HAL_TIM_PWM_Start(&conf_htim_pwm, TIM_CHANNEL_2);
	} else {
		HAL_TIM_PWM_Stop(&conf_htim_pwm, TIM_CHANNEL_2);
	}

	conf_ir_tx_idx++;
}

// ----------------------------------------------------------------------------

void confTick() {
	if (conf_clear_dot1) {
		conf_clear_dot1 = false;
		lcdRemove(lcdChar(0x30));
	}

	if (conf_clear_dot2) {
		conf_clear_dot2 = false;
		lcdRemove(lcdChar(0x31));
	}

	while (conf_ir_rx_msg_idx > 0) {
		uint32_t msg = conf_ir_rx_msg[conf_ir_rx_msg_idx - 1];
		conf_ir_rx_msg_idx--;
		confIRRX_received(msg);
	}

	if (conf_tick_display > 0) {
		conf_tick_display--;
	} else {
		conf_tick_display = CONF_CYCLE_S;
		conf_cycle++;
		if (conf_cycle > SHOW_SCORE) {
			conf_cycle = SHOW_UNIQUE;
		}
		confUpdateLCD();
	}

	if (conf_tick_tx > 0) {
		conf_tick_tx--;
	} else {
		conf_tick_tx = (badgeTypeCheck(eeIDGet()) == BADGE_PARTICIPANT ? CONF_CYCLE_TX_S : CONF_CYCLE_TX_OTHER_S);
		lcdAdd(lcdChar(0x31));
		conf_clear_dot2 = true;
		confIRTX();
	}

	if (conf_tick_unlock_bank > 0) {
		conf_tick_unlock_bank--;
	} else {
		conf_tick_unlock_bank = CONF_CYCLE_UNLOCK_S;
		conf_unlock_bank_toggle = !conf_unlock_bank_toggle;
#if defined(DEBUG)
		printf("C: Wiping unlock bank (%d)\n", conf_unlock_bank_toggle);
#endif
		for (uint8_t i = 0; i < 8; i++) {
			if (conf_unlock_bank_toggle) {
				conf_unlock_bank_a[i] = 0;
			} else {
				conf_unlock_bank_b[i] = 0;
			}
		}
	}
}

void confInit(TIM_HandleTypeDef htim_pwm, TIM_HandleTypeDef htim_enc, TIM_HandleTypeDef htim_dec) {
	conf_htim_pwm = htim_pwm;
	conf_htim_enc = htim_enc;
	conf_htim_dec = htim_dec;

	HAL_TIM_Base_Start_IT(&conf_htim_enc);
	HAL_TIM_Base_Stop_IT(&conf_htim_enc);

	HAL_TIM_Base_Start_IT(&conf_htim_dec);
	HAL_TIM_Base_Stop_IT(&conf_htim_dec);
}

void confStart() {
	for (uint8_t i = 0; i < 8; i++) {
		conf_unlock_bank_a[i] = 0;
		conf_unlock_bank_b[i] = 0;
	}

	confUpdateUniqueCount();
	confUpdateHuntCount();

	conf_show = SHOW_CYCLE;
	conf_cycle = SHOW_UNIQUE;

	confUpdateLCD();

	conf_tick_display = CONF_CYCLE_S;
	conf_tick_tx = (badgeTypeCheck(eeIDGet()) == BADGE_PARTICIPANT ? CONF_CYCLE_TX_S : CONF_CYCLE_TX_OTHER_S);
	conf_tick_unlock_bank = CONF_CYCLE_UNLOCK_S;

	conf_ir_rx_en = true;
}

void confStop() {
	conf_ir_rx_en = false;
}
