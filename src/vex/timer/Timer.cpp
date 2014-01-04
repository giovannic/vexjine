/*
 * Timer.cpp
 *
 *  Created on: 28 Jun 2010
 *      Author: nb605
 */

#include "Timer.h"

#include <cassert>

Timer::Timer() {
	pthread_spin_init(&spinlock, spinint);
	accGlobalVirtualTime = 0;
	lastGCtime = 0;
	accGCtime = 0;

	totalRemovals = 0;
	totalInstrumentationTime = 0;
}

void Timer::lock() {
	pthread_spin_lock(&spinlock);
}

void Timer::unlock() {
	pthread_spin_unlock(&spinlock);
}

void Timer::printTotalTime() {
	cout << "Total time " << totalInstrumentationTime << endl;
}

Timer::~Timer() {
	pthread_spin_destroy(&spinlock);
}


void Timer::reset() {
	setGlobalTimeTo(0);
}


void Timer::onGarbageCollectionStart() {
	Timer::lastGCtime = Timer::getRealTime();
}

long long Timer::getGarbageCollectionRealDuration() {
	long long gcTime = Timer::getRealTime() - Timer::lastGCtime;
	Timer::progressGlobalTimeBy(gcTime);
	return gcTime;

}

void Timer::printTotalTime() {
	cout << "Total time " << totalInstrumentationTime << endl;
}

void Timer::printStats(char *file_name) {
	if (file_name == NULL) {
		std::cout << timestats;
	} else {
		std::filebuf fb;
		fb.open(file_name, std::ios::out);
		std::ostream os(&fb);
		fb.close();
	}
}
