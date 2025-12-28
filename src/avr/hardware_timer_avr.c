/*
	hardware_timer_avr.c - timer configuration for avr
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

#include "hardware_timer_avr.h"

#if UHWT_SUPPORT_AVR

#include <avr/interrupt.h>
#include <avr/pgmspace.h>

const uhwt_prescalar_t scalarMask[] PROGMEM = {
	0, // SCALAR_0
	1, // SCALAR_1
	8, // SCALAR_8
	32, // SCALAR_32
	64, // SCALAR_64
	128, // SCALAR_128
	256, // SCALAR_256
	1024, // SCALAR_1024
};

#if TIMER_0_ALIAS != UHWT_TIMER_INVALID_LIT
	ISR(TIMER0_COMPA_vect) {
		((void(*)())hardTimerFunctions[TIMER_0_ALIAS])(hardTimerParams[TIMER_0_ALIAS]);
	}
#endif

#if TIMER_1_ALIAS != UHWT_TIMER_INVALID_LIT
	ISR(TIMER1_COMPA_vect) {
		((void(*)())hardTimerFunctions[TIMER_1_ALIAS])(hardTimerParams[TIMER_1_ALIAS]);
	}
#endif

#if TIMER_2_ALIAS != UHWT_TIMER_INVALID_LIT
	ISR(TIMER2_COMPA_vect) {
		((void(*)())hardTimerFunctions[TIMER_2_ALIAS])(hardTimerParams[TIMER_2_ALIAS]);
	}
#endif

/**
 * Gets scalar mask value from enum
 * 
 * @param i scalar enum
 * 
 * @return literal int value
 */
static inline uint16_t getMask(prescalar_enum_t scalar) {
	return ((uint16_t)pgm_read_word_near(scalarMask + scalar));
}

/****************************
 * Universal Hardware Timer Functions
****************************/

uhwt_freq_t uhwtPlatformCalcFreq(uhwt_prescalar_t scalar, uhwt_timertick_t ticks) {
	return F_CPU / ((uhwt_freq_t)getMask(scalar) * (ticks + 1));
}

uhwt_timertick_t uhwtCalcTicks(uhwt_freq_t targetFreq, uhwt_prescalar_t scalar) {
	return (F_CPU / (getMask(scalar) * targetFreq)) - 1;
}

uhwt_prescalar_t uhwtCalcScalar(uhwt_freq_t targetFreq, uhwt_timertick_t ticks) {
	return F_CPU / (targetFreq * (ticks + 1));
}

/****************************
 * Platform Functions
****************************/

/**
 * Inits hardware timer
 * 
 * @param num timer id
 */
#define UHWT_INIT_TIMER(num) \
	cli(); \
	HARD_TIMER_CONCATENATE3(TIMER_, num, _COMP) = 0; \
	HARD_TIMER_CONCATENATE3(TIMER_, num, _WAVEFORM) = 0; \
	HARD_TIMER_CONCATENATE3(TIMER_, num, _COUNTER) = 0; \
	sei()

bool uhwtPlatformInitTimer(uhwt_timer_t timer) {
	switch(timer) {
		#if TIMER_0_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_0_ALIAS):
				UHWT_INIT_TIMER(0);
			break;
		#endif
		#if TIMER_1_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_1_ALIAS):
				UHWT_INIT_TIMER(1);
			break;
		#endif
		#if TIMER_2_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_2_ALIAS):
				UHWT_INIT_TIMER(2);
			break;
		#endif
		default:
			return false;
		break;
	}
	return true;
}

bool uhwtPlatformDeconstructTimer(uhwt_timer_t timer) {
	return true;
}

/**
 * Stops timer
 * 
 * @param num timer id
 */
#define UHWT_STOP_TIMER(num) \
	cli(); \
	HARD_TIMER_CONCATENATE3(TIMER_, num, _INTERR) &= ~HARD_TIMER_CONCATENATE3(TIMER_, num, _INTERR_ENABLE); \
	sei()

bool uhwtPlatformStopTimer(uhwt_timer_t timer) {
	switch(timer) {
		#if TIMER_0_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_0_ALIAS):
				UHWT_STOP_TIMER(0);
			break;
		#endif
		#if TIMER_1_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_1_ALIAS):
				UHWT_STOP_TIMER(1);
			break;
		#endif
		#if TIMER_2_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_2_ALIAS):
				UHWT_STOP_TIMER(2);
			break;
		#endif
		default:
			return false;
		break;
	}
	return true;
}

/**
 * Starts hardware timer
 * 
 * @param num timer id
 */
#define UHWT_START_TIMER(num) \
	cli(); \
	HARD_TIMER_CONCATENATE3(TIMER_, num, _INTERR) |= HARD_TIMER_CONCATENATE3(TIMER_, num, _INTERR_ENABLE); \
	sei()

bool uhwtPlatformStartTimer(uhwt_timer_t timer) {
	switch(timer) {
		#if TIMER_0_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_0_ALIAS):
				UHWT_START_TIMER(0);
			break;
		#endif
		#if TIMER_1_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_1_ALIAS):
				UHWT_START_TIMER(1);
			break;
		#endif
		#if TIMER_2_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_2_ALIAS):
				UHWT_START_TIMER(2);
			break;
		#endif
		default:
			return false;
		break;
	}
	return true;
}

/**
 * Sets timer stats
 * 
 * @param num timer number
 * @param scalar scalar to set
 * @param timerTicks ticks to set
 */
#define UHWT_SET_STATS(num, scalar, timerTicks) \
	cli(); \
	HARD_TIMER_CONCATENATE3(TIMER_, num, _TARGET) = timerTicks; \
	HARD_TIMER_CONCATENATE3(uhwtTimer, num, SetScalar)(scalar); \
	HARD_TIMER_CONCATENATE3(TIMER_, num, _INCR) |= HARD_TIMER_CONCATENATE3(TIMER_, num, _INCREM_ENABLE); \
	sei()

bool uhwtPlatformSetStats(uhwt_timer_t timer, uhwt_prescalar_t scalar, uhwt_timertick_t timerTicks) {
	
	switch(timer) {
		#if TIMER_0_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_0_ALIAS):
				UHWT_SET_STATS(0, scalar, timerTicks);
			break;
		#endif
		#if TIMER_1_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_1_ALIAS):
				UHWT_SET_STATS(1, scalar, timerTicks);
			break;
		#endif
		#if TIMER_2_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_2_ALIAS):
				UHWT_SET_STATS(2, scalar, timerTicks);
			break;
		#endif
		default:
			return false;
		break;
	}
	return true;
}

uhwt_timer_t uwhtPlatformGetNextTimerStats(uhwt_claim_s claimArgs) {

	// checks priorities
	if (claimArgs.slowestTimer) {
		if (!uhwtTimerClaimed(TIMER_1_ALIAS) && !uhwtTimerStarted(TIMER_1_ALIAS)) {
			return TIMER_1_ALIAS;
		}
	}
	if (claimArgs.mostAccurateTimer) {
		if (!uhwtTimerClaimed(TIMER_2_ALIAS) && !uhwtTimerStarted(TIMER_2_ALIAS)) {
			return TIMER_2_ALIAS;
		}
	}

	return UHWT_TIMER_INVALID;
}

bool uhwtPlatformEqualFreq(uhwt_freq_t targetFreq, uhwt_prescalar_t scalar, uhwt_timertick_t ticks) {
	if (F_CPU % ((uhwt_freq_t)getMask(scalar) * (ticks + 1)) != 0) {
		return false;
	}
	return true;
}

uhwt_prescalar_t uhwtGetNextPreScalar(uhwt_prescalar_t prevScalar) {
	switch(prevScalar) {
		case(SCALAR_0):
			return SCALAR_1024;
		break;
		case(SCALAR_1024):
			return SCALAR_256;
		break;
		case(SCALAR_256):
			return SCALAR_128;
		break;
		case(SCALAR_128):
			return SCALAR_64;
		break;
		case(SCALAR_64):
			return SCALAR_32;
		break;
		case(SCALAR_32):
			return SCALAR_8;
		break;
		case(SCALAR_8):
			return SCALAR_1;
		break;
		case(SCALAR_1):
			return SCALAR_0;
		break;
		default:
			return SCALAR_0;
		break;
	};
}

uhwt_prescalar_t uhwtPlatformGetPreScalar(uhwt_timer_t timer) {
	switch(timer) {
		#if TIMER_0_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_0_ALIAS):
				return uhwtTimer0GetScalar();
			break;
		#endif
		#if TIMER_1_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_1_ALIAS):
				return uhwtTimer1GetScalar();
			break;
		#endif
		#if TIMER_2_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_2_ALIAS):
				return uhwtTimer2GetScalar();
			break;
		#endif
		default:
			return 0;
		break;
	}
}

/**
 * Gets timer ticks for timer
 * 
 * @param num timer id
 */
#define UHWT_GET_TICKS(num) ((0xffff) >> (16 - HARD_TIMER_CONCATENATE3(TIMER_, num, _MAX_TICK_BITS))) & HARD_TIMER_CONCATENATE3(TIMER_, num, _TARGET)

uhwt_timertick_t uhwtPlatformGetTimerTicks(uhwt_timer_t timer) {
	switch(timer) {
		#if TIMER_0_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_0_ALIAS):
				return UHWT_GET_TICKS(0);
			break;
		#endif
		#if TIMER_1_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_1_ALIAS):
				return UHWT_GET_TICKS(1);
			break;
		#endif
		#if TIMER_2_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_2_ALIAS):
				return UHWT_GET_TICKS(2);
			break;
		#endif
		default:
			return false;
		break;
	}
}

bool uhwtPlatformValidPreScalar(uhwt_prescalar_t scalar) {
	switch(scalar) {
		#ifdef SCALAR_1_ENABLE
			case SCALAR_1:
				return true;
			break;
		#endif
		#ifdef SCALAR_8_ENABLE
			case SCALAR_8:
				return true;
			break;
		#endif
		#ifdef SCALAR_32_ENABLE
			case SCALAR_32:
				return true;
			break;
		#endif
		#ifdef SCALAR_64_ENABLE
			case SCALAR_64:
				return true;
			break;
		#endif
		#ifdef SCALAR_128_ENABLE
			case SCALAR_128:
				return true;
			break;
		#endif
		#ifdef SCALAR_256_ENABLE
			case SCALAR_256:
				return true;
			break;
		#endif
		#ifdef SCALAR_1024_ENABLE
			case SCALAR_1024:
				return true;
			break;
		#endif
		default:
			return false;
		break;
	}
}

bool uhwtValidTimerPreScalar(uhwt_timer_t timer, uhwt_prescalar_t scalar) {
	if (!uhwtValidTimer(timer)) {
		return false;
	}
	if ((scalar == SCALAR_32 || scalar == SCALAR_128) && timer != TIMER_2_ALIAS) {
		return false;
	}
	return uhwtPlatformValidPreScalar(scalar);
}

bool uhwtValidTimerTicks(uhwt_timer_t timer, uhwt_timertick_t ticks) {
	if (!uhwtPlatformValidTimerTicks(ticks) || !uhwtValidTimer(timer)) {
		return false;
	}
	uhwt_timertick_t maxValue = 0;
	switch(timer) {
		#if TIMER_0_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_0_ALIAS):
				maxValue = TIMER_0_MAX_TICKS;
			break;
		#endif
		#if TIMER_1_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_1_ALIAS):
				maxValue = TIMER_1_MAX_TICKS;
			break;
		#endif
		#if TIMER_2_ALIAS != UHWT_TIMER_INVALID_LIT
			case(TIMER_2_ALIAS):
				maxValue = TIMER_2_MAX_TICKS;
			break;
		#endif
		default:
		break;
	}
	if (ticks > maxValue) {
		return false;
	}
	return true;
}

#endif