/*
 * Timers.cpp
 *
 *  Created on: 21 Apr 2011
 *      Author: root
 */

#include "Timers.h"

#include <cmath>
#include <climits>

using namespace std;

Timers::Timers() {
	resetCounters();
	lastCPUTime = 0;
	threadPerfPtr = onThreadInit();	// The thread CPU-time measurement starts from this point
}

Timers::~Timers() {
	threadPerfPtr = NULL;
	onThreadEnd();
}

float Timers::globalScalingFactor = 1.0;

struct vperfctr *Timers::onThreadInit() {
	// any code that should be called for the timer when a thread is initialized
#if USE_PAPI == 1
	PAPI_register_thread();
	return NULL;
#elif USE_SYS_CLOCK == 1
	return NULL;

#elif USE_PERFCTR == 1
	return PERFCTR_register_thread();
#endif

}

void Timers::onThreadEnd() {
	// any code that should be called for the timer when a thread is terminated
#if USE_PAPI == 1
	PAPI_unregister_thread();
#elif USE_SYS_CLOCK == 1

#elif USE_PERFCTR == 1
	PERFCTR_unregister_thread();
#endif

}

long long Timers::updateClocks() {
	lastCPUTime = getVirtualTime();
	lastRealTime = Time::getRealTime();
	return lastCPUTime;
}

void Timers::updateRealTimeClock() {
	lastRealTime = Time::getRealTime();
}

long long Timers::leapForwardBy(long long time) {
	estimatedRealTime += time;
	return estimatedRealTime;
}

long long Timers::leapTo(long long time) {
	if (estimatedRealTime < time) {
		estimatedRealTime = time;
	}
	return estimatedRealTime;
}

long long Timers::getAndResetLocalTime() {
	long long temp = localTimeSinceLastResume;
	localTimeSinceLastResume = 0;
	return temp;
}

long long Timers::getLocalTime() {
	return localTimeSinceLastResume;
}

void Timers::updateThreadLocalTimeSinceLastResumeTo(const long long &presetTime) {
	long long timeDiff = presetTime - lastCPUTime;// - lost_time;
	// Check whether the Virtual Time has progressed
	if (timeDiff > 0) {
		addLocalTime(timeDiff);
	}
}

void Timers::updateThreadLocalTimeSinceLastResumeToRealTime(const long long &presetTime) {
	long long timeDiff = presetTime - lastRealTime;// - lost_time;
	// Check whether the Virtual Time has progressed
	if (timeDiff > 0) {
		addLocalTime(timeDiff);
	}
}


void Timers::updateTimesBeforeWaitingWithTimeout(const long &_timeout) {
	timeout 	  = _timeout;
	// this changes the estimatedRealTime
	if (_timeout == 0) {
		threadStillBlocked = MAXIMUM_CONSECUTIVE_TIMESLICES_AS_BLOCKED_IN_MONITOR;
	} else {
		if (timeout >= (LONG_MAX - estimatedRealTime)/(virtualTimeSpeedup * globalScalingFactor)) {
			estimatedRealTime = LONG_MAX;

		} else {
			estimatedRealTime += (timeout * virtualTimeSpeedup * globalScalingFactor);

		}
	}
}

void Timers::updateCurrentLocalTime() {
	updateThreadLocalTimeSinceLastResumeTo(getVirtualTime());
}


// This function adds a virtual duration to the thread virtual and estimated real times
// Note that any (virtual) modification to the elapsed time must have already been done on the "time" variable
long long Timers::addElapsedTime(long long time) {
	virtualTime 		+= time;
	estimatedRealTime 	+= time;
	return virtualTime;
}


long long Timers::getTimeDifferenceUntil(const long long &startingTime) {
	return (startingTime - lastCPUTime) * virtualTimeSpeedup * globalScalingFactor + getAndResetLocalTime();
}

/*
 * Heuristic method based on the VT difference between the last possibly running thread and the real time expired
 */
bool Timers::hasTimeoutExpiredInRealTime(const long long &minimumMaybeAliveTime, const long long &remainingTimeToExpireTimeout) {

	if (timeout <= 0) {
		return true;
	}

	long long difference = estimatedRealTime - minimumMaybeAliveTime;

	if (difference < 0) {
		return true;	// minimumMaybeAliveTime is after estimatedRealTime
	} else {
		double factor = 1.0;

		// TODO: this is a hack to avoid huge leaps
		if (difference > 100000000) {
			//factor = (double)log10(difference) - 7.0;
			factor = 100*(double)log10(difference) - 7.0;
		}

//		long long realTimeDifference = getCurrentTimeDifferenceFromLastUpdatedRealTime();
		//cout << realTimeDifference/1e6 << " - (" << factor << " * " << remainingTimeToExpireTimeout/1e6 << ") = " << (realTimeDifference - (factor*remainingTimeToExpireTimeout))/1e6 << endl;
		if (getCurrentTimeDifferenceFromLastUpdatedRealTime() > (double)(factor*remainingTimeToExpireTimeout)) {
			return true;
		} else {
			return false;
		}
	}
}


long long Timers::getCurrentTimeDifferenceFromLastUpdatedRealTime() {
	return Time::getRealTime() - lastRealTime;
}



void Timers::resetCounters() {

	timeout = -1;			// so that the scheduler will disregard it

	virtualTime = 0;

	resumedLastAt		= 0;
	resumedLastAtReal 	= 0;

	estimatedRealTime 	= 0;
	virtualTimeSpeedup 	= 1.0;			// is one and multiplicated/divided on methodEnter/methodExit

	lastRealTime 		= Time::getRealTime();
	resumedLastAtReal 	= lastRealTime;

	lastTimePerStateERT 	 = 0;
	localTimeSinceLastResume = 0;

	lastTimeInHandler 		= estimatedRealTime;	// used for the native waiting heuristics
	lastRealTimeInHandler 	= lastRealTime;

	dontUpdateToGlobalTime  = false;

	ioWaitingTime 		= 0;
	monitorWaitingTime 	= 0;

}


bool Timers::doesTheDifferenceBetweenCpuAndRealTimeIndicateLackOfProgress(const long long &realTime) {
	long long cpuTimeDifferenceSinceLastResume = (estimatedRealTime - lastTimeInHandler)/(virtualTimeSpeedup * globalScalingFactor); //inaccurate
	long long realTimeDifferenceSinceLastResume = realTime - lastRealTimeInHandler;
	return cpuTimeDifferenceSinceLastResume < (0.01 * realTimeDifferenceSinceLastResume);
}

long long Timers::getDifferenceBetweenCpuAndRealTime(const long long &realTime) {
	long long cpuTimeDifferenceSinceLastResume = (estimatedRealTime - lastTimeInHandler)/(virtualTimeSpeedup * globalScalingFactor); //inaccurate
	long long realTimeDifferenceSinceLastResume = realTime - lastRealTimeInHandler;
	return 0.01 * realTimeDifferenceSinceLastResume - cpuTimeDifferenceSinceLastResume;
}
