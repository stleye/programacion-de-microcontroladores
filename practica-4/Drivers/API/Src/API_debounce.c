/*
 * API_debounce.c
 *
 *  Created on: Sep 17, 2026
 *      Author: sebastian
 */

#include "API_debounce.h"
#include "API_delay.h"
#include "stm32f4xx_hal.h"

#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC

static delay_t delay;

static bool_t fallingEdgeFlag;

static const uint8_t delayDuration = 40;

typedef enum {
	BUTTON_UP, BUTTON_FALLING, BUTTON_DOWN, BUTTON_RAISING,
} debounceState_t;

static debounceState_t currentState;

static void buttonPressed() {
	fallingEdgeFlag = true;
}

static void buttonReleased() {
}

void debounceFSM_init() {
	delayInit(&delay, delayDuration);
	currentState = BUTTON_UP;
}

void debounceFSM_update() {
	switch (currentState) {
	case BUTTON_UP:
		if (!HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)) {
			currentState = BUTTON_FALLING;
		}
		break;
	case BUTTON_FALLING:
		if (delayRead(&delay)) {
			if (!HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)) {
				currentState = BUTTON_DOWN;
				buttonPressed();
			} else {
				currentState = BUTTON_UP;
			}
		}
		break;
	case BUTTON_RAISING:
		if (delayRead(&delay)) {
			if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)) {
				currentState = BUTTON_UP;
				buttonReleased();
			} else {
				currentState = BUTTON_DOWN;
			}
		}
		break;
	case BUTTON_DOWN:
		if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)) {
			currentState = BUTTON_RAISING;
		}
		break;
	default:
		debounceFSM_init();
	}
}

bool_t readKey() {
	bool_t pressed = fallingEdgeFlag;
	fallingEdgeFlag = false;
	return pressed;
}
