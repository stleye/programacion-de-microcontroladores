/*
 * API_debounce.h
 *
 *  Created on: Sep 17, 2026
 *      Author: sebastian
 */

#ifndef API_INC_API_DEBOUNCE_H_
#define API_INC_API_DEBOUNCE_H_

#include <stdbool.h>

typedef bool bool_t;

void debounceFSM_init();
void debounceFSM_update();
bool_t readKey();

#endif /* API_INC_API_DEBOUNCE_H_ */
