#include "API_delay.h"

/**
 * @brief  Indica si el tiempo del delay ya se cumplio
 * @param  delay: delay a evaluar
 * @retval bool_t
 */
static bool_t hasElapsed(delay_t *delay) {
	return (HAL_GetTick() - delay->startTime) >= delay->duration;
}

/**
 * @brief  Inicializa un delay con la duracion especificada en milisegundos
 * @param  delay_t: estructura a inicializar
 * @param  duration: duración del delay
 * @retval None
 */
void delayInit(delay_t *delay, tick_t duration) {
	if (delay == NULL || duration == 0) {
		return;
	}
	delay->duration = duration;
	delay->running = false;
	delay->startTime = HAL_GetTick();
}

/**
 * @brief  Actualiza el estado del delay y retorna si ha finalizado.
 * @param  delay_t: estructura que contiene información del delay (duración, running, etc)
 * @retval bool_t
 */
bool_t delayRead(delay_t *delay) {
	if (delay == NULL) {
		return false;
	}
	if (delay->running) {
		if (hasElapsed(delay)) {
			delay->running = false;
			return true;
		}
	} else {
		delay->startTime = HAL_GetTick();
		delay->running = true;
	}
	return false;
}

/**
 * @brief  Actualiza la duración del delay.
 * @param  delay_t: delay a modificar
 * @param  duration: nueva duración del delay
 * @retval None
 */
void delayWrite(delay_t *delay, tick_t duration) {
	if (delay == NULL || duration == 0) {
		return;
	}
	delay->duration = duration;
}

/**
 * @brief  Devuelve una copia del valor del campo running de la estructura delay_t
 * @retval bool_t
 */
bool_t delayIsRunning(delay_t *delay) {
	if (delay == NULL) {
		return false;
	}
	return delay->running;
}
