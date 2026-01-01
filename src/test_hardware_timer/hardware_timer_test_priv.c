/*
	hardware_timer_test_priv.c - methods to test timer methods
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
 * Tests to run (excluding common args):
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
 * uhwtValidPreScalar
 * uhwtValidTimerTicks
 * uhwtGetCallback
 * uhwtGetNextPreScalar
 * uhwtCalcTicks
 * uhwtCalcScalar
 * 
 * Platform dependent:
 * 
 * uwhtPlatformGetNextTimerStats
 * uhwtPlatformSetCallbackParams
 * uhwtPlatformInitTimer
 * uhwtPlatformDeconstructTimer
 * uhwtPlatformStopTimer
 * uhwtPlatformStartTimer
 * uhwtPlatformSetStats
 * uhwtPlatformEqualFreq
 * uhwtPlatformGetPreScalar
 * uhwtPlatformGetTimerTicks
 */

#if defined(USE_UNITY)
	#warning "Testing hardware_timer with Unity"
#elif defined(ARDUINO)
	#warning "Testing hardware_timer with Arduino"
#else
	#warning "Testing hardware_timer with C"
#endif

#if UHWT_SUPPORT && !UHWT_SUPPORT_NATIVE

// TODO: setting to 1000 from 10 to test esp32 makes uno error
#define TEST_SLOW_FREQ 10 // frequency for testing slow timing
#ifndef TEST_FAST_FREQ
	#define TEST_FAST_FREQ UHWT_TIMER_FREQ_MAX // target frequency
#endif

/**
 * frequency for testing setting timer
 * 
 * @note included for microcontrollers where
 * @note all timers can't run TEST_SLOW_FREQ
 */
#define TEST_CASES_FREQ 100

#define SLOW_TIMER_PRIORITY 0 // slow timer priority
#define FAST_TIMER_PRIORITY 255 // fast timer priority

#define TEST_DELAY_ELLAPSE_S 1 // time for timer to run for

#ifndef SLOW_TEST_BUFFER
	#define SLOW_TEST_BUFFER 0 // amount slow timer can be off of goal
#endif
#ifndef FAST_TEST_BUFFER
	#define FAST_TEST_BUFFER 0 // amount fast timer can be off of goal
#endif

/**
 * Tests if timer is invalid timer or not
 */
#define TIMER_NOT_INVALID(timer) \
	if (timer != UHWT_TIMER_INVALID) { \
		TEST_FAIL_MESSAGE(setTimerFail); \
	}

memCharString invalidStartFail[] = {"Start State"};
memCharString invalidInitFail[] = {"Init State"};
memCharString startFail[] = {"Start"};
memCharString noSetTimerFail[] = {"No Set Timer"};
memCharString restartFail[] = {"Restart"};
memCharString cancelFail[] = {"Stop"};
memCharString recancelFail[] = {"Restop"};
memCharString allNullFail[] = {"All Null"};
memCharString freqNullFail[] = {"Freq Null"};
memCharString funcNullFail[] = {"Func Null"};

memCharString unclaimInvalidFail[] = {"Unclaim Invalid"};
memCharString unclaimNotClaimedFail[] = {"Unclaim Unclaimed"};
memCharString claimLoopFail[] = {"Claim Loop"};
memCharString allClaimedFail[] = {"All Claimed"};
memCharString unclaimLoopFail[] = {"Unclaim Loop"};
memCharString notClaimedFail[] = {"Not Claimed"};
memCharString isClaimedFail[] = {"Is Claimed"};
memCharString didntUnclaimFail[] = {"Didn't Unclaim"};
memCharString noStartFail[] = {"No Start"};
memCharString claimedActiveFail[] = {"Claimed Active"};
memCharString sameTimerFail[] = {"Same Timer"};

memCharString freq0Fail[] = {"Freq 0"};
memCharString freqMaxFail[] = {"Freq Max"};
memCharString notSameTimerFail[] = {"Not Same Timer"};
memCharString isStartFail[] = {"Is Start"};
memCharString notStartFail[] = {"Not Start"};
memCharString setTimerFail[] = {"Set Timer"};
memCharString setLoopFail[] = {"Set Loop"};
memCharString maxTimerFail[] = {"Max Timer"};
memCharString startLoopFail[] = {"Start Loop"};
memCharString cancelLoopFail[] = {"Cancel Loop"};
memCharString cancelInvalidFail[] = {"Cancel Invalid"};
memCharString startedLoopFail[] = {"Started Loop"};
memCharString didntStopFail[] = {"Didn't Stop"};

/**
 * Priority claim statements
 * P: Priority
 * S: Started
 * N: Not Started
 * C: Claimed
 * U: Unclaimed
 */

memCharString pNotStartedClaimedFail[] = {"PNC"};
memCharString pNotStartedClaimedUnequalFail[] = {"PNC Unequal"};
memCharString pStartedClaimedFail[] = {"PSC"};
memCharString pStartedClaimedUnequalFail[] = {"PSC Unequal"};
memCharString pStartedUnclaimedFail[] = {"PSU"};
memCharString pStartedUnclaimedUnequalFail[] = {"PSU Unequal"};

volatile uint32_t hardTimerCount = 0U;

/**
 * Testing function
 */
void UHWT_RAM_ATTR(testTimingFunction) testTimingFunction(void *params) {
	hardTimerCount++;
}

/**
 * Testing function
 */
void UHWT_RAM_ATTR(testTimingFunctionParams) testTimingFunctionParams(void *params) {
	hardTimerCount += *(uint32_t*)params;
}

/**
 * Resets all timers to off and unclaimed
 */
void resetTimers() {
	for (int i = 0; i < UHWT_TIMER_COUNT; i++) {
		uhwtUnclaimTimer((uhwt_timer_t)i);
		uhwtStopTimer((uhwt_timer_t)i);
		uhwtDeconstructTimer((uhwt_timer_t)i);
	}
}

/**
 * Tests timer start state
 * 
 * @param timer timer to test
 * @param start whether timer should or shouldn't be started
 */
void testGetStartState(uhwt_timer_t timer, bool start) {
	if (uhwtTimerStarted(timer) != start) {
		TEST_FAIL_MESSAGE(invalidStartFail);
	}
}

/**
 * Tests timer init state
 * 
 * @param timer timer to test
 * @param start whether timer should or shouldn't be initialized
 */
void testGetInitializedState(uhwt_timer_t timer, bool init) {
	if (uhwtTimerInitialized(timer) != init) {
		TEST_FAIL_MESSAGE(invalidInitFail);
	}
}

/**
 * Tests that no timers are set when starting
 * 
 * @warning Only call when timers aren't started nor initialized
 */
void testProgramStart() {
	resetTimers();
	for (int i = 0; i < UHWT_TIMER_COUNT; i++) {
		testGetStartState((uhwt_timer_t)i, false);
		testGetInitializedState((uhwt_timer_t)i, false);
	}
	TEST_PASS();
}

/**
 * Tests correct status for repeat starts and cancels
 */
void testRepeat() {
	resetTimers();
	uhwt_freq_t freq = TEST_CASES_FREQ;
	uhwt_timer_t timer = UHWT_TIMER_INVALID;

	if (!setHardTimer(&timer, &freq, &testTimingFunction, NULL, UHWT_PRIORITY_DEFAULT)) {
		TEST_FAIL_MESSAGE(startFail);
	}
	if (timer == UHWT_TIMER_INVALID) {
		TEST_FAIL_MESSAGE(noSetTimerFail);
	}

	uhwt_timer_t secondTimer = timer;
	freq = TEST_CASES_FREQ;
	if (!setHardTimer(&secondTimer, &freq, &testTimingFunction, NULL, UHWT_PRIORITY_DEFAULT)) {
		TEST_FAIL_MESSAGE(startFail);
	}
	if (timer == secondTimer) {
		TEST_FAIL_MESSAGE(restartFail);
	}

	if (!cancelHardTimer(timer)) {
		TEST_FAIL_MESSAGE(cancelFail);
	}
	if (cancelHardTimer(timer)) {
		TEST_FAIL_MESSAGE(recancelFail);
	}
	if (!cancelHardTimer(secondTimer)) {
		TEST_FAIL_MESSAGE(cancelFail);
	}
	if (cancelHardTimer(secondTimer)) {
		TEST_FAIL_MESSAGE(recancelFail);
	}
	TEST_PASS();
}

/**
 * Tests claim functionality
 */
void testClaims(void) {

	resetTimers();

	uhwt_timer_t timer = UHWT_TIMER_INVALID;
	uhwt_timer_t tempTimer = UHWT_TIMER_INVALID;
	uhwt_freq_t freq = TEST_CASES_FREQ;

	// unclaim
	if (uhwtUnclaimTimer(timer)) {
		TEST_FAIL_MESSAGE(unclaimInvalidFail);
	}
	if (uhwtUnclaimTimer(UHWT_TIMER0)) {
		TEST_FAIL_MESSAGE(unclaimNotClaimedFail);
	}

	// none claimed
	for (uint8_t i = 0; i < UHWT_TIMER_COUNT; i++) {
		if (uhwtTimerClaimed(i)) {
			TEST_FAIL_MESSAGE(notClaimedFail);
		}
	}

	// claim
	for (uint8_t i = 0; i < UHWT_TIMER_COUNT; i++) {
		timer = UHWT_TIMER_INVALID;
		if (!uhwtClaimTimer(&timer)) {
			TEST_FAIL_MESSAGE(claimLoopFail);
		}
	}
	timer = UHWT_TIMER_INVALID;
	if (uhwtClaimTimer(&timer)) {
		TEST_FAIL_MESSAGE(allClaimedFail);
	}

	// all claimed
	for (uint8_t i = 0; i < UHWT_TIMER_COUNT; i++) {
		if (!uhwtTimerClaimed(i)) {
			TEST_FAIL_MESSAGE(isClaimedFail);
		}
	}

	// remove all claims and test unclaimed
	for (uint8_t i = 0; i < UHWT_TIMER_COUNT; i++) {
		if (!uhwtUnclaimTimer(i)) {
			TEST_FAIL_MESSAGE(unclaimLoopFail);
		}
		if (uhwtTimerClaimed(i)) {
			TEST_FAIL_MESSAGE(didntUnclaimFail);
		}
	}

	// claim active timer
	if (!setHardTimer(&timer, &freq, testTimingFunction, NULL, UHWT_PRIORITY_DEFAULT)) {
		TEST_FAIL_MESSAGE(noStartFail);
	}
	for (uint8_t i = 0; i < UHWT_TIMER_COUNT - 1; i++) {
		tempTimer = UHWT_TIMER_INVALID;
		if (!uhwtClaimTimer(&tempTimer)) {
			TEST_FAIL_MESSAGE(claimLoopFail);
		}
	}
	if (uhwtClaimTimer(&timer)) {
		TEST_FAIL_MESSAGE(claimedActiveFail);
	}

	// remove all claims
	bool claimed = false;
	for (uint8_t i = 0; i < UHWT_TIMER_COUNT; i++) {
		if (!uhwtUnclaimTimer(i)) {
			if (!claimed) {
				claimed = true;
			}
			else {
				TEST_FAIL_MESSAGE(unclaimLoopFail);
			}
		}
	}

	if (!cancelHardTimer(timer)) {
		TEST_FAIL_MESSAGE(cancelFail);
	}
	TEST_PASS();
}

/**
 * Tests start parameters
 */
void testStart(void) {

	resetTimers();

	uhwt_timer_t timer = UHWT_TIMER_INVALID;
	uhwt_freq_t freq = TEST_CASES_FREQ;

	// null parameters
	if (setHardTimer(NULL, NULL, NULL, NULL, UHWT_PRIORITY_DEFAULT)) {
		TEST_FAIL_MESSAGE(allNullFail);
	}
	TIMER_NOT_INVALID(timer);
	if (setHardTimer(&timer, NULL, testTimingFunction, NULL, UHWT_PRIORITY_DEFAULT)) {
		TEST_FAIL_MESSAGE(freqNullFail);
	}
	TIMER_NOT_INVALID(timer);
	if (setHardTimer(&timer, &freq, NULL, NULL, UHWT_PRIORITY_DEFAULT)) {
		TEST_FAIL_MESSAGE(funcNullFail);
	}
	TIMER_NOT_INVALID(timer);
	if (cancelHardTimer(UHWT_TIMER_INVALID)) {
		TEST_FAIL_MESSAGE(cancelInvalidFail);
	}

	// cancel timer
	if (cancelHardTimer(timer)) {
		TEST_FAIL_MESSAGE(cancelFail);
	}
	if (cancelHardTimer(UHWT_TIMER0)) {
		TEST_FAIL_MESSAGE(cancelFail);
	}

	// set all timers
	for (uint8_t i = 0; i < UHWT_TIMER_COUNT; i++) {
		uhwt_timer_t loopTimer = UHWT_TIMER_INVALID;
		freq = TEST_CASES_FREQ;
		if (!setHardTimer(&loopTimer, &freq, testTimingFunction, NULL, UHWT_PRIORITY_DEFAULT)) {
			TEST_FAIL_MESSAGE(setLoopFail);
		}
		if (!uhwtTimerStarted(loopTimer)) {
			TEST_FAIL_MESSAGE(startedLoopFail);
		}
	}
	if (setHardTimer(NULL, &freq, testTimingFunction, NULL, UHWT_PRIORITY_DEFAULT)) {
		TEST_FAIL_MESSAGE(maxTimerFail);
	}
	for (uint8_t i = 0; i < UHWT_TIMER_COUNT; i++) {
		if (!cancelHardTimer((uhwt_timer_t)i)) {
			TEST_FAIL_MESSAGE(cancelLoopFail);
		}
		if (uhwtTimerStarted((uhwt_timer_t)i)) {
			TEST_FAIL_MESSAGE(didntStopFail);
		}
	}

	// 0 frequency
	freq = 0;
	if (setHardTimer(&timer, &freq, testTimingFunction, NULL, UHWT_PRIORITY_DEFAULT)) {
		TEST_FAIL_MESSAGE(freq0Fail);
	}
	TIMER_NOT_INVALID(timer);
	if (uhwtTimerStarted(timer)) {
		TEST_FAIL_MESSAGE(isStartFail);
	}
	if (cancelHardTimer(timer)) {
		TEST_FAIL_MESSAGE(cancelFail);
	}
	if (uhwtTimerStarted(timer)) {
		TEST_FAIL_MESSAGE(notStartFail);
	}
	timer = UHWT_TIMER_INVALID;

	// over max frequency
	freq = UHWT_TIMER_FREQ_MAX + 1;
	if (setHardTimer(&timer, &freq, testTimingFunction, NULL, UHWT_PRIORITY_DEFAULT)) {
		TEST_FAIL_MESSAGE(freqMaxFail);
	}
	TIMER_NOT_INVALID(timer);
	if (uhwtTimerStarted(timer)) {
		TEST_FAIL_MESSAGE(isStartFail);
	}
	if (cancelHardTimer(timer)) {
		TEST_FAIL_MESSAGE(cancelFail);
	}
	if (uhwtTimerStarted(timer)) {
		TEST_FAIL_MESSAGE(notStartFail);
	}
	timer = UHWT_TIMER_INVALID;
	TEST_PASS();
}

/**
 * Tests timer priorities are set according
 * to 'universal_hardware_timer.h'. Best Timer ignored
 */
void testTimerPriority() {
	
	resetTimers();

	uhwt_timer_t timer = UHWT_TIMER_INVALID;
	uhwt_timer_t secondTimer = UHWT_TIMER_INVALID;
	uhwt_freq_t freq = TEST_CASES_FREQ;

	// test timer claimed and unstarted
	uhwtClaimTimer(&timer);
	secondTimer = timer;
	if (!setHardTimer(&secondTimer, &freq, testTimingFunction, NULL, UHWT_PRIORITY_DEFAULT)) {
		TEST_FAIL_MESSAGE(pNotStartedClaimedFail);
	}

	if (timer != secondTimer) {
		TEST_FAIL_MESSAGE(pNotStartedClaimedUnequalFail);
	}

	// test timer claimed and started
	freq = TEST_CASES_FREQ;
	if (setHardTimer(&secondTimer, &freq, testTimingFunction, NULL, UHWT_PRIORITY_DEFAULT)) {
		TEST_FAIL_MESSAGE(pStartedClaimedFail);
	}

	if (timer != secondTimer) {
		TEST_FAIL_MESSAGE(pStartedClaimedUnequalFail);
	}

	// test timer unclaimed and started
	uhwtUnclaimTimer(secondTimer);

	if (!setHardTimer(&secondTimer, &freq, testTimingFunction, NULL, UHWT_PRIORITY_DEFAULT)) {
		TEST_FAIL_MESSAGE(pStartedUnclaimedFail);
	}

	if (timer == secondTimer) {
		TEST_FAIL_MESSAGE(pStartedUnclaimedUnequalFail);
	}
	TEST_PASS();
}

/**
 * Tests timing for hardware timer
 * 
 * @param freq target frequency to run at
 * @param buffer amount actual freq can be off
 * @param priority priority to run timer function at
 */
void testTiming(uhwt_freq_t freq, uint8_t buffer, uhwt_priority_t priority) {
	resetTimers();
	uhwt_timer_t functionTimer = UHWT_TIMER_INVALID;

	testGetStartState(functionTimer, false);
	hardTimerCount = 0U;

	uint32_t counter = 1;

	if (!setHardTimer(&functionTimer, &freq, &testTimingFunctionParams, &counter, priority)) {
		TEST_FAIL_MESSAGE(startFail);
	}

	delaySeconds(TEST_DELAY_ELLAPSE_S);

	if (!cancelHardTimer(functionTimer)) {
		TEST_FAIL_MESSAGE(cancelFail);
	}

	TEST_ASSERT_UINT32_WITHIN(buffer, freq, hardTimerCount);
	TEST_PASS();
}

/**
 * Tests slow timing accuracy
 */
void testSlowTiming() {
	testTiming(TEST_SLOW_FREQ, SLOW_TEST_BUFFER, SLOW_TIMER_PRIORITY);
}

/**
 * Tests fast timing accuracy
 */
void testFastTiming() {
	testTiming(TEST_FAST_FREQ, FAST_TEST_BUFFER, FAST_TIMER_PRIORITY);
}

void testUHWT() {
	UHWT_SET_FILE_NAME("hardware_timer_test_priv.c");
	// TODO: esp32 freezes when 'testRepeat' isn't run before timing
	// both v4 and v5
	RUN_TEST(&testProgramStart);
	RUN_TEST(&testRepeat);
	RUN_TEST(&testClaims);
	RUN_TEST(&testStart);
	RUN_TEST(&testTimerPriority);
	RUN_TEST(&testSlowTiming);
	RUN_TEST(&testFastTiming);
	resetTimers();
}

#else

memCharString passTimerIgnore[] = {"Ignoring platform timer tests"};

void passTimers() {
	TEST_IGNORE_MESSAGE(passTimerIgnore);
}

void testUHWT() {
	UHWT_SET_FILE_NAME("hardware_timer_test_priv.c");
	RUN_TEST(&passTimers);
}
#endif
