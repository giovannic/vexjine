/*
 * Timer.h
 *
 *  Created on: 28 Jun 2010
 *      Author: nb605
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "JVMTIUtil.h"
#include <papi.h>
#include <pthread.h>
#include <vector>
#include <iostream>

class ProgressRecord {

public:

	ProgressRecord(const long &_threadId, const int &_methodId, long long &_timeDiff, long long &_timeStamp, long long &_lastCPU, short &_threadPosition, short &_currentState) {
		threadId = _threadId;
		methodId = _methodId;
		timeDiff = _timeDiff;
		timeStamp= _timeStamp;
		threadPosition = _threadPosition;
		currentState = _currentState;
		lastCPU = _lastCPU;
	};

	friend std::ostream & operator <<(std::ostream &outs, const ProgressRecord &record) {

		outs << record.timeStamp << "," << record.threadId << "," << record.methodId << "," << record.timeDiff << "," << record.lastCPU << "," << monitorPositionToString(record.threadPosition) << "," << threadStateToString(record.currentState) << std::endl;

		return outs;
	};

private:
	static const char *monitorPositionToString(int position) {
		switch (position) {
			case 1: return "After entering method";
			case 2: return "Before exiting method";
			case 3: return "In signal handler";
			case 4: return "Updating internal";
			case 5: return "Before suspending";
			default: return "Unknown";
		}
	};
	enum {
		RUNNING = 1,
		WAITING = 2,
		SUSPENDED = 3,
		LEARNING_IO = 4,
		IN_IO = 5,
		ZOMBIE = 6,
		IN_NATIVE = 7,
		WAITING_INTERNAL_SOCKET_READ = 8,
		REGISTERING = 9
	};

	static const char *threadStateToString(int state) {
		switch (state) {
			case RUNNING: 		return "RUNNING";
			case SUSPENDED: 	return "SUSPENDED";
			case WAITING:		return "WAITING";
			case IN_IO: 		return "IN IO";
			case LEARNING_IO: 	return "LEARNING IO";
			case WAITING_INTERNAL_SOCKET_READ: return "WAITING TO READ FROM SOCKET";
			default: return "REGISTERING";
		}
	};

	long threadId;
	int methodId;
	short threadPosition;
	long long timeDiff;
	long long timeStamp;
	long long lastCPU;
	short currentState;
};


class Timer {

public:
	Timer();
	static bool onSystemInit();	// any code that should be called for the timer when the system is initialized
	static void onSystemClose();
	
	static void onThreadInit(); // any code that should be called for the timer when a thread is initialized
	static void onThreadEnd();

	static inline long long getVirtualTime() {
		return PAPI_get_virt_nsec();
	}
	static inline long long getRealTime() {
		return PAPI_get_real_nsec();
	}
	static inline long long forceGetVirtualTime() {
		return PAPI_get_virt_nsec();
	}
	static inline long long forceGetRealTime() {
		return PAPI_get_real_nsec();
	}

	virtual ~Timer();

	static inline long long getGlobalTime() {
		return accGlobalVirtualTime;
	}

	static inline void leapToGlobalTime(const long long & time) {
		accGlobalVirtualTime = time;
	};

	static inline void progressGlobalTimeBy(const long long & time) {
		accGlobalVirtualTime += time;
	};

	static inline bool tryForwardTimeLeap(const long long &time) {
		if (time > accGlobalVirtualTime) {
			accGlobalVirtualTime = time;
			return true;
		} else {
			return false;
		}
	}

	static inline bool monitorTimeProgress() {
		return monitoringTimeProgress;
	}

	static inline void enableTimeProgressMonitoring() {
		monitoringTimeProgress = true;
	};

	static inline void logTimeProgress(long threadId, int methodId, long long timediff, long long lastCPU, short threadPosition, short currentState) {
		timeLog.push_back(new ProgressRecord(threadId, methodId, timediff, accGlobalVirtualTime, lastCPU, threadPosition, currentState));
	};

	/**The current global virtual time.*/
	static long long accGlobalVirtualTime;
	static long long lastGCtime;
	static long long accGCtime;

	/**Time progress monitoring module: flag and data structure */
	static bool monitoringTimeProgress;
	static std::vector<ProgressRecord *> timeLog;
	static void writeLogToFile(const char *file_name);

};

#endif /* TIMER_H_ */
