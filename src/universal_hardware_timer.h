/*
	universal_hardware_timer.h - hardware timer interface for microcontrollers
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

#ifndef UNIVERSAL_HARDWARE_TIMER_H
#define UNIVERSAL_HARDWARE_TIMER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

/****************************
 * Library types and constants
****************************/

typedef uint32_t uhwt_freq_t; // hard timer frequency variable
typedef uint8_t uhwt_priority_t; // hard timer execute priority variable
typedef void (*uhwt_function_ptr_t) (void*); // timer callback function pointer
typedef void* uhwt_params_ptr_t; // timer callback parameter type

// hardware timer priority for claiming timers
typedef struct {
	bool slowestTimer: 1; // whether to use slowest timer or not
	bool mostAccurateTimer: 1; // whether to use most accurate timer or not
} uhwt_claim_s;

#define UHWT_PRIORITY_DEFAULT 0 // default hard timer priority
#define UHWT_TIMER_MAX_COUNT 16 // max number of hardware timers supported in library
#define UHWT_TIMER_INVALID_LIT -1 // literal number version of UHWT_TIMER_INVALID

/****************************
 * Platform detection
****************************/

// if a native platform is selected
#define UHWT_SUPPORT_NATIVE ( \
	defined(PIO_NATIVE_ENVIRONMENT) \
)

// if a supported avr platform is selected
#define UHWT_SUPPORT_AVR ( \
	defined(__AVR_ATmega328P__) \
)

// if a supported esp32 platform is selected
#define UHWT_SUPPORT_ESP32 ( \
	defined(ESP32) \
)

// if a supported pico platform is selected
#define UHWT_SUPPORT_PICO ( \
	defined(PICO_RP2040) || \
	defined(PICO_RP2350) \
)

// if a supported platform is selected
#define UHWT_SUPPORT ( \
	UHWT_SUPPORT_NATIVE || \
	UHWT_SUPPORT_AVR || \
	UHWT_SUPPORT_ESP32 || \
	UHWT_SUPPORT_PICO \
)

/****************************
 * Platform defines
****************************/

#if UHWT_SUPPORT_ESP32

	#include <esp_attr.h>
	#include <esp_system.h>
	#include <soc/soc_caps.h>

	/****************************
	 * Timer Config
	 * 
	 * Only 4 hardware timers available
	****************************/

	#define UHWT_TIMER_FREQ_MAX 200000 // max frequency user set timer can be
	#define UHWT_TIMER_COUNT SOC_TIMER_GROUP_TOTAL_TIMERS // amount of hardware timers to use

	typedef uint16_t uhwt_prescalar_t; // prescalar type
	typedef uint64_t uhwt_timertick_t; // timer tick type

	/**
	 * Runs function in ram
	 * 
	 * @param function function literal name
	 * 
	 * @note Usage:
	 * @note [type] UHWT_RAM_ATTR(fName) fName([params]) {
	 * @note 	[code]
	 * @note }
	 */
	#define UHWT_RAM_ATTR(function) IRAM_ATTR

#elif UHWT_SUPPORT_PICO

	#include <pico.h>

	/****************************
	 * Timer Config
	 * 
	 * Only 16 hardware alarm timers available
	 * 
	 * 2 are used for system timing on each core
	 * 
	 * Only 14 timers after api usage
	****************************/

	#define UHWT_TIMER_FREQ_MAX 250000 // max frequency user set timer can be
	#define UHWT_TIMER_COUNT 14 // amount of hardware timers to use

	typedef uint8_t uhwt_prescalar_t; // prescalar type
	typedef int64_t uhwt_timertick_t; // timer tick type

	/**
	 * Runs function in ram
	 * 
	 * @param function function literal name
	 * 
	 * @note Usage:
	 * @note [type] UHWT_RAM_ATTR(fName) fName([params]) {
	 * @note 	[code]
	 * @note }
	 */
	#define UHWT_RAM_ATTR(function) __not_in_flash(__STRING( function ))

#elif UHWT_SUPPORT_AVR

	/****************************
	 * Timer Config
	 * 
	 * Only 3 hardware timers available
	 * 
	 * 1 is used for Arduino timing
	 * 
	 * Only 2 timers after api usage, unless overriden
	****************************/

	#if F_CPU == 16000000L
		#define UHWT_TIMER_FREQ_MAX 120000 // max frequency user set timer can be
	#else
		#error "F_CPU must be 16 (MHz)"
	#endif

	#ifdef UHWT_OVERRIDE_ARDUINO_TIMER
		#define UHWT_TIMER_COUNT 3 // amount of hardware timers to use
	#else
		#define UHWT_TIMER_COUNT 2 // amount of hardware timers to use
	#endif

	typedef uint16_t uhwt_prescalar_t; // prescalar type
	typedef uint16_t uhwt_timertick_t; // timer tick type

#elif UHWT_SUPPORT_NATIVE

	/****************************
	 * Timer Config
	 * 
	 * No hardware timers available
	****************************/

	#define UHWT_TIMER_FREQ_MAX 100000 // max frequency user set timer can be
	#define UHWT_TIMER_COUNT UHWT_TIMER_MAX_COUNT // amount of hardware timers to use

	typedef uint8_t uhwt_prescalar_t; // prescalar type
	typedef uint8_t uhwt_timertick_t; // timer tick type

#else

	/****************************
	 * Timer Config
	 * 
	 * No hardware timers available
	****************************/

	#define UHWT_TIMER_FREQ_MAX 0 // max frequency user set timer can be
	#define UHWT_TIMER_COUNT 0 // amount of hardware timers to use

	typedef uint8_t uhwt_prescalar_t; // prescalar type
	typedef uint8_t uhwt_timertick_t; // timer tick type

#endif

#ifndef UHWT_RAM_ATTR
	/**
	 * Runs function in ram
	 * 
	 * @param function function literal name
	 * 
	 * @note Usage:
	 * @note [type] UHWT_RAM_ATTR(fName) fName([params]) {
	 * @note 	[code]
	 * @note }
	 */
	#define UHWT_RAM_ATTR(funcName)
#endif

typedef enum { // hardware timer enum type
	UHWT_TIMER_INVALID = -1, // invalid counter
	#if UHWT_TIMER_COUNT >= 1
		UHWT_TIMER0,
	#endif
	#if UHWT_TIMER_COUNT >= 2
		UHWT_TIMER1,
	#endif
	#if UHWT_TIMER_COUNT >= 3
		UHWT_TIMER2,
	#endif
	#if UHWT_TIMER_COUNT >= 4
		UHWT_TIMER3,
	#endif
	#if UHWT_TIMER_COUNT >= 5
		UHWT_TIMER4,
	#endif
	#if UHWT_TIMER_COUNT >= 6
		UHWT_TIMER5,
	#endif
	#if UHWT_TIMER_COUNT >= 7
		UHWT_TIMER6,
	#endif
	#if UHWT_TIMER_COUNT >= 8
		UHWT_TIMER7,
	#endif
	#if UHWT_TIMER_COUNT >= 9
		UHWT_TIMER8,
	#endif
	#if UHWT_TIMER_COUNT >= 10
		UHWT_TIMER9,
	#endif
	#if UHWT_TIMER_COUNT >= 11
		UHWT_TIMER10,
	#endif
	#if UHWT_TIMER_COUNT >= 12
		UHWT_TIMER11,
	#endif
	#if UHWT_TIMER_COUNT >= 13
		UHWT_TIMER12,
	#endif
	#if UHWT_TIMER_COUNT >= 14
		UHWT_TIMER13,
	#endif
	#if UHWT_TIMER_COUNT >= 15
		UHWT_TIMER14,
	#endif
	#if UHWT_TIMER_COUNT >= 16
		UHWT_TIMER15,
	#endif
} uhwt_timer_t;

/****************************
 * Library functions
****************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Stops hardware timer from executing
 * 
 * @param timer timer to stop
 * 
 * @return if timer was successfully canceled
 */
bool cancelHardTimer(uhwt_timer_t timer);

/**
 * Starts hardware timer execution
 * 
 * @param timer pointer to timer to start
 * @param freq pointer to desired frequency in Hz
 * @param function pointer to function to call back
 * @param params parameters to pass to callback function
 * @param priority priority to run timer at (0 min, 255 max)
 * 
 * setHardTimer(UHWT_TIMER_INVALID, ...):
 * 
 *                Claimed:     Unclaimed:
 * 
 * Started:           -,           -
 * 
 * Not Started:       -,       Free Timer
 * 
 * setHardTimer(UHWT_TIMER#, ...):
 * 
 *                Claimed:     Unclaimed:
 * 
 * Started:         Fail,      Free Timer
 * 
 * Not Started:  UHWT_TIMER#,  UHWT_TIMER#
 * 
 * @return if timer was successfully set
 */
bool setHardTimer(uhwt_timer_t *timer, uhwt_freq_t *freq,
		uhwt_function_ptr_t function, uhwt_params_ptr_t params,
		uhwt_priority_t priority);

/****************************
 * Universal Hardware Timer Functions
****************************/

/**
 * Initializes timer
 * 
 * @param timer timer to init
 * 
 * @return if successful
 */
bool uhwtInitTimer(uhwt_timer_t timer);

/**
 * Deconstruct timer
 * 
 * @param timer timer to init
 * 
 * @return if successful
 */
bool uhwtDeconstructTimer(uhwt_timer_t timer);

/**
 * Stops timer from executing
 * 
 * @param timer timer to stop
 * 
 * @return if successful
 */
bool uhwtStopTimer(uhwt_timer_t timer);

/**
 * Starts continuous timer
 * 
 * @param timer timer to start
 * 
 * @return if successful
 */
bool uhwtStartTimer(uhwt_timer_t timer);

/**
 * Tests if given timer is initialized
 * 
 * @param timer timer to test
 * 
 * @return if timer is initialized
 */
bool uhwtTimerInitialized(uhwt_timer_t timer);

/**
 * Tests if timer is started
 * 
 * @param timer timer to test
 * 
 * @return start status
 */
bool uhwtTimerStarted(uhwt_timer_t timer);

/**
 * Claims next available timer
 * 
 * @param timer pointer to timer to set
 * 
 * @return if successful
 */
bool uhwtClaimTimer(uhwt_timer_t *timer);

/**
 * Claims next available timer
 * 
 * @param timer pointer to timer to set
 * @param claimArgs claim arguments for selecting timer
 * 
 * @return if successful
 */
bool uhwtClaimTimerStats(uhwt_timer_t *timer, uhwt_claim_s claimArgs);

/**
 * Sets timer as unclaimed
 * 
 * @param timer timer to unclaim
 * 
 * @return if unclaim was successful
 */
bool uhwtUnclaimTimer(uhwt_timer_t timer);

/**
 * Tests if timer is claimed
 * 
 * @param timer timer to test
 * 
 * @return claim status
 */
bool uhwtTimerClaimed(uhwt_timer_t timer);

/**
 * Gets next available timer
 * 
 * @return timer retrieved
 */
uhwt_timer_t uwhtGetNextTimer();

/**
 * Gets next available timer
 * 
 * @param claimArgs claim arguments for selecting timer
 * 
 * @return timer retrieved
 */
uhwt_timer_t uwhtGetNextTimerStats(uhwt_claim_s claimArgs);

/**
 * uhwtGetStats(UHWT_TIMER_INVALID, ...):
 * 
 *                 Claimed:     Unclaimed:
 * 
 * Started:           -,            -
 * 
 * Not Started:       -,        Next Timer
 * 
 * uhwtGetStats(UHWT_TIMER#, ...):
 * 
 *                 Claimed:     Unclaimed:
 * 
 * Started:          Fail,      Next Timer
 * 
 * Not Started:   UHWT_TIMER#,  UHWT_TIMER#
 */
 
/**
 * Gets pre scalar and timer tick stats for target frequency for next available timer
 * 
 * @param timer pointer to timer to store new timer
 * @param targetFreq target frequency to achieve
 * @param scalar pointer to pre scalar to store new value
 * @param timerTicks pointer to timer ticks to store new value
 * 
 * @note uhwtGetStats(UHWT_TIMER_INVALID, ...):
 * @note gets stats for next available timer
 * @note uhwtGetStats(UHWT_TIMER#, ...):
 * @note if not started, gets stats for UHWT_TIMER#
 * @note if started and claimed, function fails
 * @note if started and unclaimed, gets stats for next available timer
 * 
 * @return if stats successfully set
 */
bool uhwtGetStats(uhwt_timer_t *timer, uhwt_freq_t targetFreq,
		uhwt_prescalar_t *scalar, uhwt_timertick_t *timerTicks);

/**
 * uhwtGetClosestStats(UHWT_TIMER_INVALID, ...):
 * 
 *                  Claimed:     Unclaimed:
 * 
 * Started:            -,            -
 * 
 * Not Started:        -,        Next Best
 * 
 * uhwtGetClosestStats(UHWT_TIMER#, ...):
 * 
 *                  Claimed:     Unclaimed:
 * 
 * Started:        Next Best,    Next Best
 * 
 * Not Started:   UHWT_TIMER#,   Next Best
 */
 
/**
 * Gets pre scalar and timer tick stats for target frequency for closest available timer
 * 
 * @param timer pointer to timer to store new timer
 * @param targetFreq target frequency to achieve
 * @param scalar pointer to pre scalar to store new value
 * @param timerTicks pointer to timer ticks to store new value
 * 
 * @note uhwtGetClosestStats(UHWT_TIMER_INVALID, ...):
 * @note gets stats for closest available timer
 * @note uhwtGetClosestStats(UHWT_TIMER#, ...):
 * @note if not started and is claimed, gets stats for closest available timer or given timer
 * @note else, gets stats for closest available timer
 * 
 * @return if stats successfully set
 */
bool uhwtGetClosestStats(uhwt_timer_t *timer, uhwt_freq_t targetFreq,
		uhwt_prescalar_t *scalar, uhwt_timertick_t *timerTicks);

/**
 * Sets timer stats for frequency
 * 
 * @param timer timer to set
 * @param scalar scalar to set
 * @param timerTicks ticks to set
 * 
 * @return if successful
 */
bool uhwtSetStats(uhwt_timer_t timer, uhwt_prescalar_t scalar, uhwt_timertick_t timerTicks);

/**
 * Sets function to execute for timer ISR
 * 
 * @param timer timer to set
 * @param function function to set
 * @param params params to set
 * 
 * @return if successfully set
 */
bool uhwtSetCallbackParams(uhwt_timer_t timer,
		uhwt_function_ptr_t function, uhwt_params_ptr_t params);

/**
 * Runs preprocessed code to initialize and set up timer
 * 
 * @param timer pointer to timer to store
 * @param targetFreq frequency to achieve
 * @param function pointer to function to call back
 * @param params parameters to pass to callback function
 * 
 * @return if successful
 */
bool uhwtSetupTimer(uhwt_timer_t *timer, uhwt_freq_t targetFreq,
		uhwt_function_ptr_t function, uhwt_params_ptr_t params);

/**
 * Runs preprocessed code to initialize and set up complex timer
 * 
 * @param timer pointer to timer to store
 * @param targetFreq frequency to achieve
 * @param function pointer to function to call back
 * @param params parameters to pass to callback function
 * @param priority priority to run timer at (0 min, 255 max)
 * 
 * @return if successful
 */
bool uhwtSetupComplexTimer(uhwt_timer_t *timer, uhwt_freq_t targetFreq,
		uhwt_function_ptr_t function, uhwt_params_ptr_t params,
		uhwt_priority_t priority);

/**
 * Tests if given frequency is valid or not
 * 
 * @param freq frequency to test
 * 
 * @return if frequency is valid
 */
static inline bool uhwtValidFrequency(uhwt_freq_t freq) {
	if (freq > UHWT_TIMER_FREQ_MAX || freq == 0) {
		return false;
	}
	return true;
}

/**
 * Tests if given timer is valid
 * 
 * @param timer timer to test
 * 
 * @return if timer is valid
 */
static inline bool uhwtValidTimer(uhwt_timer_t timer) {
	if (timer < 0 || timer >= UHWT_TIMER_COUNT) {
		return false;
	}
	return true;
}

/**
 * Calculates timer frequency from given timer presets
 * 
 * @param scalar pre scalar for timer
 * @param ticks timer ticks per cycle
 * 
 * @return calculated frequency
 */
uhwt_freq_t uhwtCalcFreq(uhwt_prescalar_t scalar, uhwt_timertick_t ticks);

/****************************
 * Platform Functions
****************************/

/**
 * Gets prescalar for given timer
 * 
 * @param timer timer id for retrieval
 * 
 * @return prescalar
 */
uhwt_prescalar_t uhwtGetPreScalar(uhwt_timer_t timer);

/**
 * Gets timer ticks for given timer
 * 
 * @param timer timer id for retrieval
 * 
 * @return timer ticks
 */
uhwt_timertick_t uhwtGetTimerTicks(uhwt_timer_t timer);

/**
 * Tests if timer presets equal the target frequency
 * 
 * @param targetFreq frequency to test
 * @param scalar pre scalar for timer
 * @param ticks timer ticks per cycle
 * 
 * @return if presets set equal to timer
 */
bool uhwtEqualFreq(uhwt_freq_t targetFreq, uhwt_prescalar_t scalar,
		uhwt_timertick_t ticks);

/**
 * Sets function execution priority
 * 
 * @param timer timer to set
 * @param priority value to set
 */
void uhwtSetPriority(uhwt_timer_t timer, uhwt_priority_t priority);

#ifdef __cplusplus
}
#endif

#endif