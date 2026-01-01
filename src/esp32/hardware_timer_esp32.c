/*
	hardware_timer_esp32.c - timer configuration for Espressif ESP32
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
 * APB_CLK = 80,000,000Hz
 * 
 * freq = desired frequency (Hz)
 * 
 * freq = APB_CLK / (scalar * timerTicks)
 * 
 * 64-bit counter
 * 16-bit scalar
 */

#include "../private/hardware_timer_priv.h"

#if UHWT_SUPPORT_ESP32

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>

#define TIMER_COUNT_ZERO 0U // value for setting timer tick count to 0
#define SCALAR_MAX UINT16_MAX // max value for timer scalar

//#undef ESP_IDF_VERSION_MAJOR
//#define ESP_IDF_VERSION_MAJOR 5

#if ESP_IDF_VERSION_MAJOR == 4

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

#elif ESP_IDF_VERSION_MAJOR == 5

#include <soc/clk_tree_defs.h>
#include <driver/gptimer_types.h>
#include <hal/timer_types.h>
#include <driver/gptimer.h>

/*#ifndef APB_CLK_FREQ
#define APB_CLK_FREQ 80000000
#endif

#ifndef GPTIMER_CLK_SRC_DEFAULT
#define GPTIMER_CLK_SRC_DEFAULT SOC_MOD_CLK_APB
#endif*/

#if UHWT_TIMER_COUNT >= 1
	gptimer_handle_t handler0 = NULL;
#endif
#if UHWT_TIMER_COUNT >= 2
	gptimer_handle_t handler1 = NULL;
#endif
#if UHWT_TIMER_COUNT >= 3
	gptimer_handle_t handler2 = NULL;
#endif
#if UHWT_TIMER_COUNT >= 4
	gptimer_handle_t handler3 = NULL;
#endif

gptimer_handle_t *timers[] = {
	#if UHWT_TIMER_COUNT >= 1
		&handler0,
	#endif
	#if UHWT_TIMER_COUNT >= 2
		&handler1,
	#endif
	#if UHWT_TIMER_COUNT >= 3
		&handler2,
	#endif
	#if UHWT_TIMER_COUNT >= 4
		&handler3,
	#endif
};

typedef gptimer_handle_t** timer_ptr_t;

// gptimer requires this as minimum freq
#define HARD_TIMER_FREQ_MIN ((APB_CLK_FREQ / SCALAR_MAX) + 1)

/**
 * Gets timer based on desired timer
 * 
 * @param timer timer to select
 * 
 * @return pointer to timer selected
 */
timer_ptr_t getTimer(uhwt_timer_t timer) {

	if (!uhwtValidTimer(timer)) {
		return NULL;
	}
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

#endif

/**
 * Scales input priority
 * 
 * @param priority of type uhwt_priority_t
 * 
 * @note function can only run up to priority 'ESP_INTR_FLAG_LEVEL3' since functions are in c
 * 
 * @return priority flag for 'intr_alloc_flags' when calling 'timer_isr_callback_add'
 */
int setPriority(uhwt_priority_t priority) {
	#ifdef PLATFORMIO
		// use input priority for PlatformIO
		int adjustment = priority / (UINT8_MAX / 3);
		#if ESP_IDF_VERSION_MAJOR == 4
			return (1 << adjustment);
		#elif ESP_IDF_VERSION_MAJOR == 5
			return adjustment;
		#else
			return UHWT_PRIORITY_DEFAULT;
		#endif
	#else
		// use default priority with esp-idf
		return UHWT_PRIORITY_DEFAULT;
	#endif
}

/****************************
 * Universal Hardware Timer Functions
****************************/

uhwt_freq_t uhwtPlatformCalcFreq(uhwt_prescalar_t scalar, uhwt_timertick_t ticks) {
	return APB_CLK_FREQ / (scalar * ticks);
}

uhwt_timertick_t uhwtCalcTicks(uhwt_freq_t targetFreq, uhwt_prescalar_t scalar) {
	return APB_CLK_FREQ / (targetFreq * scalar);
}

uhwt_prescalar_t uhwtCalcScalar(uhwt_freq_t targetFreq, uhwt_timertick_t ticks) {
	return APB_CLK_FREQ / (targetFreq * ticks);
}

void uhwtSetPriority(uhwt_timer_t timer, uhwt_priority_t priority) {
	if (uhwtValidTimer(timer)) {
		uhwtPriorities[timer] = priority;
	}
}

/****************************
 * Platform Functions
****************************/

bool uhwtPlatformInitTimer(uhwt_timer_t timer) {

	#if ESP_IDF_VERSION_MAJOR == 4

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
	
	#elif ESP_IDF_VERSION_MAJOR == 5

		timer_ptr_t timerPtr = getTimer(timer);

		// creates new timer
		ESP_ERROR_CHECK(gptimer_new_timer(&timerConfigs[timer], *timerPtr));
		// sets up callback function
		ESP_ERROR_CHECK(gptimer_set_alarm_action(**timerPtr, &timerAlarmConfigs[timer]));
		// callback config
		ESP_ERROR_CHECK(gptimer_register_event_callbacks(**timerPtr,
				&timerCallbackConfigs[timer], NULL));

	#endif

	return true;
}

bool uhwtPlatformDeconstructTimer(uhwt_timer_t timer) {

	#if ESP_IDF_VERSION_MAJOR == 4

		timer_group_t timerGroup = getTimerGroup(timer);
		timer_idx_t timerNum = getTimerNum(timer);

		ESP_ERROR_CHECK(timer_set_counter_value(timerGroup, timerNum, TIMER_COUNT_ZERO));
		ESP_ERROR_CHECK(timer_isr_callback_remove(timerGroup, timerNum));
		ESP_ERROR_CHECK(timer_deinit(timerGroup, timerNum));

	#elif ESP_IDF_VERSION_MAJOR == 5

		timer_ptr_t timerPtr = getTimer(timer);
		ESP_ERROR_CHECK(gptimer_del_timer(**timerPtr));
		**timerPtr = NULL;
	#endif

	return true;
}

bool uhwtPlatformStopTimer(uhwt_timer_t timer) {

	#if ESP_IDF_VERSION_MAJOR == 4

		timer_group_t timerGroup = getTimerGroup(timer);
		timer_idx_t timerNum = getTimerNum(timer);

		ESP_ERROR_CHECK(timer_set_alarm(timerGroup, timerNum, false));
		ESP_ERROR_CHECK(timer_pause(timerGroup, timerNum));
	#elif ESP_IDF_VERSION_MAJOR == 5

		timer_ptr_t timerPtr = getTimer(timer);
		ESP_ERROR_CHECK(gptimer_stop(**timerPtr));
		ESP_ERROR_CHECK(gptimer_disable(**timerPtr));
	#endif

	return true;
}

bool uhwtPlatformStartTimer(uhwt_timer_t timer) {
	
	#if ESP_IDF_VERSION_MAJOR == 4

		timer_group_t timerGroup = getTimerGroup(timer);
		timer_idx_t timerNum = getTimerNum(timer);

		// run timer
		ESP_ERROR_CHECK(timer_set_auto_reload(timerGroup, timerNum, true));
		ESP_ERROR_CHECK(timer_set_alarm(timerGroup, timerNum, true));
		ESP_ERROR_CHECK(timer_start(timerGroup, timerNum));
	#elif ESP_IDF_VERSION_MAJOR == 5

		timer_ptr_t timerPtr = getTimer(timer);

		// starts timer
		ESP_ERROR_CHECK(gptimer_enable(**timerPtr));
		ESP_ERROR_CHECK(gptimer_start(**timerPtr));
	#endif

	return true;
}

bool uhwtPlatformSetStats(uhwt_timer_t timer, uhwt_prescalar_t scalar, uhwt_timertick_t timerTicks) {
	#if ESP_IDF_VERSION_MAJOR == 4

		timer_group_t timerGroup = getTimerGroup(timer);
		timer_idx_t timerNum = getTimerNum(timer);

		ESP_ERROR_CHECK(timer_set_alarm_value(timerGroup, timerNum, timerTicks));
		ESP_ERROR_CHECK(timer_set_divider(timerGroup, timerNum, scalar));
		return true;
	#elif ESP_IDF_VERSION_MAJOR == 5

		uhwt_freq_t tempFreq = uhwtCalcFreq(scalar, timerTicks);
		uhwt_timertick_t tempTicks = 1;

		// adjusts frequency to be above min
		while (tempFreq < HARD_TIMER_FREQ_MIN) {
			tempFreq *= 2;
			tempTicks *= 2;
		}

		timerConfigs[timer].resolution_hz = tempFreq;
		timerAlarmConfigs[timer].alarm_count = tempTicks;

		if (!uhwtPlatformDeconstructTimer(timer)) {
			return false;
		}
		return uhwtPlatformInitTimer(timer);

	#endif
}

bool uhwtPlatformEqualFreq(uhwt_freq_t targetFreq, uhwt_prescalar_t scalar, uhwt_timertick_t ticks) {
	if (APB_CLK_FREQ % (scalar * ticks) != 0) {
		return false;
	}
	return true;
}

uhwt_prescalar_t uhwtGetNextPreScalar(uhwt_prescalar_t prevScalar) {
	// u16 bit max
	// 2^16 - 1 -> 2^15 -> 2^14 -> ... -> 2^2 -> 2^1 -> 1 -> 0 -> 2^16 - 1

	#define MAX_PRESCALAR_BITS 16
	
	// 0: start of request
	if (prevScalar == 0) {
		return UINT16_MAX;
	}
	// 2^16 - 1
	else if (prevScalar == UINT16_MAX) {
		return 0b1000000000000000;
	}
	else {
		prevScalar = prevScalar >> (1);
		for (uint8_t i = 0; i < MAX_PRESCALAR_BITS; i++) {
			if (!(prevScalar ^ (1 << i))) {
				return prevScalar;
			}
		}
	}
	return 0;
}

uhwt_prescalar_t uhwtPlatformGetPreScalar(uhwt_timer_t timer) {

	#if ESP_IDF_VERSION_MAJOR == 4
		timer_config_t config;

		timer_group_t timerGroup = getTimerGroup(timer);
		timer_idx_t timerNum = getTimerNum(timer);

		ESP_ERROR_CHECK(timer_get_config(timerGroup, timerNum, &config));
		return config.divider;
	#elif ESP_IDF_VERSION_MAJOR == 5
		// TODO: use gptimer_get_resolution or user set for frequency?
		return uhwtCalcScalar(timerConfigs[timer].resolution_hz, uhwtPlatformGetTimerTicks(timer));
	#endif
}

uhwt_timertick_t uhwtPlatformGetTimerTicks(uhwt_timer_t timer) {
	#if ESP_IDF_VERSION_MAJOR == 4

		uhwt_timertick_t ticks;

		timer_group_t timerGroup = getTimerGroup(timer);
		timer_idx_t timerNum = getTimerNum(timer);

		ESP_ERROR_CHECK(timer_get_alarm_value(timerGroup, timerNum, &ticks));
		return ticks;
	#elif ESP_IDF_VERSION_MAJOR == 5
		return timerAlarmConfigs[timer].alarm_count;
	#endif
}

bool uhwtPlatformValidTimerTicks(uhwt_timertick_t ticks) {
	
	/**
	 * Pre Scalar counts (and amount of timers)
	 * 
	 * General Timers: https://docs.espressif.com/projects/arduino-esp32/en/latest/api/timer.html
	 * 
	 * ESP32: 64 (2x2) https://documentation.espressif.com/esp32_datasheet_en.pdf
	 * ESP32-S2: 64 (4? general vs 1? datasheet) https://documentation.espressif.com/esp32-s2_datasheet_en.pdf
	 * ESP32-S3: 54? datasheet vs 64? general (4) https://documentation.espressif.com/esp32-s3_datasheet_en.pdf
	 * ESP32-C2 or ESP-8684: 54 datasheet only (1 datasheet only) https://documentation.espressif.com/esp8684_datasheet_en.pdf
	 * ESP32-C3: 54 (2) https://documentation.espressif.com/esp32-c3_datasheet_en.pdf
	 * ESP32-C5: 54 datasheet only (2 datasheet only) https://documentation.espressif.com/esp32-c5_datasheet_en.pdf
	 * ESP32-C6: 54? datasheet vs 64? general (2) https://documentation.espressif.com/esp32-c6_datasheet_en.pdf
	 * ESP32-C61: 54 datasheet only (2 datasheet only) https://documentation.espressif.com/esp32-c61_datasheet_en.pdf
	 * ESP32-H2: 54? datasheet vs 64? general (2) https://documentation.espressif.com/esp32-h2_datasheet_en.pdf
	 * ESP32-P4: 54 datasheet only (4 datasheet only) https://documentation.espressif.com/esp32-p4-chip-revision-v1.3_datasheet_en.pdf
	 */

	// TODO: 54 bit ticks

	if (ticks == 0) {
		return false;
	}
	
	return true;
}

bool uhwtPlatformValidPreScalar(uhwt_prescalar_t scalar) {
	// scalar >= 2
	if (0xfffe & scalar) {
		return true;
	}
	return false;
}

bool uhwtPlatformSetCallbackParams(uhwt_timer_t timer,
		uhwt_function_ptr_t function, uhwt_params_ptr_t params) {
		
	#if ESP_IDF_VERSION_MAJOR == 4

		timer_group_t timerGroup = getTimerGroup(timer);
		timer_idx_t timerNum = getTimerNum(timer);

		ESP_ERROR_CHECK(timer_isr_callback_add(timerGroup,
				timerNum, uhwtGetCallback(timer), params,
				setPriority(uhwtPriorities[timer])));

		return true;

	#elif ESP_IDF_VERSION_MAJOR == 5

		if (!uhwtPlatformDeconstructTimer(timer)) {
			return false;
		}

		timerCallbackConfigs[timer].on_alarm = uhwtGetCallback(timer);
		return uhwtPlatformInitTimer(timer);

	#endif
}

#endif