/*
	hardware_timer_test_common.c - tests common timer methods
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

#include "hardware_timer_test_priv.h"

/**
 * Tests to run:
 * 
 * uhwtTimerInitialized
 * uhwtTimerStarted
 * uhwtClaimTimer
 * uhwtUnclaimTimer
 * uhwtTimerClaimed
 * uwhtGetNextTimer
 * uhwtValidFrequency
 * uhwtValidTimer
 * 
 * Common invalid args:
 * 
 * uhwtInitTimer
 * uhwtDeconstructTimer
 * uhwtStopTimer
 * uhwtStartTimer
 * uhwtClaimTimerStats
 * uwhtGetNextTimerStats
 * uhwtGetStats
 * uhwtGetClosestStats
 * uhwtSetCallbackParams
 * uhwtSetStats
 * uhwtSetupTimer
 * uhwtSetupComplexTimer
 * uhwtCalcFreq
 * uhwtGetPreScalar
 * uhwtGetTimerTicks
 * uhwtEqualFreq
 * uhwtSetPriority
 * uhwtValidTimerPreScalar
 * uhwtValidTimerTicks
 * uhwtGetCallback
 * uhwtGetNextPreScalar
 * uhwtCalcTicks
 * uhwtCalcScalar
 * uhwtPlatformValidPreScalar
 * uhwtPlatformValidTimerTicks
 */

#if UHWT_TIMER_FREQ_MAX <= 0
	memCharString noMaxFreqIgnore[] = {"Skipping rest of tests that require >0 UHWT_TIMER_FREQ_MAX"};
#else
	#define ENOUGH_FREQ
#endif

#if UHWT_TIMER_COUNT <= 0
	memCharString noTimerIgnore[] = {"Skipping rest of tests that require >0 UHWT_TIMER_COUNT"};
#else
	#define ENOUGH_TIMERS
#endif

#if !defined(ENOUGH_FREQ) && !defined(ENOUGH_TIMERS)
	memCharString noTimerOrFreqIgnore[] = {"Skipping rest of tests that require >0 for UHWT_TIMER_COUNT and UHWT_TIMER_FREQ_MAX"};
#endif

// freq
#define VALID_FREQ 1
#define INVALID_FREQ 0

// timer
#define VALID_TIMER 0
#define INVALID_TIMER UHWT_TIMER_INVALID

// scalar
#if UHWT_SUPPORT_ESP32
	#define VALID_SCALAR 2
#else
	#define VALID_SCALAR 1
#endif
#define INVALID_SCALAR 0

// ticks
#define VALID_TICKS 1
#define INVALID_TICKS 0

/**
 * Tests that given defaults are correct
 * 
 * @return if given defaults are correct
 */
bool testValidDefaults() {

	bool allValid = true;

	#ifdef ENOUGH_FREQ
		if (!uhwtValidFrequency(VALID_FREQ)) {
			allValid = false;
		}
	#endif
	if (uhwtValidFrequency(INVALID_FREQ)) {
		allValid = false;
	}
	#ifdef ENOUGH_TIMERS
		if (!uhwtValidTimer(VALID_TIMER)) {
			allValid = false;
		}
	#endif
	if (uhwtValidTimer(INVALID_TIMER)) {
		allValid = false;
	}
	#if defined(ENOUGH_TIMERS) && defined(ENOUGH_FREQ)
		if (!uhwtValidTimerPreScalar(VALID_TIMER, VALID_SCALAR)) {
			allValid = false;
		}
		if (uhwtValidTimerPreScalar(VALID_TIMER, INVALID_SCALAR)) {
			allValid = false;
		}
		if (!uhwtValidTimerTicks(VALID_TIMER, VALID_TICKS)) {
			allValid = false;
		}
		if (uhwtValidTimerTicks(VALID_TIMER, INVALID_TICKS)) {
			allValid = false;
		}
	#endif
	return allValid;
}

/**
 * Tests 'uhwtValidFrequency'
 */
void testValidFreq() {

	// invalid inputs
	if (uhwtValidFrequency(INVALID_FREQ)) {
		TEST_FAIL();
	}
	if (uhwtValidFrequency(UHWT_TIMER_FREQ_MAX + 1)) {
		TEST_FAIL();
	}

	#ifdef ENOUGH_FREQ

		// valid inputs
		if (!uhwtValidFrequency(VALID_FREQ)) {
			TEST_FAIL();
		}
		if (!uhwtValidFrequency(UHWT_TIMER_FREQ_MAX)) {
			TEST_FAIL();
		}
		TEST_PASS();
	#else
		TEST_IGNORE_MESSAGE(noMaxFreqIgnore);
	#endif
}

/**
 * Tests 'uhwtValidTimer'
 */
void testValidTimer() {

	// invalid inputs
	if (uhwtValidTimer(INVALID_TIMER)) {
		TEST_FAIL();
	}
	if (uhwtValidTimer(UHWT_TIMER_COUNT)) {
		TEST_FAIL();
	}
	if (uhwtValidTimer(-2)) {
		TEST_FAIL();
	}
	
	#ifdef ENOUGH_TIMERS

		// valid inputs
		if (!uhwtValidTimer(VALID_TIMER)) {
			TEST_FAIL();
		}
		TEST_PASS();
	#else
		TEST_IGNORE_MESSAGE(noTimerIgnore);
	#endif
}

/**
 * Tests 'uhwtInitTimer'
 * Doesn't test with platform
 */
void testInitTimerArgs() {

	// invalid inputs
	if (uhwtInitTimer(INVALID_TIMER)) {
		TEST_FAIL();
	}

	#ifdef ENOUGH_TIMERS

		// valid inputs
		uhwtSetTimerInitialized(VALID_TIMER);
		if (uhwtInitTimer(VALID_TIMER)) {
			TEST_FAIL();
		}
		uhwtSetTimerDeconstructed(VALID_TIMER);
		TEST_PASS();
	#else
		TEST_IGNORE_MESSAGE(noTimerIgnore);
	#endif
}

/**
 * Tests 'uhwtDeconstructTimer'
 * Doesn't test with platform
 */
void testDeconstructTimerArgs() {

	// invalid inputs
	if (uhwtDeconstructTimer(INVALID_TIMER)) {
		TEST_FAIL();
	}

	#ifdef ENOUGH_TIMERS

		// not initialized
		if (uhwtDeconstructTimer(VALID_TIMER)) {
			TEST_FAIL();
		}

		// started
		uhwtSetTimerInitialized(VALID_TIMER);
		uhwtSetTimerStarted(VALID_TIMER);
		if (uhwtDeconstructTimer(VALID_TIMER)) {
			TEST_FAIL();
		}
		uhwtSetTimerStopped(VALID_TIMER);
		uhwtSetTimerDeconstructed(VALID_TIMER);
		
		TEST_PASS();
	#else
		TEST_IGNORE_MESSAGE(noTimerIgnore);
	#endif
}

/**
 * Tests 'uhwtTimerInitialized'
 */
void testInitialized() {

	// invalid inputs
	if (uhwtTimerInitialized(INVALID_TIMER)) {
		TEST_FAIL();
	}

	#ifdef ENOUGH_TIMERS

		// not initialized
		if (uhwtTimerInitialized(VALID_TIMER)) {
			TEST_FAIL();
		}

		// valid inputs
		uhwtSetTimerInitialized(VALID_TIMER);
		if (!uhwtTimerInitialized(VALID_TIMER)) {
			TEST_FAIL();
		}
		uhwtSetTimerDeconstructed(VALID_TIMER);
		if (uhwtTimerInitialized(VALID_TIMER)) {
			TEST_FAIL();
		}
		
		TEST_PASS();
	#else
		TEST_IGNORE_MESSAGE(noTimerIgnore);
	#endif
}

/**
 * Tests 'uhwtClaimTimer'
 */
void testClaimTimer() {

	// invalid inputs
	if (uhwtClaimTimer(NULL)) {
		TEST_FAIL();
	}

	#ifdef ENOUGH_TIMERS

		// timer invalid
		uhwt_timer_t timer = INVALID_TIMER;
		if (!uhwtClaimTimer(&timer)) {
			TEST_FAIL();
		}
		if (!uhwtValidTimer(timer)) {
			TEST_FAIL();
		}
		if (!uhwtUnclaimTimer(timer)) {
			TEST_FAIL();
		}

		// valid timer
		timer = VALID_TIMER;
		if (!uhwtClaimTimer(&timer)) {
			TEST_FAIL();
		}
		if (!uhwtValidTimer(timer)) {
			TEST_FAIL();
		}

		// repeat claim
		if (uhwtClaimTimer(&timer)) {
			TEST_FAIL();
		}
		if (!uhwtValidTimer(timer)) {
			TEST_FAIL();
		}
		if (!uhwtUnclaimTimer(timer)) {
			TEST_FAIL();
		}

		// repeat unclaim
		if (uhwtUnclaimTimer(timer)) {
			TEST_FAIL();
		}

		// timer started
		timer = VALID_TIMER;
		uhwtSetTimerInitialized(timer);
		uhwtSetTimerStarted(timer);

		if (uhwtClaimTimer(&timer)) {
			TEST_FAIL();
		}
		if (uhwtUnclaimTimer(timer)) {
			TEST_FAIL();
		}

		uhwtSetTimerStopped(timer);
		uhwtSetTimerDeconstructed(timer);

		// claim all
		for (uint8_t i = 0; i < UHWT_TIMER_COUNT; i++) {
			timer = INVALID_TIMER;

			if (!uhwtClaimTimer(&timer)) {
				TEST_FAIL();
			}
			if (!uhwtValidTimer(timer)) {
				TEST_FAIL();
			}
		}

		// all are claimed
		timer = INVALID_TIMER;
		if (uhwtClaimTimer(&timer)) {
			TEST_FAIL();
		}
		if (uhwtValidTimer(timer)) {
			TEST_FAIL();
		}

		// release all
		for (uint8_t i = 0; i < UHWT_TIMER_COUNT; i++) {

			if (!uhwtUnclaimTimer(i)) {
				TEST_FAIL();
			}
		}
		
		TEST_PASS();
	#else
		TEST_IGNORE_MESSAGE(noTimerIgnore);
	#endif
}

/**
 * Tests 'uhwtClaimTimerStats'
 * Doesn't test with platform
 */
void testClaimTimerStatsArgs() {

	// invalid inputs
	uhwt_claim_s claimArgs;
	if (uhwtClaimTimerStats(NULL, claimArgs)) {
		TEST_FAIL();
	}

	#ifdef ENOUGH_TIMERS

		// default to 'uhwtClaimTimer'
		uhwt_timer_t timer = VALID_TIMER;
		if (!uhwtClaimTimerStats(&timer, claimArgs)) {
			TEST_FAIL();
		}
		if (timer != VALID_TIMER) {
			TEST_FAIL();
		}

		if (!uhwtUnclaimTimer(timer)) {
			TEST_FAIL();
		}

		TEST_PASS();
		
	#else
		TEST_IGNORE_MESSAGE(noTimerIgnore);
	#endif
}

/**
 * Tests 'uhwtUnclaimTimer'
 */
void testUnclaimTimer() {

	// invalid inputs
	if (uhwtUnclaimTimer(INVALID_TIMER)) {
		TEST_FAIL();
	}

	#ifdef ENOUGH_TIMERS

		uhwt_timer_t timer = INVALID_TIMER;

		// timer not claimed
		if (uhwtUnclaimTimer(VALID_TIMER)) {
			TEST_FAIL();
		}

		// claim valid
		if (!uhwtClaimTimer(&timer)) {
			TEST_FAIL();
		}
		if (timer != VALID_TIMER) {
			TEST_FAIL();
		}
		if (!uhwtValidTimer(timer)) {
			TEST_FAIL();
		}
		if (!uhwtUnclaimTimer(timer)) {
			TEST_FAIL();
		}

		// repeat unclaim
		if (uhwtUnclaimTimer(timer)) {
			TEST_FAIL();
		}
		
		TEST_PASS();
	#else
		TEST_IGNORE_MESSAGE(noTimerIgnore);
	#endif
}

/**
 * Tests 'uhwtTimerClaimed'
 */
void testTimerClaimed() {

	// invalid inputs
	if (uhwtTimerClaimed(INVALID_TIMER)) {
		TEST_FAIL();
	}

	#ifdef ENOUGH_TIMERS

		if (uhwtTimerClaimed(VALID_TIMER)) {
			TEST_FAIL();
		}

		uhwt_timer_t timer = INVALID_TIMER;

		// claim valid
		if (!uhwtClaimTimer(&timer)) {
			TEST_FAIL();
		}
		if (!uhwtValidTimer(timer)) {
			TEST_FAIL();
		}
		if (!uhwtTimerClaimed(timer)) {
			TEST_FAIL();
		}

		// unclaim valid
		if (!uhwtUnclaimTimer(timer)) {
			TEST_FAIL();
		}
		if (uhwtTimerClaimed(timer)) {
			TEST_FAIL();
		}
		
		TEST_PASS();
	#else
		TEST_IGNORE_MESSAGE(noTimerIgnore);
	#endif
}

/**
 * Tests 'uhwtStartTimer'
 * Doesn't test with platform
 */
void testStartTimerArgs() {

	// invalid inputs
	if (uhwtStartTimer(INVALID_TIMER)) {
		TEST_FAIL();
	}

	#ifdef ENOUGH_TIMERS

		// timer not configured
		if (uhwtStartTimer(VALID_TIMER)) {
			TEST_FAIL();
		}

		// timer started
		uhwtSetTimerInitialized(VALID_TIMER);
		uhwtSetTimerStarted(VALID_TIMER);
		if (uhwtStartTimer(VALID_TIMER)) {
			TEST_FAIL();
		}
		uhwtSetTimerStopped(VALID_TIMER);
		uhwtSetTimerDeconstructed(VALID_TIMER);
		
		TEST_PASS();
	#else
		TEST_IGNORE_MESSAGE(noTimerIgnore);
	#endif
}

/**
 * Tests 'uhwtStopTimer'
 * Doesn't test with platform
 */
void testStopTimerArgs() {

	// invalid inputs
	if (uhwtStopTimer(INVALID_TIMER)) {
		TEST_FAIL();
	}

	#ifdef ENOUGH_TIMERS

		// timer not configured
		if (uhwtStopTimer(VALID_TIMER)) {
			TEST_FAIL();
		}

		// timer initialized
		uhwtSetTimerInitialized(VALID_TIMER);
		if (uhwtStopTimer(VALID_TIMER)) {
			TEST_FAIL();
		}
		uhwtSetTimerDeconstructed(VALID_TIMER);
		
		TEST_PASS();
	#else
		TEST_IGNORE_MESSAGE(noTimerIgnore);
	#endif
}

/**
 * Tests 'uhwtTimerStarted'
 */
void testTimerStarted() {

	// invalid inputs
	if (uhwtTimerStarted(INVALID_TIMER)) {
		TEST_FAIL();
	}

	#ifdef ENOUGH_TIMERS

		// timer not configured
		if (uhwtTimerStarted(VALID_TIMER)) {
			TEST_FAIL();
		}

		// timer initialized
		uhwtSetTimerInitialized(VALID_TIMER);
		if (uhwtTimerStarted(VALID_TIMER)) {
			TEST_FAIL();
		}

		// timer started
		uhwtSetTimerStarted(VALID_TIMER);
		if (!uhwtTimerStarted(VALID_TIMER)) {
			TEST_FAIL();
		}

		// timer stopped
		uhwtSetTimerStopped(VALID_TIMER);
		if (uhwtTimerStarted(VALID_TIMER)) {
			TEST_FAIL();
		}

		uhwtSetTimerDeconstructed(VALID_TIMER);
		
		TEST_PASS();
	#else
		TEST_IGNORE_MESSAGE(noTimerIgnore);
	#endif
}

/**
 * Tests 'testGetNextTimer'
 */
void testGetNextTimer() {

	#ifdef ENOUGH_TIMERS

		// getting same timer when no action occurs
		for (uint8_t i = 0; i < 2; i++) {
			uhwt_timer_t timer = uwhtGetNextTimer();
			if (timer != VALID_TIMER) {
				TEST_FAIL();
			}
		}

		// initialized timers
		for (uint8_t i = 0; i < UHWT_TIMER_COUNT; i++) {
			uhwt_timer_t timer = uwhtGetNextTimer();
			uhwtSetTimerInitialized(timer);
			if (timer != (uhwt_timer_t)i) {
				TEST_FAIL();
			}
		}
		if (uhwtValidTimer(uwhtGetNextTimer())) {
			TEST_FAIL();
		}
		for (uint8_t i = 0; i < UHWT_TIMER_COUNT; i++) {
			uhwtSetTimerDeconstructed(i);
		}

		// claimed timers
		for (uint8_t i = 0; i < UHWT_TIMER_COUNT; i++) {
			uhwt_timer_t timer = uwhtGetNextTimer();
			uhwtClaimTimer(&timer);
			if (timer != (uhwt_timer_t)i) {
				TEST_FAIL();
			}
		}
		if (uhwtValidTimer(uwhtGetNextTimer())) {
			TEST_FAIL();
		}
		for (uint8_t i = 0; i < UHWT_TIMER_COUNT; i++) {
			uhwtUnclaimTimer(i);
		}
		
		TEST_PASS();
	#else
		TEST_IGNORE_MESSAGE(noTimerIgnore);
	#endif
}

/**
 * Tests 'uwhtPlatformGetNextTimerStats'
 * Doesn't test with platform
 */
void testPlatformGetNextTimerStatsArgs() {

	#ifdef ENOUGH_TIMERS
		uhwt_claim_s claimArgs;
		if (uwhtPlatformGetNextTimerStats(claimArgs) != UHWT_TIMER_INVALID) {
			TEST_FAIL();
		}
		TEST_PASS();
	#else
		TEST_IGNORE_MESSAGE(noTimerIgnore);
	#endif
}

/**
 * Tests 'uhwtGetStats'
 * Doesn't test with platform
 */
void testGetStatsArgs() {

	#ifdef ENOUGH_FREQ

		uhwt_timer_t timer = INVALID_TIMER;
		uhwt_prescalar_t scalar = INVALID_SCALAR;
		uhwt_timertick_t ticks = INVALID_TICKS;

		// invalid inputs
		if (uhwtGetStats(NULL, VALID_FREQ, &scalar, &ticks)) {
			TEST_FAIL();
		}
		if (uhwtGetStats(&timer, INVALID_FREQ, &scalar, &ticks)) {
			TEST_FAIL();
		}
		if (uhwtGetStats(&timer, VALID_FREQ, NULL, &ticks)) {
			TEST_FAIL();
		}
		if (uhwtGetStats(&timer, VALID_FREQ, &scalar, NULL)) {
			TEST_FAIL();
		}

		#ifdef ENOUGH_TIMERS

			// claimed and started
			timer = VALID_TIMER;
			uhwtClaimTimer(&timer);
			uhwtSetTimerInitialized(timer);
			uhwtSetTimerStarted(timer);

			if (uhwtGetStats(&timer, VALID_FREQ, &scalar, &ticks)) {
				TEST_FAIL();
			}

			uhwtSetTimerStopped(timer);
			uhwtSetTimerDeconstructed(timer);
			uhwtUnclaimTimer(timer);
			
			TEST_PASS();
		#else
			TEST_IGNORE_MESSAGE(noTimerIgnore);
		#endif

	#else
		TEST_IGNORE_MESSAGE(noMaxFreqIgnore);
	#endif
}

/**
 * Tests 'uhwtGetClosestStats'
 * Doesn't test with platform
 */
void testGetClosestStatsArgs() {

	#ifndef UHWT_CONFIGS_NOT_EQUAL
		TEST_IGNORE_MESSAGE("'uhwtGetClosestStats' same as 'uhwtGetStats'");
	#else
		#ifdef ENOUGH_FREQ
			uhwt_timer_t timer = INVALID_TIMER;
			uhwt_prescalar_t scalar = INVALID_SCALAR;
			uhwt_timertick_t ticks = INVALID_TICKS;

			// invalid inputs
			if (uhwtGetClosestStats(NULL, VALID_FREQ, &scalar, &ticks)) {
				TEST_FAIL();
			}
			if (uhwtGetClosestStats(&timer, INVALID_FREQ, &scalar, &ticks)) {
				TEST_FAIL();
			}
			if (uhwtGetClosestStats(&timer, VALID_FREQ, NULL, &ticks)) {
				TEST_FAIL();
			}
			if (uhwtGetClosestStats(&timer, VALID_FREQ, &scalar, NULL)) {
				TEST_FAIL();
			}
			TEST_PASS();

		#else
			TEST_IGNORE_MESSAGE(noMaxFreqIgnore);
		#endif
	#endif
}

/**
 * Tests 'uhwtSetStats'
 * Doesn't test with platform
 */
void testSetStatsArgs() {

	#if defined(ENOUGH_TIMERS) && defined(ENOUGH_FREQ)

		// invalid inputs
		if (uhwtSetStats(INVALID_TIMER, VALID_SCALAR, VALID_TICKS)) {
			TEST_FAIL();
		}

		// not initialized
		if (uhwtSetStats(VALID_TIMER, VALID_SCALAR, VALID_TICKS)) {
			TEST_FAIL();
		}

		// invalid stats
		uhwtSetTimerInitialized(VALID_TIMER);
		if (uhwtSetStats(VALID_TIMER, INVALID_SCALAR, VALID_TICKS)) {
			TEST_FAIL();
		}
		if (uhwtSetStats(VALID_TIMER, VALID_SCALAR, INVALID_TICKS)) {
			TEST_FAIL();
		}

		// started
		uhwtSetTimerStarted(VALID_TIMER);
		if (uhwtSetStats(VALID_TIMER, VALID_SCALAR, VALID_TICKS)) {
			TEST_FAIL();
		}
		uhwtSetTimerStopped(VALID_TIMER);
		uhwtSetTimerDeconstructed(VALID_TIMER);

		TEST_PASS();

	#else
		TEST_IGNORE_MESSAGE(noTimerOrFreqIgnore);
	#endif
}

/**
 * Tests 'uhwtGetPreScalar'
 * Doesn't test with platform
 */
void testGetPreScalarArgs() {

	// invalid inputs
	if (uhwtGetPreScalar(INVALID_TIMER)) {
		TEST_FAIL();
	}

	TEST_PASS();
}

/**
 * Tests 'uhwtGetTimerTicks'
 * Doesn't test with platform
 */
void testGetGetTimerTicksArgs() {
	
	// invalid inputs
	if (uhwtGetTimerTicks(INVALID_TIMER)) {
		TEST_FAIL();
	}

	TEST_PASS();
}

/**
 * Empty function for testing callbacks
 */
void emptyFunction() {}

/**
 * Tests 'uhwtSetCallbackParams'
 * Doesn't test with platform
 */
void testSetCallbackParams() {

	// invalid inputs
	if (uhwtSetCallbackParams(INVALID_TIMER, &emptyFunction, NULL)) {
		TEST_FAIL();
	}

	#ifdef ENOUGH_TIMERS

		// not initialzied
		if (uhwtSetCallbackParams(VALID_TIMER, &emptyFunction, NULL)) {
			TEST_FAIL();
		}

		// invalid function
		uhwtSetTimerInitialized(VALID_TIMER);
		if (uhwtSetCallbackParams(VALID_TIMER, NULL, NULL)) {
			TEST_FAIL();
		}

		// started
		uhwtSetTimerStarted(VALID_TIMER);
		if (uhwtSetCallbackParams(VALID_TIMER, &emptyFunction, NULL)) {
			TEST_FAIL();
		}
		uhwtSetTimerStopped(VALID_TIMER);
		uhwtSetTimerDeconstructed(VALID_TIMER);

		TEST_PASS();

	#else
		TEST_IGNORE_MESSAGE(noTimerIgnore);
	#endif
}

/**
 * Tests 'uhwtSetupTimer'
 * Doesn't test with platform
 */
void testSetupTimerArgs() {

	#ifdef ENOUGH_FREQ

		uhwt_timer_t timer = INVALID_TIMER;

		// invalid inputs
		if (uhwtSetupTimer(NULL, VALID_FREQ, &emptyFunction, NULL)) {
			TEST_FAIL();
		}
		if (uhwtSetupTimer(&timer, VALID_FREQ, NULL, NULL)) {
			TEST_FAIL();
		}
		TEST_PASS();

	#else
		TEST_IGNORE_MESSAGE(noMaxFreqIgnore);
	#endif
}

/**
 * Tests 'uhwtSetupComplexTimer'
 * Doesn't test with platform
 */
void testSetupComplexTimerArgs() {

	#ifdef ENOUGH_FREQ

		uhwt_timer_t timer = INVALID_TIMER;

		// invalid inputs
		if (uhwtSetupComplexTimer(NULL, VALID_FREQ, &emptyFunction, NULL, UHWT_PRIORITY_DEFAULT)) {
			TEST_FAIL();
		}
		if (uhwtSetupComplexTimer(&timer, VALID_FREQ, NULL, NULL, UHWT_PRIORITY_DEFAULT)) {
			TEST_FAIL();
		}
		TEST_PASS();

	#else
		TEST_IGNORE_MESSAGE(noMaxFreqIgnore);
	#endif
}

/**
 * Tests 'uhwtCalcFreq'
 * Doesn't test with platform
 */
void testCalcFreqArgs() {

	#if defined(ENOUGH_TIMERS) && defined(ENOUGH_FREQ)

		// invalid inputs
		if (uhwtCalcFreq(INVALID_SCALAR, VALID_TICKS)) {
			TEST_FAIL();
		}
		if (uhwtCalcFreq(VALID_SCALAR, INVALID_TICKS)) {
			TEST_FAIL();
		}
		TEST_PASS();

	#else
		TEST_IGNORE_MESSAGE(noTimerOrFreqIgnore);
	#endif
}

/**
 * Tests 'uhwtEqualFreq'
 * Doesn't test with platform
 */
void testEqualFreqArgs() {

	#if defined(ENOUGH_TIMERS) && defined(ENOUGH_FREQ)

		// invalid inputs
		if (uhwtEqualFreq(INVALID_FREQ, VALID_SCALAR, VALID_TICKS)) {
			TEST_FAIL();
		}
		if (uhwtEqualFreq(VALID_FREQ, INVALID_SCALAR, VALID_TICKS)) {
			TEST_FAIL();
		}
		if (uhwtEqualFreq(VALID_FREQ, VALID_SCALAR, INVALID_TICKS)) {
			TEST_FAIL();
		}
		TEST_PASS();

	#else
		TEST_IGNORE_MESSAGE(noTimerOrFreqIgnore);
	#endif
}

/**
 * Tests 'uhwtPlatformValidPreScalar'
 * Doesn't test with platform
 */
void testPlatformValidPreScalarArgs() {

	#if defined(ENOUGH_TIMERS) && defined(ENOUGH_FREQ)

		if (uhwtPlatformValidPreScalar(INVALID_SCALAR)) {
			TEST_FAIL();
		}
		if (!uhwtPlatformValidPreScalar(VALID_SCALAR)) {
			TEST_FAIL();
		}

		TEST_PASS();
	#else
		TEST_IGNORE_MESSAGE(noTimerOrFreqIgnore);
	#endif
}

/**
 * Tests 'uhwtPlatformValidTimerTicks'
 * Doesn't test with platform
 */
void testPlatformValidTimerTicksArgs() {

	#if defined(ENOUGH_TIMERS) && defined(ENOUGH_FREQ)

		if (uhwtPlatformValidTimerTicks(INVALID_TICKS)) {
			TEST_FAIL();
		}
		if (!uhwtPlatformValidTimerTicks(VALID_TICKS)) {
			TEST_FAIL();
		}

		TEST_PASS();
	#else
		TEST_IGNORE_MESSAGE(noTimerOrFreqIgnore);
	#endif
}

/**
 * Tests 'uhwtValidTimerPreScalar'
 * Doesn't test with platform
 */
void testValidTimerPreScalarArgs() {

	#if defined(ENOUGH_TIMERS) && defined(ENOUGH_FREQ)

		if (uhwtValidTimerPreScalar(INVALID_TIMER, VALID_SCALAR)) {
			TEST_FAIL();
		}
		if (uhwtValidTimerPreScalar(VALID_TIMER, INVALID_SCALAR)) {
			TEST_FAIL();
		}
		if (!uhwtValidTimerPreScalar(VALID_TIMER, VALID_SCALAR)) {
			TEST_FAIL();
		}

		TEST_PASS();
	#else
		TEST_IGNORE_MESSAGE(noTimerOrFreqIgnore);
	#endif
}

/**
 * Tests 'uhwtValidTimerTicks'
 * Doesn't test with platform
 */
void testValidTimerTicksArgs() {

	#if defined(ENOUGH_TIMERS) && defined(ENOUGH_FREQ)

		if (uhwtValidTimerTicks(INVALID_TIMER, VALID_TICKS)) {
			TEST_FAIL();
		}
		if (uhwtValidTimerTicks(VALID_TIMER, INVALID_TICKS)) {
			TEST_FAIL();
		}
		if (!uhwtValidTimerTicks(VALID_TIMER, VALID_TICKS)) {
			TEST_FAIL();
		}

		TEST_PASS();
	#else
		TEST_IGNORE_MESSAGE(noTimerOrFreqIgnore);
	#endif
}

void displayDefaultError() {
	TEST_FAIL_MESSAGE("Platform defaults incorrect");
}

void uhwtTestCommon() {
	UHWT_SET_FILE_NAME("hardware_timer_test_common.c");

	if (testValidDefaults()) {

		// static checks
		RUN_TEST(&testValidFreq);
		RUN_TEST(&testValidTimer);

		// init
		RUN_TEST(&testInitTimerArgs);
		RUN_TEST(&testDeconstructTimerArgs);
		RUN_TEST(&testInitialized);

		// claim
		RUN_TEST(&testClaimTimer);
		RUN_TEST(&testClaimTimerStatsArgs);
		RUN_TEST(&testUnclaimTimer);
		RUN_TEST(&testTimerClaimed);

		// start
		RUN_TEST(&testStartTimerArgs);
		RUN_TEST(&testStopTimerArgs);
		RUN_TEST(&testTimerStarted);

		// next timer
		RUN_TEST(&testGetNextTimer);
		RUN_TEST(&testPlatformGetNextTimerStatsArgs);

		// stats
		RUN_TEST(&testGetStatsArgs);
		RUN_TEST(&testGetClosestStatsArgs);
		RUN_TEST(&testSetStatsArgs);
		RUN_TEST(&testGetPreScalarArgs);
		RUN_TEST(&testGetGetTimerTicksArgs);

		// callback
		RUN_TEST(&testSetCallbackParams);

		// setup
		RUN_TEST(&testSetupTimerArgs);
		RUN_TEST(&testSetupComplexTimerArgs);

		// freq
		RUN_TEST(&testCalcFreqArgs);
		RUN_TEST(&testEqualFreqArgs);

		// valid
		RUN_TEST(&testPlatformValidPreScalarArgs);
		RUN_TEST(&testPlatformValidTimerTicksArgs);
		RUN_TEST(&testValidTimerPreScalarArgs);
		RUN_TEST(&testValidTimerTicksArgs);
	}
	else {
		RUN_TEST(&displayDefaultError);
	}
}