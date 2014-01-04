/*
 * Timers.h: Class for the various timers that thread use
 *
 *  Created on: 21 Apr 2011
 *      Author: root
 */

#ifndef TIMERS_H_
#define TIMERS_H_

#include "MethodCallInfo.h"
#include "Time.h"

//#include <cassert>

#define MAXIMUM_CONSECUTIVE_TIMESLICES_AS_BLOCKED_IN_MONITOR 10

class Timers {
public:
	Timers();
	virtual ~Timers();

	static void setGlobalScalingFactor(const float &_globalScalingFactor) {
		if (_globalScalingFactor > 0.0) {
			globalScalingFactor = _globalScalingFactor;
		}
	}

	inline float const & getTimeScalingFactor() const {
		return virtualTimeSpeedup;
	};
	inline void setTimeScalingFactor(const float &factor) {
		virtualTimeSpeedup = factor;
	};

	inline void setSynchronizationBarrierTime(const long long &schedulerTimeslot) {
		estimatedRealTime += schedulerTimeslot;
	};
	inline void clearSynchronizationBarrierTime(const long long &schedulerTimeslot) {
		estimatedRealTime -= schedulerTimeslot;
	};



	inline void updateWaitingTimeFrom(const long long &startingWaiting) {
		long long leap = estimatedRealTime - startingWaiting;
		if (leap > 0) {
			monitorWaitingTime += leap;
		}
	};



	// New deal
	inline void leapForwardTo(const long long &currentTime) {
		if (estimatedRealTime < currentTime) {
			estimatedRealTime = currentTime;
		}
	}

	inline long long const & getEstimatedRealTime() const {
		return estimatedRealTime;
	};
	inline long long const & getLastCPUTime() const {
		return lastCPUTime;
	};


	inline void setEstimatedRealTime(const long long &_estimatedRealTime) {
		estimatedRealTime = _estimatedRealTime;
	};
	inline void setLastCPUTime(const long long &_lastCpuTime) {
		lastCPUTime = _lastCpuTime;
	};
	inline void setVirtualTime(const long long &_virtualTime) {
		virtualTime = _virtualTime;
	};

	inline long long getCurrentCpuTime() const {
		return virtualTime;
	};

	inline long long const getVirtualTime() const {
		#if USE_PAPI == 1
		return PAPI_get_virt_nsec();
		#elif USE_SYS_CLOCK == 1
		struct timespec currentRealTime;
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &currentRealTime );
		return (long long)(currentRealTime.tv_sec * 1000000000 + currentRealTime.tv_nsec);


		#else
		return PERFCTR_getVT_for(threadPerfPtr);
		#endif
	};

	inline void logStartingVirtualTime() {
		virtualTimeAtSimulationStart = virtualTime;		// used for modelling
	};

	void updateCpuTimeAddingSimulatedVirtualTime() {
		lastCPUTime += (virtualTime - virtualTimeAtSimulationStart);
	}
	void updateExitingMethodInfo(const int &methodId, MethodCallInfo *exitingMethod) {
		exitingMethod->setInfo(methodId, virtualTime, estimatedRealTime, monitorWaitingTime, ioWaitingTime);
	}

	inline void setBlockingTimeToLastResumedTime() {
		std::swap(estimatedRealTime, resumedLastAt);
	};
	inline void unsetBlockingTimeToLastResumedTime() {
		std::swap(estimatedRealTime, resumedLastAt);
	};

	inline long long getThreadTimeBeforeMethodInstrumentation() {
		return getVirtualTime() - Time::getBytecodeDelay();
	};

	inline long long getThreadTimeBeforeIoMethodInstrumentation() {
		return getVirtualTime() - Time::getIoBytecodeDelay();
	};



	void updateTimesBeforeWaitingWithTimeout(const long &_timeout);

	inline void setDontUpdateToGlobalTime(const bool &_dontUpdateToGlobalTime) {
		dontUpdateToGlobalTime = _dontUpdateToGlobalTime;
	};

	inline void updateCpuTimeClock() {
		lastCPUTime = getVirtualTime();
	};

	inline long long const & getResumedLastAt() const {
		return resumedLastAt;
	};

	inline void setResumedLastAt(const long long & _resumedLastAt) {
		resumedLastAt = _resumedLastAt;
	};

	inline void setCpuTimeClockAfterIo() {
		lastCPUTime = getVirtualTime() + Time::getIoBytecodeDelay();
	};

	inline long long const & getResumedLastAtReal() const {
		return resumedLastAtReal;
	};
	inline void setLocalTime(const long &newTime) {
		localTimeSinceLastResume = newTime;
	};

	inline void setResumedLastAtReal(const long long & _resumedLastAtReal) {
		resumedLastAtReal = _resumedLastAtReal;
	};
	inline void updateResumedLastRealAt() {
		resumedLastAtReal = Time::getRealTime();
	};

	inline long long const & getLastRealTime() const {
		return lastRealTime;
	};

	inline void setLastRealTime(const long long & _lastRealTime) {
		lastRealTime = _lastRealTime;
	};

	inline void leapToGlobalTime(const long long &currentVirtualTime) {
		if (!dontUpdateToGlobalTime) {
			estimatedRealTime = currentVirtualTime;
		}
	};

	inline bool beforeCurrentGlobalTime(const long long &currentVirtualTime) {
		if (currentVirtualTime > estimatedRealTime) {
			return true;
		} else {
			return false;
		}
	};


	inline long long const & getLocalTimeSinceLastResume() const {
		return localTimeSinceLastResume;
	};

	inline long long const & getLastTimePerStateERT() const {
		return lastTimePerStateERT;
	};
	inline void setLastTimePerStateERT(const long long & _lastTimePerStateERT) {
		lastTimePerStateERT = _lastTimePerStateERT;
	};
	inline long long const & getLastTimeInHandler() const {
		return lastTimeInHandler;
	};


	void rollbackFromTimeOutValue() {
		estimatedRealTime -= timeout;
		timeout = -1;
	};
	inline long const & getTimeout() const {
		return timeout;
	};
	inline void setTimeout(const long & _timeout) {
		timeout = _timeout;
	};

	inline void setTimeoutFlagToDenotePossiblyResumingThread() {
		timeout = 0;
		threadStillBlocked = MAXIMUM_CONSECUTIVE_TIMESLICES_AS_BLOCKED_IN_MONITOR;
	}

	// Method used to support questions on virtual leaps forward in virtual time:
	// The method is only called, when a thread wants to leap forward, but another thread is waiting on a monitor
	// Since we can not be certain that the monitor has been released in the meantime, we need to allow for some
	// real time to pass (MAXIMUM_CONSECUTIVE_TIMESLICES_AS_BLOCKED_IN_MONITOR * SCHEDULER_TIMESLOT) before we denote
	// that the thread is still blocked. In that case the forward leaping thread is allowed to do so.
	inline bool isThreadStillBlockedInMonitor() {
		if (timeout == 0) {
			return (threadStillBlocked-- > 0);	// if a thread is asked and is found to be blocked for a consecutive number of times, then release it
		} else {
			return false;
		}
	}

	bool hasTimeoutExpiredInRealTime(const long long &minimumMaybeAliveTime, const long long &remainingTimeToExpireTimeout);
	inline bool maximumPossibleIoTimeExpired() {
		return getCurrentTimeDifferenceFromLastUpdatedRealTime() > 500000000; // 0.5sec max I/O time
	}

	void updateCurrentLocalTime();


	long long updateClocks();
	void updateRealTimeClock();
	long long addElapsedTime(long long time);
	long long leapForwardBy(long long time);
	long long leapTo(long long time);
	long long getTimeDifferenceUntil(const long long &startingTime);
	long long getAndResetLocalTime();
	long long getLocalTime();

	void updateThreadLocalTimeSinceLastResumeTo(const long long &presetTime);
	void updateThreadLocalTimeSinceLastResumeToRealTime(const long long &presetTime);

	/*
	 * External virtual time progress, f.e. running thread events
	 */
	void addGlobalTime(const long long &timeAdded) {
		addElapsedTime(timeAdded);
		localTimeSinceLastResume += timeAdded;
	}

	/*
	 * Internal running thread events
	 */
	void addLocalTime(const long long &timeDiff) {
		// take into account the speedup
		long long timeAdded = timeDiff * virtualTimeSpeedup  * globalScalingFactor;
		addElapsedTime(timeAdded);
		localTimeSinceLastResume += timeAdded;
	}

	/*
	 * Increase localtime and ERT but not CPU time (used for I/O)
	 */
	void addLocalIoTime(const long long &timeDiff) {
		// take into account the speedup
		long long timeAdded = timeDiff * virtualTimeSpeedup;	// Don't use  globalScalingFactor here
		estimatedRealTime 		 += timeAdded;
		localTimeSinceLastResume += timeAdded;
	}


	void updateLastResumedTo(const long long &realHandlerTime) {
		// Update these
		lastTimeInHandler 		= estimatedRealTime;
		lastRealTimeInHandler 	= realHandlerTime;
	};

	inline long long const & getMonitorWaitingTime() const {
		return monitorWaitingTime;
	};
	inline long long const & getIoWaitingTime() const {
		return ioWaitingTime;
	};

	bool doesTheDifferenceBetweenCpuAndRealTimeIndicateLackOfProgress(const long long &realTime);
	long long getDifferenceBetweenCpuAndRealTime(const long long &realTime);

//
//	bool doesTheDifferenceBetweenCpuAndRealTimeIndicateLackOfProgress(const long long &realTime) {
//		long long cpuTimeDifferenceSinceLastResume = (estimatedRealTime - lastTimeInHandler)/virtualTimeSpeedup; //inaccurate
//		long long realTimeDifferenceSinceLastResume = realTime - lastRealTimeInHandler;
//		return (0.01 * realTimeDifferenceSinceLastResume - cpuTimeDifferenceSinceLastResume) > 0;
//	};
//
//	long long getDifferenceBetweenCpuAndRealTime(const long long &realTime) {
//		long long cpuTimeDifferenceSinceLastResume = (estimatedRealTime - lastTimeInHandler)/virtualTimeSpeedup; //inaccurate
//		long long realTimeDifferenceSinceLastResume = realTime - lastRealTimeInHandler;
//		return 0.01 * realTimeDifferenceSinceLastResume - cpuTimeDifferenceSinceLastResume;
//	};


	inline bool shouldNotUpdateToGlobalTime() {
		return dontUpdateToGlobalTime;
	};
	inline void updateIoWaitingTimeFrom(const long long &startingWaiting, const long long &actualIoDuration) {
		long long leap = estimatedRealTime - startingWaiting;
		if (leap > 0) {
			if (leap > actualIoDuration) {
				ioWaitingTime += actualIoDuration;
//				std::cout << actualIoDuration << " " << leap << " adding actual " << actualIoDuration << std::endl;
			} else {
				ioWaitingTime += leap;
//				std::cout << actualIoDuration << " " << leap << " adding diff_from_GVT " << leap << std::endl;
			}
		}
	};

	inline long long getCurrentVirtualTimestampOfRunningThread() {
		return resumedLastAt + localTimeSinceLastResume;
	}


	inline void updateTimesAfterResuming(const long long &resumingVirtualTime) {

		if (dontUpdateToGlobalTime) {
			resumedLastAt 			= estimatedRealTime;	// used for model-simulation remote timelines
		} else {
			resumedLastAt 			= resumingVirtualTime;
		}
		resumedLastAtReal 		= Time::getRealTime();

		lastTimeInHandler 		= resumedLastAt;
		lastRealTimeInHandler 	= resumedLastAtReal;

		localTimeSinceLastResume = 0;
	}

protected:
	void resetCounters();
	long long getCurrentTimeDifferenceFromLastUpdatedRealTime();



//
//	inline void resetLocalTime() {
//		localTimeSinceLastResume = 0;
//	};
//	inline bool aheadOfCurrentGlobalTime(const long long &currentVirtualTime) {
//		if (currentVirtualTime < estimatedRealTime) {
//			return true;
//		} else {
//			return false;
//		}
//	};
//	inline void setLocalTimeSinceLastResume(const long long & _localTimeSinceLastResume) {
//		localTimeSinceLastResume = _localTimeSinceLastResume;
//	};
//	inline void setLastTimeInHandler(const long long & _lastTimeInHandler) {
//		lastTimeInHandler = _lastTimeInHandler;
//	};
public:
	inline long long const & getLastRealTimeInHandler() const {
		return lastRealTimeInHandler;
	};
protected:
//	inline void setLastRealTimeInHandler(const long long & _lastRealTimeInHandler) {
//		lastRealTimeInHandler = _lastRealTimeInHandler;
//	};
//
//
//




//
//
//	inline void clearTimeScalingFactor() {
//		virtualTimeSpeedup = 1.0;
//	};
//



	struct vperfctr *onThreadInit();
	void onThreadEnd();

	long timeout;
	int threadStillBlocked;	// variable used to denote whether a thread is still blocked at a monitor - used in heuristic for avoiding leaps forward

	long long estimatedRealTime;
	long long lastCPUTime;										//  - 8 bytes
	long long virtualTime; //virtual time allocated to this thread 	- 8 bytes
	float virtualTimeSpeedup;//factor by which virtual time is modified.
	long long localTimeSinceLastResume;

	long long resumedLastAt;	// estimated time when the thread was last resumed
	long long resumedLastAtReal;	// real time when the thread was last resumed
	long long lastTimeInHandler;
	long long lastRealTimeInHandler;

	long long lastRealTime;
	long long lastTimePerStateERT;

	long long virtualTimeAtSimulationStart;

	long long ioWaitingTime;
	long long monitorWaitingTime;

	bool dontUpdateToGlobalTime;

	struct vperfctr *threadPerfPtr;
	static float globalScalingFactor;
};

#endif /* TIMERS_H_ */
