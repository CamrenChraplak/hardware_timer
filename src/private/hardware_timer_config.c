/*
	hardware_timer_config.c - hardware timer config functionality
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

#include "hardware_timer_priv.h"

#if !UHWT_SUPPORT
	#warning "universal_hardware_timer.h library not supported!"
#endif

#ifdef UHWT_PRIORITY_SUPPORT
	uhwt_priority_t uhwtPriorities[UHWT_TIMER_COUNT]; // priorities of timer execution
#else
	void uhwtSetPriority(uhwt_timer_t timer, uhwt_priority_t priority) {}
#endif

bool uhwtStopTimer(uhwt_timer_t timer) {
	if (!uhwtTimerStarted(timer)) {
		return false;
	}
	if (uhwtPlatformStopTimer(timer)) {
		uhwtSetTimerStopped(timer);
		return true;
	}
	return false;
}

bool uhwtStartTimer(uhwt_timer_t timer) {
	if (!uhwtValidTimer(timer) || uhwtTimerStarted(timer) || !uhwtTimerInitialized(timer)) {
		return false;
	}
	if (uhwtPlatformStartTimer(timer)) {
		uhwtSetTimerStarted(timer);
		return true;
	}
	return false;
}

bool uhwtInitTimer(uhwt_timer_t timer) {
	if (uhwtTimerInitialized(timer) || !uhwtValidTimer(timer)) {
		return false;
	}
	if (uhwtPlatformInitTimer(timer)) {
		uhwtSetTimerInitialized(timer);
		return true;
	}
	return false;
}

bool uhwtDeconstructTimer(uhwt_timer_t timer) {
	if (!uhwtTimerInitialized(timer) || uhwtTimerStarted(timer)) {
		return false;
	}
	if (uhwtPlatformDeconstructTimer(timer)) {
		uhwtSetTimerDeconstructed(timer);
		return true;
	}
	return false;
}

uhwt_timer_t uwhtGetNextTimer() {
	for (uhwt_timer_t i = 0; i < UHWT_TIMER_COUNT; i++) {
		if (!uhwtTimerInitialized(i) && !uhwtTimerClaimed(i)) {
			return i;
		}
	}
	return UHWT_TIMER_INVALID;
}

uhwt_timer_t uwhtGetNextTimerStats(uhwt_claim_s claimArgs) {

	uhwt_timer_t timer = uwhtPlatformGetNextTimerStats(claimArgs);

	if (!uhwtValidTimer(timer)) {
		return uwhtGetNextTimer();
	}

	return timer;
}

bool uhwtEqualFreq(uhwt_freq_t targetFreq, uhwt_prescalar_t scalar, uhwt_timertick_t ticks) {
	uhwt_freq_t calcFreq = uhwtCalcFreq(scalar, ticks);
	if (!uhwtValidFrequency(targetFreq) || !uhwtValidFrequency(calcFreq)) {
		return false;
	}
	if (calcFreq == targetFreq) {
		return uhwtPlatformEqualFreq(targetFreq, scalar, ticks);
	}
	return false;
}

/**
 * Sets up hardware timer
 * 
 * @param timer pointer to timer to store
 * @param targetFreq frequency to achieve
 * @param function pointer to function to call back
 * @param params parameters to pass to callback function
 * @param simpleStats if using simple or not get stats
 * 
 * @return if successful
 */
bool uhwtSetup(uhwt_timer_t *timer, uhwt_freq_t targetFreq, uhwt_function_ptr_t function, uhwt_params_ptr_t params, bool simpleStats) {
	if (function == NULL || timer == NULL) {
		return false;
	}
	if (!uhwtValidFrequency(targetFreq)) {
		return false;
	}

	uhwt_prescalar_t scalar;
	uhwt_timertick_t timerTicks;

	if (simpleStats) {
		if (!uhwtGetStats(timer, targetFreq, &scalar, &timerTicks)) {
			return false;
		}
	}
	else {
		if (!uhwtGetClosestStats(timer, targetFreq, &scalar, &timerTicks)) {
			return false;
		}
	}

	if (!uhwtValidTimerPreScalar(*timer, scalar) || !uhwtValidTimerTicks(*timer, timerTicks)) {
		return false;
	}
	
	if (!uhwtTimerStarted(*timer)) {
		if (!uhwtInitTimer(*timer)) {
			return false;
		}
		if (!uhwtSetCallbackParams(*timer, function, params)) {
			return false;
		}
		if (!uhwtSetStats(*timer, scalar, timerTicks)) {
			return false;
		}
		return true;
	}

	return false;
}

bool uhwtSetupTimer(uhwt_timer_t *timer, uhwt_freq_t targetFreq, uhwt_function_ptr_t function, uhwt_params_ptr_t params) {
	return uhwtSetup(timer, targetFreq, function, params, true);
}

bool uhwtSetupComplexTimer(uhwt_timer_t *timer, uhwt_freq_t targetFreq, uhwt_function_ptr_t function, uhwt_params_ptr_t params, uhwt_priority_t priority) {
	if (uhwtSetup(timer, targetFreq, function, params, false)) {
		uhwtSetPriority(*timer, priority);
		return true;
	}
	return false;
}

// TODO: remove

bool setHardTimer(uhwt_timer_t *timer, uhwt_freq_t *freq, uhwt_function_ptr_t function, uhwt_params_ptr_t params, uhwt_priority_t priority) {

	if (freq == NULL) {
		return false;
	}
	if (!uhwtSetupTimer(timer, *freq, function, params)) {
	//if (!uhwtSetupComplexTimer(timer, *freq, function, params, priority)) {
		return false;
	}
	*freq = uhwtCalcFreq(uhwtGetPreScalar(*timer), uhwtGetTimerTicks(*timer));
	return uhwtStartTimer(*timer);
}

// TODO: remove

bool cancelHardTimer(uhwt_timer_t timer) {
	if (uhwtStopTimer(timer)) {
		return uhwtDeconstructTimer(timer);
	}
	return false;
}