/*
	hardware_timer_test_priv.h - methods to test timer methods
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

#ifndef HARDWARE_TIMER_TEST_PRIV_H
#define HARDWARE_TIMER_TEST_PRIV_H

#include "../universal_hardware_timer_test.h"
#include "../private/hardware_timer_priv.h"

typedef const char memCharString;

// use Unity
#if defined(PIO_UNIT_TESTING) || defined(ESP_IDF_UNIT_TESTING) || defined(PICO_SDK_UNIT_TESTING)
	#include <unity.h>
	#define USE_UNITY // use Unity for testing
	#define UHWT_SET_FILE_NAME(fileName) UnitySetTestFile(fileName) // sets file name for testing
// use Serial or printf
#else
	typedef void (*test_func_t)(void);

	#ifdef __cplusplus
	extern "C" {
	#endif

	/**
	 * Sets up test to run
	 * 
	 * @param funcName function name to display
	 * @param funcLine file line number relating to start of test
	 */
	void runTest(const char* funcName, const int funcLine);

	/**
	 * Prints message and exits current test
	 * 
	 * @param message message to display
	 * @param funcLine file line number relating to status location
	 * @param messType message type to display
	 */
	void printMessageType(memCharString *message, const int funcLine, int messType);

	/**
	 * Tests if timer count is within range
	 * 
	 * @param buffer amount real count can be off
	 * @param targetCount target count for testing
	 * @param realCount actual count recieved
	 * 
	 * @return if timer count was successful
	 */
	bool timerCountWithin(uint32_t buffer, uint32_t targetCount, uint32_t realCount, const int funcLine);

	#ifdef __cplusplus
	}
	#endif

	#define TEST_FAILED 0
	#define TEST_IGNORED 1
	#define TEST_PASSED 2

	#define RUN_TEST(function) runTest(#function, __LINE__); (function)()

	#define TEST_IGNORE_MESSAGE(message) printMessageType(message, __LINE__, TEST_IGNORED); return
	#define TEST_IGNORE() printMessageType("IGNORE", __LINE__, TEST_IGNORED); return
	#define TEST_FAIL_MESSAGE(message) printMessageType(message, __LINE__, TEST_FAILED); return
	#define TEST_FAIL() printMessageType("FAIL", __LINE__, TEST_IGNORED); return
	#define TEST_PASS() printMessageType("", __LINE__, TEST_PASSED); return

	#define TEST_ASSERT_UINT32_WITHIN(buffer, targetCount, realCount) if (!timerCountWithin(buffer, targetCount, realCount, __LINE__)) {return;}
#endif

#ifndef UHWT_SET_FILE_NAME
	#define UHWT_SET_FILE_NAME(fileName) // sets file name for testing
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Delays operation for set amount of seconds
 * 
 * @param seconds seconds to delay
 */
void delaySeconds(uint8_t seconds);

#ifdef __cplusplus
}
#endif

#endif