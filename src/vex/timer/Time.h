/*
 * Time.h: Class to return real/virtual timestamps
 *
 *  Created on: 1 Sep 2011
 *      Author: root
 */

#ifndef TIME_H_
#define TIME_H_

#include <ostream>

#ifndef USE_PAPI
#ifndef USE_PERFCTR
#define USE_SYS_CLOCK 1
#else
#define USE_PERFCTR 1
#endif
#else
#define USE_PAPI 1
#endif

#ifdef USE_PAPI
#include <papi.h>
#elif USE_SYS_CLOCK == 1
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#elif USE_PERFCTR == 1
extern "C" {
#include "perfdirect.h"
}
#endif

class Time {
private:
	Time();

public:
	static bool onSystemInit();	// any code that should be called for the timer when the system is initialized
	static void onSystemClose();
	static void onThreadInit();

	static bool usingPapiTimer();
	static inline long long getVirtualTime() {
		#if USE_PAPI == 1
		return PAPI_get_virt_nsec();

		#elif USE_SYS_CLOCK == 1
		struct timespec currentCpuTime;
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &currentCpuTime );
		return (long long)(currentCpuTime.tv_sec * 1000000000 + currentCpuTime.tv_nsec);

		#elif USE_PERFCTR == 1
		return PERFCTR_getVT();
		#endif

	}
	static inline long long getRealTime() {
		#if USE_PAPI == 1
		return PAPI_get_real_nsec();

		#elif USE_SYS_CLOCK == 1
		struct timespec currentRealTime;
		clock_gettime(CLOCK_REALTIME, &currentRealTime );
		return (long long)(currentRealTime.tv_sec * 1000000000 + currentRealTime.tv_nsec);

		#elif USE_PERFCTR == 1
		return PERFCTR_getRT();
		#endif
	}

	static inline long long getBytecodeDelay() {
		return bytecodeDelay;

	};

	static inline long long getIoBytecodeDelay() {
		return ioBytecodeDelay;
	};

	static inline long long getRealTimeBeforeMethodInstrumentation() {
		return getRealTime() - ioBytecodeDelay;
	};

	static inline long long getThreadTimeBeforeInteractionPoint() {
		return getVirtualTime() - interactionPointDelay;
	};

	static void writeDelaysToFile(const char *filename);
	static bool parseDelaysFile(const char *filename);

	static inline void setDelays(long methodDelay, long ioMethod, long ipDelay) {
		bytecodeDelay += methodDelay;
		ioBytecodeDelay += ioMethod;
		interactionPointDelay += ipDelay;
	}

	static void profileTimers();
	static int measureYieldDuration();			// profile duration of sched_yield
	static long long profileDelayPerMeasurement();

	static void printDelays();
	static void printDelaysToFile(std::ofstream &myfile);
	static void concisePrinting(std::ostream &out, const long long &timeValue, bool htmlEncoding);

	// The next calls are used to measure the duration of background load executing in real time - though generic as this can be the motivation was to trap GC in Java
	static void onBackgroundLoadExecutionStart();
	static long long getBackgroundLoadExecutionRealDuration();

	static int getYieldDuration() {
		return yieldDuration;
	}
protected:

	static int bytecodeDelay;
	static int ioBytecodeDelay;
	static int interactionPointDelay;
	static int yieldDuration;

	static long long lastBackgroundLoadExecutionTime;
	static long long totalBackgroundLoadExecutionTime;
	static long long totalCollections;

};

#endif /* TIME_H_ */
