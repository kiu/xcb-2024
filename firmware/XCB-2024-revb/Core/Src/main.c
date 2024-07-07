/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "dev_buzz.h"
#include "dev_eeprom.h"
#include "dev_lcd.h"

#include "mode_conf.h"
#include "mode_play.h"
#include "mode_sensor.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum {
	MODE_INIT, MODE_CONF, MODE_PLAY, MODE_SENSOR
} mode_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

static volatile bool tick = false;
static volatile uint16_t irq_state = 0;

static volatile bool mode_switch = true;
static mode_t mode = MODE_INIT;
static mode_t modeNext = MODE_CONF;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim14;
TIM_HandleTypeDef htim16;
TIM_HandleTypeDef htim17;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM14_Init(void);
static void MX_TIM16_Init(void);
static void MX_TIM17_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

#if defined(DEBUG)
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#endif

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#if defined(DEBUG)
PUTCHAR_PROTOTYPE {
	HAL_UART_Transmit(&huart1, (uint8_t*) &ch, 1, 0xFFFF);
	return ch;
}
#endif

void modeCheck() {
	if (!mode_switch) {
		return;
	}
	HAL_Delay(300);

	uint8_t next = 0xFF;
	uint8_t match = 0;
	if (!HAL_GPIO_ReadPin(MODE_CONF_GPIO_Port, MODE_CONF_Pin)) {
		next = MODE_CONF;
		match++;
	}

	if (!HAL_GPIO_ReadPin(MODE_PLAY_GPIO_Port, MODE_PLAY_Pin)) {
		next = MODE_PLAY;
		match++;
	}
	if (!HAL_GPIO_ReadPin(MODE_SENSOR_GPIO_Port, MODE_SENSOR_Pin)) {
		next = MODE_SENSOR;
		match++;
	}

	if (match == 1) {
		modeNext = next;
		mode_switch = false;
	}
}

void modeChange() {
#if defined(DEBUG)
	printf("M: Mode change: %d --> %d\n", mode, modeNext);
#endif

#if defined(DEBUG)
	printf("M: Mode stop: %d\n", mode);
#endif

	if (mode == MODE_CONF) {
		confStop();
	}
	if (mode == MODE_PLAY) {
		playStop();
	}
	if (mode == MODE_SENSOR) {
		sensorStop();
	}

	lcdShow(0);
	mode = modeNext;

#if defined(DEBUG)
	printf("M: Mode start: %d\n", mode);
#endif

	if (mode == MODE_CONF) {
		lcdShow(lcdChar(0x35)); // CON
		HAL_Delay(1000);
		confStart();
	}
	if (mode == MODE_PLAY) {
		lcdShow(lcdChar(0x36)); // PLY
		HAL_Delay(1000);
		playStart();
	}
	if (mode == MODE_SENSOR) {
		lcdShow(lcdChar(0x37)); // SEN
		HAL_Delay(1000);
		sensorStart();
	}
}

void modeTick() {
//#if defined(DEBUG)
//	printf("M: Mode tick: %d\n", mode);
//#endif

	if (mode == MODE_CONF) {
		confTick();
	}
	if (mode == MODE_PLAY) {
		playTick();
	}
	if (mode == MODE_SENSOR) {
		sensorTick();
	}
}

void buttonClear() {
	irq_state &= ~BTN_TL_Pin;
	irq_state &= ~BTN_TR_Pin;
	irq_state &= ~BTN_BR_Pin;
	irq_state &= ~BTN_BL_Pin;
}

void buttonCheck() {
	uint8_t btn = 0;

	if (irq_state & BTN_TL_Pin) {
		irq_state &= ~BTN_TL_Pin;
		btn |= (1 << 0);
	}
	if (irq_state & BTN_TR_Pin) {
		irq_state &= ~BTN_TR_Pin;
		btn |= (1 << 1);
	}
	if (irq_state & BTN_BR_Pin) {
		irq_state &= ~BTN_BR_Pin;
		btn |= (1 << 2);
	}
	if (irq_state & BTN_BL_Pin) {
		irq_state &= ~BTN_BL_Pin;
		btn |= (1 << 3);
	}

	while (btn > 0) {
		for (uint8_t i = 0; i < 4; i++) {

			if (btn & (1 << i)) {
				btn &= ~(1 << i);
#if defined(DEBUG)
				printf("M: Button: %d\n", i);
#endif

				if (mode == MODE_CONF) {
					confButton(i);
				}
				if (mode == MODE_PLAY) {
					playButton(i);
				}
				if (mode == MODE_SENSOR) {
					sensorButton(i);
				}
			}
		}
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t pin) {
	irq_state |= pin;

	if ((pin & MODE_CONF_Pin) | (pin & MODE_PLAY_Pin) | (pin & MODE_SENSOR_Pin)) {
		mode_switch = true;
	}

	if (pin & LCD_IRQ_Pin) {
		tick = true;
	}

	if (pin & IR_RX_Pin) {
		if (mode == MODE_CONF) {
			confIRRX_gpio_dec();
		}
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim == &htim16) {
		confIRTX_htim_enc();
	}

	if (htim == &htim17) {
		confIRRX_htim_dec();
	}
}

badge_t badgeTypeCheck(uint8_t id) {
	if (id >= ID_UNLOCK_START && id <= ID_UNLOCK_END) {
		return BADGE_PARTICIPANT;
	}
	if (id >= ID_TREASURE_START && id <= ID_TREASURE_END) {
		return BADGE_TREASURE;
	}
	return BADGE_OTHER;
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_I2C1_Init();
	MX_TIM3_Init();
	MX_TIM14_Init();
	MX_TIM16_Init();
	MX_TIM17_Init();
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */

#if defined(DEBUG)
	printf("\n");
	printf("https://github.com/kiu/xcb-2024\n");
	printf("M: Firmware: 0x%02X\n", FIRMWARE);
#endif

	eeInit();

	badge_t badge = badgeTypeCheck(eeIDGet());
#if defined(DEBUG)
	printf("M: Badge Type: %d\n", badge);
#endif

	lcdInit();

	uint32_t lcd = 0;

	lcd = 0;
	lcd |= lcdChar(0x2F); // Fxx
	lcd |= lcdDigits_2(16, FIRMWARE, 0, 0, 0);
	lcdShow(lcd);
	buzzInit(htim14);
	HAL_Delay(1200);

	lcd = 0;
	lcd |= lcdChar(0x39); // Uxx
	lcd |= lcdDigits_2(16, eeIDGet(), 0, 0, 0);
	lcdShow(lcd);
	HAL_Delay(1200);

	sensorInit(hi2c1);

	confInit(htim3, htim16, htim17);

	if (badge != BADGE_PARTICIPANT) {
		modeNext = MODE_CONF;
		modeChange();
	}

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		if (badge == BADGE_PARTICIPANT) {
			buttonCheck();
			modeCheck();
			if (mode != modeNext) {
				modeChange();
			}
		}

		if (tick) {
			tick = false;
			modeTick();
		}

#if !defined(DEBUG)
		if (confAllowSleep()) {
			HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
		}
#endif

	}

	return 0;

	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_I2C1;
	PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
	PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x2000090E;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void) {

	/* USER CODE BEGIN TIM3_Init 0 */

	/* USER CODE END TIM3_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };

	/* USER CODE BEGIN TIM3_Init 1 */

	/* USER CODE END TIM3_Init 1 */
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 0;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 209;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM3_Init 2 */

	/* USER CODE END TIM3_Init 2 */
	HAL_TIM_MspPostInit(&htim3);

}

/**
 * @brief TIM14 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM14_Init(void) {

	/* USER CODE BEGIN TIM14_Init 0 */

	/* USER CODE END TIM14_Init 0 */

	TIM_OC_InitTypeDef sConfigOC = { 0 };

	/* USER CODE BEGIN TIM14_Init 1 */

	/* USER CODE END TIM14_Init 1 */
	htim14.Instance = TIM14;
	htim14.Init.Prescaler = 0;
	htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim14.Init.Period = 65535;
	htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_Base_Init(&htim14) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim14) != HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim14, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM14_Init 2 */

	/* USER CODE END TIM14_Init 2 */
	HAL_TIM_MspPostInit(&htim14);

}

/**
 * @brief TIM16 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM16_Init(void) {

	/* USER CODE BEGIN TIM16_Init 0 */

	/* USER CODE END TIM16_Init 0 */

	/* USER CODE BEGIN TIM16_Init 1 */

	/* USER CODE END TIM16_Init 1 */
	htim16.Instance = TIM16;
	htim16.Init.Prescaler = 0;
	htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim16.Init.Period = 4800;
	htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim16.Init.RepetitionCounter = 0;
	htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_Base_Init(&htim16) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM16_Init 2 */

	/* USER CODE END TIM16_Init 2 */

}

/**
 * @brief TIM17 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM17_Init(void) {

	/* USER CODE BEGIN TIM17_Init 0 */

	/* USER CODE END TIM17_Init 0 */

	/* USER CODE BEGIN TIM17_Init 1 */

	/* USER CODE END TIM17_Init 1 */
	htim17.Instance = TIM17;
	htim17.Init.Prescaler = 0;
	htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim17.Init.Period = 4800;
	htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim17.Init.RepetitionCounter = 0;
	htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_Base_Init(&htim17) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM17_Init 2 */

	/* USER CODE END TIM17_Init 2 */

}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 38400;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_SWAP_INIT;
	huart1.AdvancedInit.Swap = UART_ADVFEATURE_SWAP_ENABLE;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, SENSOR_VCC_Pin | LCD_CS_Pin | LCD_WRITE_Pin | LCD_DATA_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : PF0 PF1 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	/*Configure GPIO pins : PA0 PA3 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : SENSOR_VCC_Pin LCD_CS_Pin LCD_WRITE_Pin LCD_DATA_Pin */
	GPIO_InitStruct.Pin = SENSOR_VCC_Pin | LCD_CS_Pin | LCD_WRITE_Pin | LCD_DATA_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : MODE_SENSOR_Pin MODE_PLAY_Pin BTN_TL_Pin BTN_BL_Pin
	 BTN_BR_Pin BTN_TR_Pin */
	GPIO_InitStruct.Pin = MODE_SENSOR_Pin | MODE_PLAY_Pin | BTN_TL_Pin | BTN_BL_Pin | BTN_BR_Pin | BTN_TR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : LCD_IRQ_Pin MODE_CONF_Pin */
	GPIO_InitStruct.Pin = LCD_IRQ_Pin | MODE_CONF_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : IR_RX_Pin */
	GPIO_InitStruct.Pin = IR_RX_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(IR_RX_GPIO_Port, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

	HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);

	HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
