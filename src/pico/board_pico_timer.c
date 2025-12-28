/*
	board_pico_timer.c - timer configuration for all Raspberry Pi Picos
	Copyright (C) 2025 Camren Chraplak

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

/**
 * timerTicks: time in ms or us
 * scalar: SCALAR_MS (millis) or SCALAR_US (micros)
 */

#include "../private/hardware_timer_priv.h"

#if UHWT_SUPPORT_PICO

#include <pico/time.h>

// hardware timers
struct repeating_timer timers[UHWT_TIMER_COUNT];
uhwt_timertick_t picoTicks[UHWT_TIMER_COUNT];

/**
 * Gets timer based on desired timer
 * 
 * @param timer timer to select
 * 
 * @return pointer to timer selected
 */
static inline struct repeating_timer* getTimer(uhwt_timer_t timer) {
	return &timers[timer];
}

#define PICO_SDK_TIMER_MAX 1000000 // max frequency in Hz

/****************************
 * Universal Hardware Timer Functions
****************************/

uhwt_freq_t uhwtPlatformCalcFreq(uhwt_prescalar_t scalar, uhwt_timertick_t ticks) {
	return PICO_SDK_TIMER_MAX / ticks;
}

uhwt_timertick_t uhwtCalcTicks(uhwt_freq_t targetFreq, uhwt_prescalar_t scalar) {
	return PICO_SDK_TIMER_MAX / targetFreq;
}

uhwt_prescalar_t uhwtCalcScalar(uhwt_freq_t targetFreq, uhwt_timertick_t ticks) {
	return 1;
}

/****************************
 * Platform Functions
****************************/

bool uhwtPlatformInitTimer(uhwt_timer_t timer) {
	return true;
}

bool uhwtPlatformDeconstructTimer(uhwt_timer_t timer) {
	return true;
}

bool uhwtPlatformStopTimer(uhwt_timer_t timer) {
	struct repeating_timer* timerPtr = getTimer(timer);
	if (!cancel_repeating_timer(timerPtr)) {
		return false;
	}
	return true;
}

bool uhwtPlatformStartTimer(uhwt_timer_t timer) {
	struct repeating_timer* timerPtr = getTimer(timer);
	if (!add_repeating_timer_us(-picoTicks[timer], uhwtGetCallback(timer), NULL, timerPtr)) {
		return false;
	}
	return true;
}

bool uhwtPlatformSetStats(uhwt_timer_t timer, uhwt_prescalar_t scalar, uhwt_timertick_t timerTicks) {
	picoTicks[timer] = timerTicks;
	return true;
}

bool uhwtPlatformEqualFreq(uhwt_freq_t targetFreq, uhwt_prescalar_t scalar, uhwt_timertick_t ticks) {
	if (PICO_SDK_TIMER_MAX % ticks != 0) {
		return false;
	}
	return true;
}

uhwt_prescalar_t uhwtGetNextPreScalar(uhwt_prescalar_t prevScalar) {
	// u64 bit max
	// always constant

	// 0: start of request
	if (!prevScalar) {
		return 1;
	}
	// end of request
	return 0;
}

uhwt_prescalar_t uhwtPlatformGetPreScalar(uhwt_timer_t timer) {
	return 1;
}

uhwt_timertick_t uhwtPlatformGetTimerTicks(uhwt_timer_t timer) {
	return picoTicks[timer];
}

bool uhwtPlatformValidPreScalar(uhwt_prescalar_t scalar) {
	if (scalar == 1) {
		return true;
	}
	return false;
}

#endif