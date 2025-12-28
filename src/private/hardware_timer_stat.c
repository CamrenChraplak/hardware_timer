/*
	hardware_timer_stat.c - hardware timer stat functionality
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

#include "hardware_timer_priv.h"

// stores status of all hardware timers
uhwt_stat_s uhwtStats;

bool uhwtTimerClaimed(uhwt_timer_t timer) {
	if (!uhwtValidTimer(timer)) {
		return false;
	}
	return !!(uhwtStats.uhwtClaimed & (((uhwt_stat_t)1) << (timer)));
}

bool uhwtTimerStarted(uhwt_timer_t timer) {
	if (!uhwtValidTimer(timer)) {
		return false;
	}
	return !!(uhwtStats.uhwtStarted & (((uhwt_stat_t)1) << (timer)));
}

bool uhwtTimerInitialized(uhwt_timer_t timer) {
	if (!uhwtValidTimer(timer)) {
		return false;
	}
	return !!(uhwtStats.uhwtInitialized & (((uhwt_stat_t)1) << (timer)));
}

uhwt_prescalar_t uhwtGetPreScalar(uhwt_timer_t timer) {
	if (!uhwtValidTimer(timer)) {
		return 0;
	}
	return uhwtPlatformGetPreScalar(timer);
}

uhwt_timertick_t uhwtGetTimerTicks(uhwt_timer_t timer) {
	if (!uhwtValidTimer(timer)) {
		return 0;
	}
	return uhwtPlatformGetTimerTicks(timer);
}

bool uhwtClaimTimer(uhwt_timer_t *timer) {

	if (timer == NULL) {
		return false;
	}

	if (uhwtValidTimer(*timer)) {
		if (!uhwtTimerClaimed(*timer) && !uhwtTimerStarted(*timer)) {
			uhwtStats.uhwtClaimed |= (1 << (*timer));
			return true;
		}
	}
	else {
		*timer = uwhtGetNextTimer();
		if (*timer != UHWT_TIMER_INVALID) {
			uhwtStats.uhwtClaimed |= (1 << (*timer));
			return true;
		}
	}
	
	return false;
}

bool uhwtClaimTimerStats(uhwt_timer_t *timer, uhwt_claim_s claimArgs) {

	if (timer == NULL) {
		return false;
	}

	if (!uhwtValidTimer(*timer)) {
		*timer = uwhtPlatformGetNextTimerStats(claimArgs);
		if (uhwtClaimTimer(timer)) {
			return true;
		}
		*timer = UHWT_TIMER_INVALID;
	}

	return uhwtClaimTimer(timer);
}

bool uhwtUnclaimTimer(uhwt_timer_t timer) {
	if (uhwtTimerClaimed(timer)) {
		uhwtStats.uhwtClaimed &= (~(1 << (timer)));
		return true;
	}
	return false;
}

bool uhwtSetStats(uhwt_timer_t timer, uhwt_prescalar_t scalar, uhwt_timertick_t timerTicks) {
	if (!uhwtTimerStarted(timer) && uhwtTimerInitialized(timer) && uhwtValidTimer(timer) &&
			uhwtValidTimerPreScalar(timer, scalar) &&
			uhwtValidTimerTicks(timer, timerTicks)) {

		return uhwtPlatformSetStats(timer, scalar, timerTicks);
	}
	return false;
}

uhwt_freq_t uhwtCalcFreq(uhwt_prescalar_t scalar, uhwt_timertick_t ticks) {
	if (!uhwtPlatformValidPreScalar(scalar) || !uhwtPlatformValidTimerTicks(ticks)) {
		return 0;
	}
	return uhwtPlatformCalcFreq(scalar, ticks);
}

/**
 * Gets difference between given frequencies
 * 
 * @param freq1 first frequency to test
 * @param freq2 second frequency to test
 * 
 * @result difference between frequencies
 */
static inline uhwt_freq_t uhwtFreqDelta(uhwt_freq_t freq1, uhwt_freq_t freq2) {
	if (freq1 > freq2) {
		return freq1 - freq2;
	}
	return freq2 - freq1;
}

/**
 * Calculates closest stat for given timer
 * 
 * @param timer timer to calculate for
 * @param targetFreq target frequency to achieve
 * @param scalar pointer to pre scalar to set
 * @param timerTicks pointer to timer ticks to set
 * 
 * @return calculated frequency
 */
uhwt_freq_t uhwtGetClosestFreq(uhwt_timer_t timer, uhwt_freq_t targetFreq, uhwt_prescalar_t *scalar, uhwt_timertick_t *timerTicks) {

	// get tick of 1 and corresponding scalar
	*timerTicks = 1;
	if (uhwtValidTimerTicks(timer, *timerTicks)) {
		*scalar = uhwtCalcScalar(targetFreq, *timerTicks);

		// test if tick of 1 works
		if (uhwtValidTimerPreScalar(timer, *scalar)) {
			if (uhwtEqualFreq(targetFreq, *scalar, *timerTicks)) {
				return targetFreq;
			}
		}
	}

	*timerTicks = 0;
	*scalar = 0;

	uhwt_freq_t closestFreq = 0;
	uhwt_prescalar_t tempScalar = uhwtGetNextPreScalar(0);

	while (tempScalar != 0) {
		// ignore invalid scalars
		if (!uhwtValidTimerPreScalar(timer, tempScalar)) {
			tempScalar = uhwtGetNextPreScalar(tempScalar);
			continue;
		}

		// ignore invalid ticks
		uhwt_timertick_t calcTicks = uhwtCalcTicks(targetFreq, tempScalar);
		if (!uhwtValidTimerTicks(timer, calcTicks)) {
			tempScalar = uhwtGetNextPreScalar(tempScalar);
			continue;
		}

		uhwt_freq_t tempFreq = uhwtCalcFreq(tempScalar, calcTicks);

		// test if newly calculated frequency is closer
		if (uhwtFreqDelta(targetFreq, closestFreq) > uhwtFreqDelta(targetFreq, tempFreq)) {
			*scalar = tempScalar;
			*timerTicks = calcTicks;
			closestFreq = tempFreq;

			// frequency is exact value
			if (uhwtEqualFreq(targetFreq, tempScalar, calcTicks)) {
				break;
			}
		}

		tempScalar = uhwtGetNextPreScalar(tempScalar);
	}

	if (*timerTicks == 0 || *scalar == 0) {
		return 0;
	}

	return closestFreq;
}

bool uhwtGetStats(uhwt_timer_t *timer, uhwt_freq_t targetFreq, uhwt_prescalar_t *scalar, uhwt_timertick_t *timerTicks) {
	if (timer == NULL || scalar == NULL || timerTicks == NULL) {
		return false;
	}
	if (!uhwtValidFrequency(targetFreq)) {
		return false;
	}
	if (uhwtTimerClaimed(*timer) && uhwtTimerStarted(*timer)) {
		return false;
	}

	// gets next free timer if one isn't selected
	if ((!uhwtTimerClaimed(*timer) && uhwtTimerStarted(*timer)) || !uhwtValidTimer(*timer)) {
		*timer = uwhtGetNextTimer();
	}
	if (!uhwtValidTimer(*timer)) {
		return false;
	}

	// gets closest frequency
	uhwtGetClosestFreq(*timer, targetFreq, scalar, timerTicks);

	if (!uhwtValidTimerPreScalar(*timer, *scalar) || !uhwtValidTimerTicks(*timer, *timerTicks)) {
		return false;
	}

	return true;
}

bool uhwtGetClosestStats(uhwt_timer_t *timer, uhwt_freq_t targetFreq,
		uhwt_prescalar_t *scalar, uhwt_timertick_t *timerTicks) {

	#ifndef UHWT_CONFIGS_NOT_EQUAL
		return uhwtGetStats(timer, targetFreq, scalar, timerTicks);
	#else

		if (timer == NULL || scalar == NULL || timerTicks == NULL) {
			return false;
		}
		if (!uhwtValidFrequency(targetFreq)) {
			return false;
		}

		uhwt_freq_t closestFreq = 0;
		*scalar = 0;
		*timerTicks = 0;
		uhwt_timer_t givenTimer = *timer;
		*timer = UHWT_TIMER_INVALID;

		for (uint8_t i = 0; i < UHWT_TIMER_COUNT; i++) {
			uhwt_prescalar_t tempScalar;
			uhwt_timertick_t tempTicks;

			if ((uhwtTimerClaimed(i) && i != givenTimer) || uhwtTimerStarted(i)) {
				continue;
			}

			// gets closest frequency
			uhwt_freq_t calcFreq = uhwtGetClosestFreq(i, targetFreq, &tempScalar, &tempTicks);

			// test if newly calculated frequency is closer
			if (uhwtFreqDelta(targetFreq, closestFreq) > uhwtFreqDelta(targetFreq, calcFreq)) {
				*scalar = tempScalar;
				*timerTicks = tempTicks;
				*timer = i;

				// frequency is exact value
				if (uhwtEqualFreq(targetFreq, *scalar, *timerTicks)) {
					break;
				}
				closestFreq = calcFreq;
			}
		}

		if (!uhwtValidTimerPreScalar(*timer, *scalar) || !uhwtValidTimerTicks(*timer, *timerTicks)) {
			return false;
		}

		return true;

	#endif
}

uhwt_timer_t uwhtPlatformGetNextTimerStats(uhwt_claim_s claimArgs) __attribute__((weak));
uhwt_timer_t uwhtPlatformGetNextTimerStats(uhwt_claim_s claimArgs) {
	return UHWT_TIMER_INVALID;
}

bool uhwtPlatformValidPreScalar(uhwt_prescalar_t scalar) __attribute__((weak));
bool uhwtPlatformValidPreScalar(uhwt_prescalar_t scalar) {
	if (scalar == 0) {
		return false;
	}
	return true;
}

bool uhwtPlatformValidTimerTicks(uhwt_timertick_t ticks) __attribute__((weak));
bool uhwtPlatformValidTimerTicks(uhwt_timertick_t ticks) {
	if (ticks == 0) {
		return false;
	}
	return true;
}

bool uhwtValidTimerPreScalar(uhwt_timer_t timer, uhwt_prescalar_t scalar) __attribute__((weak));
bool uhwtValidTimerPreScalar(uhwt_timer_t timer, uhwt_prescalar_t scalar) {
	if (!uhwtValidTimer(timer)) {
		return false;
	}
	return uhwtPlatformValidPreScalar(scalar);
}

bool uhwtValidTimerTicks(uhwt_timer_t timer, uhwt_timertick_t ticks) __attribute__((weak));
bool uhwtValidTimerTicks(uhwt_timer_t timer, uhwt_timertick_t ticks) {
	if (!uhwtValidTimer(timer)) {
		return false;
	}
	return uhwtPlatformValidTimerTicks(ticks);
}