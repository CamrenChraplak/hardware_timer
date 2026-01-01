/*
	hardware_timer_test_delay.c - delay methods for timer
	Copyright (C) 2025-2026 Camren Chraplak

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "hardware_timer_test_priv.h"

#if UHWT_SUPPORT_ESP32

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void delaySeconds(uint8_t seconds) {
	vTaskDelay(seconds * 1000 / portTICK_PERIOD_MS);
}

#elif defined(ARDUINO) && !(defined(UHWT_OVERRIDE_ARDUINO_TIMER) && (UHWT_SUPPORT_AVR))

#include <Arduino.h>

void delaySeconds(uint8_t seconds) {
	delay(((uint32_t)seconds) * 1000);
}

#else

#if UHWT_SUPPORT_AVR
	#include "../avr/hardware_timer_avr.h"
#endif

/**
 * Interrupt function for delay
 * 
 * @param params parameters passed
 */
void timerDelayCounter(void *params) {
	*(uint8_t*)params += 1;
}

void delaySeconds(uint8_t seconds) {
	uhwt_timer_t timer = UHWT_TIMER_INVALID;
	uint8_t delayCount = 0U;

	uhwt_freq_t freq =
	#ifdef FREQ_MIN_8_COUNTER
		FREQ_MIN_8_COUNTER;
	#else
		1;
	#endif

	if (!setHardTimer(&timer, &freq, &timerDelayCounter, (void*)&delayCount, UHWT_PRIORITY_DEFAULT)) {
		cancelHardTimer(timer);
		return;
	}

	while (delayCount < seconds * freq) {
		__asm__ __volatile__ ("nop");
	}
	cancelHardTimer(timer);
}

#endif