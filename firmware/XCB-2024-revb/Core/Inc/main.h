/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdbool.h>

#if defined(DEBUG)
#include "stdio.h"
#endif

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

typedef enum {
	BUTTON_TL, BUTTON_TR, BUTTON_BR, BUTTON_BL
} button_t;

typedef enum {
	BADGE_PARTICIPANT, BADGE_TREASURE, BADGE_OTHER
} badge_t;

#define FIRMWARE 0x01

#define BUTTON_PRESS_DELAY 150

// 0x00
#define ID_UNLOCK_START 0x01
#define ID_UNLOCK_END 0xDF
#define ID_TREASURE_START 0xE0
#define ID_TREASURE_END 0xEE
// 0xEF
#define ID_CMD_WIPE 0xF0
// 0xF1
// 0xF2
// 0xF3
// 0xF4
// 0xF5
// 0xF6
// 0xF7
#define ID_CMD_TEST 0xF8
// 0xF9
// 0xFA
// 0xFB
// 0xFC
// 0xFD
// 0xFE
// 0xFF

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

void buttonClear();

badge_t badgeTypeCheck(uint8_t id);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SENSOR_VCC_Pin GPIO_PIN_1
#define SENSOR_VCC_GPIO_Port GPIOA
#define BUZZ_Pin GPIO_PIN_4
#define BUZZ_GPIO_Port GPIOA
#define LCD_CS_Pin GPIO_PIN_5
#define LCD_CS_GPIO_Port GPIOA
#define LCD_WRITE_Pin GPIO_PIN_6
#define LCD_WRITE_GPIO_Port GPIOA
#define LCD_DATA_Pin GPIO_PIN_7
#define LCD_DATA_GPIO_Port GPIOA
#define MODE_SENSOR_Pin GPIO_PIN_0
#define MODE_SENSOR_GPIO_Port GPIOB
#define MODE_SENSOR_EXTI_IRQn EXTI0_1_IRQn
#define MODE_PLAY_Pin GPIO_PIN_1
#define MODE_PLAY_GPIO_Port GPIOB
#define MODE_PLAY_EXTI_IRQn EXTI0_1_IRQn
#define LCD_IRQ_Pin GPIO_PIN_8
#define LCD_IRQ_GPIO_Port GPIOA
#define LCD_IRQ_EXTI_IRQn EXTI4_15_IRQn
#define IR_RX_Pin GPIO_PIN_11
#define IR_RX_GPIO_Port GPIOA
#define IR_RX_EXTI_IRQn EXTI4_15_IRQn
#define MODE_CONF_Pin GPIO_PIN_12
#define MODE_CONF_GPIO_Port GPIOA
#define MODE_CONF_EXTI_IRQn EXTI4_15_IRQn
#define BTN_TL_Pin GPIO_PIN_3
#define BTN_TL_GPIO_Port GPIOB
#define BTN_TL_EXTI_IRQn EXTI2_3_IRQn
#define BTN_BL_Pin GPIO_PIN_4
#define BTN_BL_GPIO_Port GPIOB
#define BTN_BL_EXTI_IRQn EXTI4_15_IRQn
#define IR_TX_Pin GPIO_PIN_5
#define IR_TX_GPIO_Port GPIOB
#define BTN_BR_Pin GPIO_PIN_6
#define BTN_BR_GPIO_Port GPIOB
#define BTN_BR_EXTI_IRQn EXTI4_15_IRQn
#define BTN_TR_Pin GPIO_PIN_7
#define BTN_TR_GPIO_Port GPIOB
#define BTN_TR_EXTI_IRQn EXTI4_15_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
