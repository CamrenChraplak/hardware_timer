/*
	hardware_timer_esp32_v5.c - timer configuration for esp idf v5
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

#include "../private/hardware_timer_priv.h"

#if UHWT_SUPPORT_ESP32 && ESP_IDF_VERSION_MAJOR == 5

#define SCALAR_MAX UINT16_MAX // max value for timer scalar

#include <soc/clk_tree_defs.h>
#include <driver/gptimer_types.h>
#include <hal/timer_types.h>
#include <driver/gptimer.h>

#ifndef APB_CLK_FREQ
#define APB_CLK_FREQ 80000000
#endif

#ifndef GPTIMER_CLK_SRC_DEFAULT
#define GPTIMER_CLK_SRC_DEFAULT SOC_MOD_CLK_APB
#endif

gptimer_handle_t timers[] = {
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
};

uhwt_timertick_t storedTicks[] = {
	#if UHWT_TIMER_COUNT >= 1
		0U,
	#endif
	#if UHWT_TIMER_COUNT >= 2
		0U,
	#endif
	#if UHWT_TIMER_COUNT >= 3
		0U,
	#endif
	#if UHWT_TIMER_COUNT >= 4
		0U,
	#endif
};

uhwt_prescalar_t storedScalar[] = {
	#if UHWT_TIMER_COUNT >= 1
		0U,
	#endif
	#if UHWT_TIMER_COUNT >= 2
		0U,
	#endif
	#if UHWT_TIMER_COUNT >= 3
		0U,
	#endif
	#if UHWT_TIMER_COUNT >= 4
		0U,
	#endif
};

// gptimer requires this as minimum freq, typically ~1222
#define HARD_TIMER_FREQ_MIN ((APB_CLK_FREQ / SCALAR_MAX) + 1)

/**
 * Gets timer based on desired timer
 * 
 * @param timer timer to select
 * 
 * @return pointer to timer selected
 */
static inline gptimer_handle_t* getTimer(uhwt_timer_t timer) {
	return &timers[timer];
}

#define DEFAULT_CONFIG { \
	.clk_src = GPTIMER_CLK_SRC_DEFAULT, \
	.direction = GPTIMER_COUNT_UP, \
	.resolution_hz = HARD_TIMER_FREQ_MIN, \
	.intr_priority = UHWT_PRIORITY_DEFAULT, \
}

gptimer_config_t timerConfigs[UHWT_TIMER_COUNT] = {
	#if UHWT_TIMER_COUNT > 0
		DEFAULT_CONFIG,
	#endif
	#if UHWT_TIMER_COUNT > 1
		DEFAULT_CONFIG,
	#endif
	#if UHWT_TIMER_COUNT > 2
		DEFAULT_CONFIG,
	#endif
	#if UHWT_TIMER_COUNT > 3
		DEFAULT_CONFIG,
	#endif
};

#define DEFAULT_ALARM_CONFIG { \
	.reload_count = 0, \
	.alarm_count = 1, \
	.flags.auto_reload_on_alarm = true, \
}

gptimer_alarm_config_t timerAlarmConfigs[UHWT_TIMER_COUNT] = {
	#if UHWT_TIMER_COUNT > 0
		DEFAULT_ALARM_CONFIG,
	#endif
	#if UHWT_TIMER_COUNT > 1
		DEFAULT_ALARM_CONFIG,
	#endif
	#if UHWT_TIMER_COUNT > 2
		DEFAULT_ALARM_CONFIG,
	#endif
	#if UHWT_TIMER_COUNT > 3
		DEFAULT_ALARM_CONFIG,
	#endif
};

/**
 * Void callback for initialization, of type gptimer_alarm_cb_t
 */
bool uhwtVoidCallback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx) {
	return false;
}

#define DEFAULT_CALLBACK_CONFIG { \
	.on_alarm = &uhwtVoidCallback, \
}

gptimer_event_callbacks_t timerCallbackConfigs[UHWT_TIMER_COUNT] = {
	#if UHWT_TIMER_COUNT > 0
		DEFAULT_CALLBACK_CONFIG,
	#endif
	#if UHWT_TIMER_COUNT > 1
		DEFAULT_CALLBACK_CONFIG,
	#endif
	#if UHWT_TIMER_COUNT > 2
		DEFAULT_CALLBACK_CONFIG,
	#endif
	#if UHWT_TIMER_COUNT > 3
		DEFAULT_CALLBACK_CONFIG,
	#endif
};

/**
 * Reconfigures timer with new stats
 * 
 * @param timer timer to reconfigure
 * 
 * @return if successful
 */
static inline bool uhwtReconfigureTimer(uhwt_timer_t timer) {
	if (!uhwtPlatformDeconstructTimer(timer)) {
		return false;
	}
	return uhwtPlatformInitTimer(timer);
}

/****************************
 * Platform Functions
****************************/

bool uhwtPlatformInitTimer(uhwt_timer_t timer) {

	gptimer_handle_t *timerPtr = getTimer(timer);

	// creates new timer
	ESP_ERROR_CHECK(gptimer_new_timer(&timerConfigs[timer], timerPtr));
	// sets up callback function
	ESP_ERROR_CHECK(gptimer_set_alarm_action(*timerPtr, &timerAlarmConfigs[timer]));
	// callback config
	ESP_ERROR_CHECK(gptimer_register_event_callbacks(*timerPtr,
			&timerCallbackConfigs[timer], NULL));

	return true;
}

bool uhwtPlatformDeconstructTimer(uhwt_timer_t timer) {

	gptimer_handle_t *timerPtr = getTimer(timer);
	ESP_ERROR_CHECK(gptimer_del_timer(*timerPtr));
	timerPtr = NULL;

	return true;
}

bool uhwtPlatformStopTimer(uhwt_timer_t timer) {

	gptimer_handle_t *timerPtr = getTimer(timer);
	ESP_ERROR_CHECK(gptimer_stop(*timerPtr));
	ESP_ERROR_CHECK(gptimer_disable(*timerPtr));

	return true;
}

bool uhwtPlatformStartTimer(uhwt_timer_t timer) {

	gptimer_handle_t *timerPtr = getTimer(timer);
	ESP_ERROR_CHECK(gptimer_enable(*timerPtr));
	ESP_ERROR_CHECK(gptimer_start(*timerPtr));

	return true;
}

bool uhwtPlatformSetStats(uhwt_timer_t timer, uhwt_prescalar_t scalar, uhwt_timertick_t timerTicks) {

	uhwt_freq_t tempFreq = uhwtCalcFreq(scalar, timerTicks);
	uhwt_timertick_t tempTicks = 1;

	// adjusts frequency to be above min
	while (tempFreq < HARD_TIMER_FREQ_MIN) {
		tempFreq *= 2;
		tempTicks *= 2;
	}

	timerConfigs[timer].resolution_hz = tempFreq;
	timerAlarmConfigs[timer].alarm_count = tempTicks;

	storedTicks[timer] = timerTicks;
	storedScalar[timer] = scalar;

	return uhwtReconfigureTimer(timer);
}

uhwt_prescalar_t uhwtPlatformGetPreScalar(uhwt_timer_t timer) {
	return storedScalar[timer];
}

uhwt_timertick_t uhwtPlatformGetTimerTicks(uhwt_timer_t timer) {
	return storedTicks[timer];
}

bool uhwtPlatformSetCallbackParams(uhwt_timer_t timer,
		uhwt_function_ptr_t function, uhwt_params_ptr_t params) {

	timerCallbackConfigs[timer].on_alarm = uhwtGetCallback(timer);
	return uhwtReconfigureTimer(timer);
}

#endif