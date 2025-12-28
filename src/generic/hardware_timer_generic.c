/*
	hardware_timer_generic.c - timer configuration for PlatformIO native and generic timer
	and unsupported platforms
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

#if !UHWT_SUPPORT || UHWT_SUPPORT_NATIVE

uhwt_freq_t uhwtPlatformCalcFreq(uhwt_prescalar_t scalar, uhwt_timertick_t ticks) {
	return 1;
}

uhwt_timertick_t uhwtCalcTicks(uhwt_freq_t targetFreq, uhwt_prescalar_t scalar) {
	return 1;
}

uhwt_prescalar_t uhwtCalcScalar(uhwt_freq_t targetFreq, uhwt_timertick_t ticks) {
	return 1;
}

bool uhwtPlatformInitTimer(uhwt_timer_t timer) {
	return true;
}

bool uhwtPlatformDeconstructTimer(uhwt_timer_t timer) {
	return true;
}

bool uhwtPlatformStopTimer(uhwt_timer_t timer) {
	return true;
}

bool uhwtPlatformStartTimer(uhwt_timer_t timer) {
	return true;
}

bool uhwtPlatformSetStats(uhwt_timer_t timer, uhwt_prescalar_t scalar, uhwt_timertick_t timerTicks) {
	return true;
}

uhwt_prescalar_t uhwtGetNextPreScalar(uhwt_prescalar_t prevScalar) {
	if (prevScalar == 0) {
		return 1;
	}
	return 0;
}

uhwt_prescalar_t uhwtPlatformGetPreScalar(uhwt_timer_t timer) {
	return 1;
}

uhwt_timertick_t uhwtPlatformGetTimerTicks(uhwt_timer_t timer) {
	return 1;
}

bool uhwtPlatformEqualFreq(uhwt_freq_t targetFreq, uhwt_prescalar_t scalar, uhwt_timertick_t ticks) {
	return true;
}

#endif