/*
	hardware_timer_callback.c - hardware timer callback functionality
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

#include "hardware_timer_priv.h"

// functions to execute at end of ISR
uhwt_function_ptr_t hardTimerFunctions[] = {
	#if UHWT_TIMER_COUNT >= 1
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 2
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 3
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 4
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 5
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 6
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 7
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 8
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 9
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 10
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 11
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 12
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 13
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 14
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 15
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 16
		NULL,
	#endif
};

// function parameters to pass
uhwt_params_ptr_t hardTimerParams[] = {
	#if UHWT_TIMER_COUNT >= 1
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 2
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 3
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 4
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 5
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 6
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 7
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 8
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 9
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 10
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 11
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 12
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 13
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 14
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 15
		NULL,
	#endif
	#if UHWT_TIMER_COUNT >= 16
		NULL,
	#endif
};

#ifndef UHWT_NO_CALLBACK_SUPPORT

	// callback functions for linking to ISR
	uhwt_platform_callback_ptr_t hardTimerCallbacks[] = {
		#if UHWT_TIMER_COUNT >= 1
			NULL,
		#endif
		#if UHWT_TIMER_COUNT >= 2
			NULL,
		#endif
		#if UHWT_TIMER_COUNT >= 3
			NULL,
		#endif
		#if UHWT_TIMER_COUNT >= 4
			NULL,
		#endif
		#if UHWT_TIMER_COUNT >= 5
			NULL,
		#endif
		#if UHWT_TIMER_COUNT >= 6
			NULL,
		#endif
		#if UHWT_TIMER_COUNT >= 7
			NULL,
		#endif
		#if UHWT_TIMER_COUNT >= 8
			NULL,
		#endif
		#if UHWT_TIMER_COUNT >= 9
			NULL,
		#endif
		#if UHWT_TIMER_COUNT >= 10
			NULL,
		#endif
		#if UHWT_TIMER_COUNT >= 11
			NULL,
		#endif
		#if UHWT_TIMER_COUNT >= 12
			NULL,
		#endif
		#if UHWT_TIMER_COUNT >= 13
			NULL,
		#endif
		#if UHWT_TIMER_COUNT >= 14
			NULL,
		#endif
		#if UHWT_TIMER_COUNT >= 15
			NULL,
		#endif
		#if UHWT_TIMER_COUNT >= 16
			NULL,
		#endif
	};

	#if UHWT_SUPPORT_ESP32

		typedef bool uhwt_callback_ret_t;
		#define CALLBACK_RETURN() return false

		#if ESP_IDF_VERSION_MAJOR == 4
			#define CALL_PARAMS void *params
		#elif ESP_IDF_VERSION_MAJOR == 5
			#define CALL_PARAMS gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *params
		#endif

	#elif UHWT_SUPPORT_PICO

		typedef bool uhwt_callback_ret_t;
		#define CALLBACK_RETURN() return true
		#define CALL_PARAMS repeating_timer_t *rt

	#endif

	/**
	 * Creates callback functions for each timer
	 * 
	 * @param num timer number to set
	 */
	#define TIMER_CALLBACK_PROTOTYPE(num) \
		static uhwt_callback_ret_t HARD_TIMER_CONCATENATE(timerCallback, num)(CALL_PARAMS) { \
			((void(*)())hardTimerFunctions[num])(hardTimerParams[num]); \
			CALLBACK_RETURN(); \
		}

	#if UHWT_TIMER_COUNT >= 1
		TIMER_CALLBACK_PROTOTYPE(0)
	#endif
	#if UHWT_TIMER_COUNT >= 2
		TIMER_CALLBACK_PROTOTYPE(1)
	#endif
	#if UHWT_TIMER_COUNT >= 3
		TIMER_CALLBACK_PROTOTYPE(2)
	#endif
	#if UHWT_TIMER_COUNT >= 4
		TIMER_CALLBACK_PROTOTYPE(3)
	#endif
	#if UHWT_TIMER_COUNT >= 5
		TIMER_CALLBACK_PROTOTYPE(4)
	#endif
	#if UHWT_TIMER_COUNT >= 6
		TIMER_CALLBACK_PROTOTYPE(5)
	#endif
	#if UHWT_TIMER_COUNT >= 7
		TIMER_CALLBACK_PROTOTYPE(6)
	#endif
	#if UHWT_TIMER_COUNT >= 8
		TIMER_CALLBACK_PROTOTYPE(7)
	#endif
	#if UHWT_TIMER_COUNT >= 9
		TIMER_CALLBACK_PROTOTYPE(8)
	#endif
	#if UHWT_TIMER_COUNT >= 10
		TIMER_CALLBACK_PROTOTYPE(9)
	#endif
	#if UHWT_TIMER_COUNT >= 11
		TIMER_CALLBACK_PROTOTYPE(10)
	#endif
	#if UHWT_TIMER_COUNT >= 12
		TIMER_CALLBACK_PROTOTYPE(11)
	#endif
	#if UHWT_TIMER_COUNT >= 13
		TIMER_CALLBACK_PROTOTYPE(12)
	#endif
	#if UHWT_TIMER_COUNT >= 14
		TIMER_CALLBACK_PROTOTYPE(13)
	#endif
	#if UHWT_TIMER_COUNT >= 15
		TIMER_CALLBACK_PROTOTYPE(14)
	#endif
	#if UHWT_TIMER_COUNT >= 16
		TIMER_CALLBACK_PROTOTYPE(15)
	#endif

#endif

/**
 * Simplifies callback test cases
 * 
 * @param num timer number
 */
#define CALLBACK_TEST_CASE(num) \
	case(HARD_TIMER_CONCATENATE(UHWT_TIMER, num)): \
		hardTimerCallbacks[num] = HARD_TIMER_CONCATENATE(timerCallback, num); \
	break;

bool uhwtSetCallbackParams(uhwt_timer_t timer,
		uhwt_function_ptr_t function, uhwt_params_ptr_t params) {
	if (!uhwtValidTimer(timer) || !uhwtTimerInitialized(timer) || uhwtTimerStarted(timer) || function == NULL) {
		return false;
	}

	hardTimerFunctions[timer] = function;
	hardTimerParams[timer] = params;

	#ifndef UHWT_NO_CALLBACK_SUPPORT

		switch(timer) {
			#if UHWT_TIMER_COUNT >= 1
				CALLBACK_TEST_CASE(0)
			#endif
			#if UHWT_TIMER_COUNT >= 2
				CALLBACK_TEST_CASE(1)
			#endif
			#if UHWT_TIMER_COUNT >= 3
				CALLBACK_TEST_CASE(2)
			#endif
			#if UHWT_TIMER_COUNT >= 4
				CALLBACK_TEST_CASE(3)
			#endif
			#if UHWT_TIMER_COUNT >= 5
				CALLBACK_TEST_CASE(4)
			#endif
			#if UHWT_TIMER_COUNT >= 6
				CALLBACK_TEST_CASE(5)
			#endif
			#if UHWT_TIMER_COUNT >= 7
				CALLBACK_TEST_CASE(6)
			#endif
			#if UHWT_TIMER_COUNT >= 8
				CALLBACK_TEST_CASE(7)
			#endif
			#if UHWT_TIMER_COUNT >= 9
				CALLBACK_TEST_CASE(8)
			#endif
			#if UHWT_TIMER_COUNT >= 10
				CALLBACK_TEST_CASE(9)
			#endif
			#if UHWT_TIMER_COUNT >= 11
				CALLBACK_TEST_CASE(10)
			#endif
			#if UHWT_TIMER_COUNT >= 12
				CALLBACK_TEST_CASE(11)
			#endif
			#if UHWT_TIMER_COUNT >= 13
				CALLBACK_TEST_CASE(12)
			#endif
			#if UHWT_TIMER_COUNT >= 14
				CALLBACK_TEST_CASE(13)
			#endif
			#if UHWT_TIMER_COUNT >= 15
				CALLBACK_TEST_CASE(14)
			#endif
			#if UHWT_TIMER_COUNT >= 16
				CALLBACK_TEST_CASE(15)
			#endif
			default:
			break;
		}

	#endif

	if (!uhwtPlatformSetCallbackParams(timer, function, params)) {
		hardTimerFunctions[timer] = NULL;
		hardTimerParams[timer] = NULL;
		hardTimerCallbacks[timer] = NULL;
		return false;
	}

	return true;
}

uhwt_platform_callback_ptr_t uhwtGetCallback(uhwt_timer_t timer) {
	#ifndef UHWT_NO_CALLBACK_SUPPORT
		if (timer == UHWT_TIMER_INVALID) {
			return NULL;
		}
		return hardTimerCallbacks[timer];
	#else
		return NULL;
	#endif
}

bool uhwtPlatformSetCallbackParams(uhwt_timer_t timer,
		uhwt_function_ptr_t function, uhwt_params_ptr_t params) __attribute__((weak));
bool uhwtPlatformSetCallbackParams(uhwt_timer_t timer,
		uhwt_function_ptr_t function, uhwt_params_ptr_t params) {
	return true;
}