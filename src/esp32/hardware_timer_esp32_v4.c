/*
	hardware_timer_esp32_v4.c - timer configuration for esp idf v4
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

#include "../private/hardware_timer_priv.h"

#if UHWT_SUPPORT_ESP32 && ESP_IDF_VERSION_MAJOR == 4

#include <driver/timer.h>
#include <esp_intr_alloc.h>

/**
 * Gets timer group from timer id
 * 
 * @param timer timer to get
 * 
 * @return timer group
 */
static inline timer_group_t getTimerGroup(uhwt_timer_t timer) {
	#if SOC_TIMER_GROUPS == 1
		return TIMER_GROUP_0;
	#elif SOC_TIMER_GROUP_TIMERS_PER_GROUP == 1
		return timer;
	#else
		return timer / SOC_TIMER_GROUP_TIMERS_PER_GROUP;
	#endif
}

/**
 * Gets timer num in group from tiemr id
 * 
 * @param timer timer to get
 * 
 * @return timer num in group
 */
static inline timer_idx_t getTimerNum(uhwt_timer_t timer) {
	#if SOC_TIMER_GROUP_TIMERS_PER_GROUP == 1
		return TIMER_0;
	#elif SOC_TIMER_GROUPS == 1
		return timer;
	#else
		return timer % SOC_TIMER_GROUP_TIMERS_PER_GROUP;
	#endif
}

/**
 * Scales input priority
 * 
 * @param priority of type uhwt_priority_t
 * 
 * @note function can only run up to priority 'ESP_INTR_FLAG_LEVEL3' since functions are in c
 * 
 * @return priority flag for 'intr_alloc_flags' when calling 'timer_isr_callback_add'
 */
static inline int setPriority(uhwt_priority_t priority) {
	#ifdef PLATFORMIO
		return (1 << (priority / (UINT8_MAX / 3)));
	#else
		return UHWT_PRIORITY_DEFAULT;
	#endif
}

/****************************
 * Platform Functions
****************************/

bool uhwtPlatformInitTimer(uhwt_timer_t timer) {

	timer_config_t config = {
		.divider = 2,
		.counter_dir = true,
		.counter_en = TIMER_PAUSE,
		.alarm_en = TIMER_ALARM_DIS,
		.auto_reload = false,
	};

	timer_group_t timerGroup = getTimerGroup(timer);
	timer_idx_t timerNum = getTimerNum(timer);
	
	ESP_ERROR_CHECK(timer_init(timerGroup, timerNum, &config));
	ESP_ERROR_CHECK(timer_set_counter_value(timerGroup, timerNum, TIMER_COUNT_ZERO));

	return true;
}

bool uhwtPlatformDeconstructTimer(uhwt_timer_t timer) {

	timer_group_t timerGroup = getTimerGroup(timer);
	timer_idx_t timerNum = getTimerNum(timer);

	ESP_ERROR_CHECK(timer_set_counter_value(timerGroup, timerNum, TIMER_COUNT_ZERO));
	ESP_ERROR_CHECK(timer_isr_callback_remove(timerGroup, timerNum));
	ESP_ERROR_CHECK(timer_deinit(timerGroup, timerNum));

	return true;
}

bool uhwtPlatformStopTimer(uhwt_timer_t timer) {

	timer_group_t timerGroup = getTimerGroup(timer);
	timer_idx_t timerNum = getTimerNum(timer);

	ESP_ERROR_CHECK(timer_set_alarm(timerGroup, timerNum, false));
	ESP_ERROR_CHECK(timer_pause(timerGroup, timerNum));

	return true;
}

bool uhwtPlatformStartTimer(uhwt_timer_t timer) {
	
	timer_group_t timerGroup = getTimerGroup(timer);
	timer_idx_t timerNum = getTimerNum(timer);

	// run timer
	ESP_ERROR_CHECK(timer_set_auto_reload(timerGroup, timerNum, true));
	ESP_ERROR_CHECK(timer_set_alarm(timerGroup, timerNum, true));
	ESP_ERROR_CHECK(timer_start(timerGroup, timerNum));

	return true;
}

bool uhwtPlatformSetStats(uhwt_timer_t timer, uhwt_prescalar_t scalar, uhwt_timertick_t timerTicks) {

	timer_group_t timerGroup = getTimerGroup(timer);
	timer_idx_t timerNum = getTimerNum(timer);

	ESP_ERROR_CHECK(timer_set_alarm_value(timerGroup, timerNum, timerTicks));
	ESP_ERROR_CHECK(timer_set_divider(timerGroup, timerNum, scalar));
	return true;
}

uhwt_prescalar_t uhwtPlatformGetPreScalar(uhwt_timer_t timer) {

	timer_config_t config;

	timer_group_t timerGroup = getTimerGroup(timer);
	timer_idx_t timerNum = getTimerNum(timer);

	ESP_ERROR_CHECK(timer_get_config(timerGroup, timerNum, &config));
	return config.divider;
}

uhwt_timertick_t uhwtPlatformGetTimerTicks(uhwt_timer_t timer) {

	uhwt_timertick_t ticks;

	timer_group_t timerGroup = getTimerGroup(timer);
	timer_idx_t timerNum = getTimerNum(timer);

	ESP_ERROR_CHECK(timer_get_alarm_value(timerGroup, timerNum, &ticks));
	return ticks;
}

bool uhwtPlatformSetCallbackParams(uhwt_timer_t timer,
		uhwt_function_ptr_t function, uhwt_params_ptr_t params) {

	timer_group_t timerGroup = getTimerGroup(timer);
	timer_idx_t timerNum = getTimerNum(timer);

	ESP_ERROR_CHECK(timer_isr_callback_add(timerGroup,
			timerNum, uhwtGetCallback(timer), params,
			setPriority(uhwtPriorities[timer])));

	return true;
}

#endif