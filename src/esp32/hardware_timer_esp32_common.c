/*
	hardware_timer_esp32_common.c - timer configuration for Espressif ESP32
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

#endif