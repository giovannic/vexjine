/*
 * Timer.cpp
 *
 *  Created on: 28 Jun 2010
 *      Author: nb605
 */

#include "Timer.h"
#include <fstream>

long long Timer::accGlobalVirtualTime;
long long Timer::lastGCtime = 0;
long long Timer::accGCtime = 0;
bool Timer::monitoringTimeProgress = false;

Timer::Timer() {
	// TODO Auto-generated constructor stub

}

Timer::~Timer() {
	 
}

bool Timer::onSystemInit() {
	// any code that should be called for the timer when the system is initialized
	int retval;

	// Initializing PAPI
	if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT) {
		printf("PAPI Library initialization error! \n");
		return false;
	}

	if (PAPI_thread_init(pthread_self) != PAPI_OK) {
		printf("PAPI thread Library initialisation error! \n");
		return false;
	}

	return true;
}

void Timer::onSystemClose() {
   PAPI_shutdown();
}

void Timer::onThreadInit() {
	// any code that should be called for the timer when a thread is initialized
	PAPI_thread_init(gettid);
	PAPI_register_thread();
}

void Timer::onThreadEnd() {
	// any code that should be called for the timer when a thread is terminated
	PAPI_unregister_thread();
}



void Timer::writeLogToFile(const char *file_name) {

	std::filebuf fb;

	fb.open(file_name, std::ios::out);

	std::ostream os(&fb);
/*
    map<unsigned long, string>::iterator iter;
	iter = threadNames.begin();
	while (iter != threadNames.end()) {
		os << iter->first << "," << iter->second <<endl;
		iter++;
	}

	os << endl;
*/
	int length = timeLog.size();
	for (int i =0; i<length; i++) {
		os << *(timeLog[i]);
	}

	fb.close();

}

void Timer::printDelays() {
	#if REMOVE_DELAY_TYPE == BYTECODE_DELAY_TYPE
	fprintf(stderr, "Lost times: per bytecode: %d - per IP: %d\n", bytecodeDelay, interactionPointDelay);
	fflush(stderr);
	#else
	fprintf(stderr, "Lost times: per method: %d - per IP: %d\n", methodDelay, interactionPointDelay);
	fflush(stderr);
	#endif
}
