/*
 * ThreadManagerRegistry.cpp
 *
 *  Created on: 31 Mar 2011
 *      Author: root
 */

#include "ThreadManagerRegistry.h"
#include "ThreadManager.h"
#include "ThreadState.h"


#include <cassert>

ThreadManagerRegistry::ThreadManagerRegistry(const unsigned int &_maximumCpus) {
	maximumCpus = _maximumCpus;
	managerList = new ThreadManager*[maximumCpus];
	currentlyRegistered = 0;
	pthread_spin_init(&spinlock, 0);
	nextCpuToStartThread = 0;
}

bool ThreadManagerRegistry::addThreadManager(ThreadManager *manager) {
	if (manager != NULL && currentlyRegistered < maximumCpus) {
		managerList[currentlyRegistered++] = manager;
		return true;
	}
	return false;
}


/***
 * Method returning a pointer to the current controller of a thread. The whole pointer-to-pointer approach
 * has the following reasoning: if thread T is controlled by M1, then if we return a pointer to M1 (and not a pointer to
 * the pointer to M1), then if after the return, M1 signals-suspends T and T is resumed by another controller M2, then the
 * following operations on the originally returned pointer will take place on M1 and not M2, which is the correct current pointer
 * of thread T.
 */
ThreadManager **ThreadManagerRegistry::getCurrentThreadManagerOf(VexThreadState *state) {

	if (state != NULL) {
		return state->getThreadCurrentlyControllingManagerPtr();
	}
	return NULL;
}


ThreadManager *ThreadManagerRegistry::getCurrentThreadManager() {
	cout << "Calling ThreadManagerRegistry get current thread manager" << endl;
	assert(false);
//	VexThreadState *state = VexThreadState::getCurrentThreadState();
//	if (state != NULL) {
//		unsigned int currentManagerId = state->getControllingManagerId();
//		if (currentManagerId < currentlyRegistered) {
//			return managerList[currentManagerId];
//		}
//	}
	return NULL;
}

ThreadManager *ThreadManagerRegistry::getDefaultManager() {
	if (currentlyRegistered != 0) {
		return managerList[0];
	}
	return NULL;
}

void ThreadManagerRegistry::end() {
	for (unsigned int i = 0; i<currentlyRegistered; ++i) {
		managerList[i]->end();
	}
}

void ThreadManagerRegistry::resetDefaultSchedulerTimeslot() {
	for (unsigned int i = 0; i<currentlyRegistered; ++i) {
		managerList[i]->resetDefaultSchedulerTimeslot();
	}
}

void ThreadManagerRegistry::setLog(Log *logger) {
	for (unsigned int i = 0; i<currentlyRegistered; ++i) {
		managerList[i]->setLog(logger);
	}
}

void ThreadManagerRegistry::setDefaultSchedulerTimeslot(const long long &timeslot) {
	for (unsigned int i = 0; i<currentlyRegistered; ++i) {
		managerList[i]->setDefaultSchedulerTimeslot(timeslot);
	}
}



void ThreadManagerRegistry::enableSchedulerStats() {
	for (unsigned int i = 0; i<currentlyRegistered; ++i) {
		managerList[i]->enableSchedulerStats();
	}
}

bool ThreadManagerRegistry::holdsAnyKeys(VexThreadState *state) {
	int threadId = state->getId();
	for (unsigned int i = 0; i<currentlyRegistered; ++i) {
		if (managerList[i]->keyHeldBy(threadId)) {
			return true;
		}
	}
	return false;
}


void ThreadManagerRegistry::setVisualizer(Visualizer *visualizer) {
	for (unsigned int i = 0; i<currentlyRegistered; ++i) {
		managerList[i]->setVisualizer(visualizer);
	}
}

unsigned long ThreadManagerRegistry::getTotalPosixSignalsSent() {
	unsigned long sum = 0;
	for (unsigned int i = 0; i<currentlyRegistered; ++i) {
		sum += managerList[i]->getPosixSignalsSent();
	}
	return sum;
}

void ThreadManagerRegistry::printRunningThread() {
	for (unsigned int i = 0; i<currentlyRegistered; ++i) {
		managerList[i]->printRunningThreadWithCore();
	}
}
void ThreadManagerRegistry::onThreadSpawn(VexThreadState * state) {
//	ThreadManager **manager = getCurrentThreadManagerOf(state);
//	if (manager != NULL) {
//		(*manager)->onThreadSpawn(state);
//	} else {

	unsigned int temp = 0;
	// Load balancing thread entries - particularly useful in cases like parallelizable loops that may spawn threads and expect them to work
	// immediately on a virtual core
	// Keep in mind that locking and unlocking here is allowed because the thread is not registered yet,
	// to be signalled and suspended by the VEX scheduler while holding the lock
	pthread_spin_lock(&spinlock);
	temp = nextCpuToStartThread++;
	if (nextCpuToStartThread == currentlyRegistered) {
		nextCpuToStartThread = 0;
	}
	pthread_spin_unlock(&spinlock);

	managerList[temp]->onThreadSpawn(state);

//	}
}
//
//void ThreadManagerRegistry::onThreadWaitingStart(const long long &startingTime, VexThreadState *state) {
//	ThreadManager **manager = getCurrentThreadManagerOf(state);
//	if (manager != NULL) {
//		(*manager)->onThreadWaitingStart(startingTime, state);
//	} else {
//		state->updateCpuTimeClock();
//	}
//}
//
//void ThreadManagerRegistry::onThreadWaitingStart(VexThreadState * state) {
//	ThreadManager **manager = getCurrentThreadManagerOf(state);
//	if (manager != NULL) {
//		(*manager)->onThreadWaitingStart(state);
//	}
//}
//
//void ThreadManagerRegistry::onThreadWaitingEnd(VexThreadState *state) {
//	ThreadManager **manager = getCurrentThreadManagerOf(state);
//	if (manager != NULL) {
//		(*manager)->onThreadWaitingEnd(state);
//	} else if (state != NULL) {
//		state->updateCpuTimeClock();
//	}
//}
//
//
//void ThreadManagerRegistry::onThreadTimedWaitingStart(VexThreadState * state, long &timeout) {
//	ThreadManager **manager = getCurrentThreadManagerOf(state);
//	if (manager != NULL) {
//		(*manager)->onThreadTimedWaitingStart(state, timeout);
//	}
//}
//
//void ThreadManagerRegistry::onThreadTimedWaitingEnd(VexThreadState * state, const long  & interruptTime) {
//	ThreadManager **manager = getCurrentThreadManagerOf(state);
//	if (manager != NULL) {
//		(*manager)->onThreadTimedWaitingEnd(state, interruptTime);
//	} else {
//		state->updateCpuTimeClock();
//	}
//}
//
//void ThreadManagerRegistry::onThreadYield(VexThreadState * state, const long long & startingTime) {
//	ThreadManager **manager = getCurrentThreadManagerOf(state);
//	if (manager != NULL) {
//		(*manager)->onThreadYield(state, startingTime);
//	} else {
//		state->updateCpuTimeClock();
//	}
//}
//
//void ThreadManagerRegistry::onThreadEnd(VexThreadState * state) {
//	ThreadManager **manager = getCurrentThreadManagerOf(state);
//	if (manager != NULL) {
//		(*manager)->onThreadEnd(state);
//	} else {
//		getDefaultManager()->onThreadEnd(state);
//	}
//}
//
//void ThreadManagerRegistry::onThreadInteractionPointEncounter(VexThreadState * state, const long long & startingTime) {
////	ThreadManager **manager = getCurrentThreadManagerOf(state);
////	if (manager != NULL) {
////		(*manager)->onThreadInteractionPointEncounter(state, startingTime);
////	} else {
////		state->updateCpuTimeClock();
////	}
//}
//
//void ThreadManagerRegistry::onThreadContendedEnter(VexThreadState * state, const long long & presetTime) {
//	ThreadManager **manager = getCurrentThreadManagerOf(state);
//	if (manager != NULL) {
//		(*manager)->onThreadContendedEnter(state, presetTime);
//	} else {
//		if (state != NULL) {
//			state->updateCpuTimeClock();
//		}
//	}
//}
//
//void ThreadManagerRegistry::onThreadContendedEntered(VexThreadState *state) {
//	ThreadManager **manager = getCurrentThreadManagerOf(state);
//	if (manager != NULL) {
//		(*manager)->onThreadContendedEntered(state);
//	} else {
//		if (state != NULL) {
//			state->updateCpuTimeClock();
//		}
//	}
//}
//
//void ThreadManagerRegistry::lockMutex(VexThreadState *state) {
//	ThreadManager **manager = getCurrentThreadManagerOf(state);
//	if (manager != NULL) {
//		(*manager)->lockMutex();
//	}
//}
//
//void ThreadManagerRegistry::unlockMutex(VexThreadState *state) {
//	ThreadManager **manager = getCurrentThreadManagerOf(state);
//	if (manager != NULL) {
//		(*manager)->unlockMutex();
//	}
//}
//
//void ThreadManagerRegistry::setCurrentThreadVT(const long long &presetTime, VexThreadState *state) {
//	ThreadManager **manager = getCurrentThreadManagerOf(state);
//	if (manager != NULL) {
//		(*manager)->setCurrentThreadVT(presetTime, state);
//	} else {
//		state->updateCpuTimeClock();
//	}
//}
//
//
//void ThreadManagerRegistry::notifySchedulerForVirtualizedTime(VexThreadState *state, const float &speedup) {
//// TODO: this has global interest
//	ThreadManager **manager = getCurrentThreadManagerOf(state);
//	if (manager != NULL) {
//		(*manager)->notifySchedulerForVirtualizedTime(state, speedup);
//	}
//}

long long ThreadManagerRegistry::getGlobalTime() {
	return getDefaultManager()->getCurrentGlobalTime();
}

void ThreadManagerRegistry::suspendCurrentThread(VexThreadState *state, const long long & startingTime, const char & options) {
	state->lockShareResourceAccessKey();
	ThreadManager **manager = getCurrentThreadManagerOf(state);
	if (manager == NULL) {
		getDefaultManager()->suspendCurrentThread(state, startingTime, options);
	} else {
		(*manager)->suspendCurrentThread(state, startingTime, options);
	}
	state->unlockShareResourceAccessKey();
}

//void ThreadManagerRegistry::onThreadExplicitlySetWaiting(const long long & startingTime, VexThreadState *state) {
//	ThreadManager **manager = getCurrentThreadManagerOf(state);
//	if (manager != NULL) {
//		(*manager)->onThreadExplicitlySetWaiting(startingTime, state);
//	}
//}


void ThreadManagerRegistry::writeStats(const char *filename) {
	ofstream myfile;
	if (filename) {
		myfile.open(filename);
		if (myfile.fail()) {
			fprintf(stderr, "Problem opening output file %s. Exiting", filename);
			fflush(stderr);
			return;
		}
	} else {
		return;
	}

	for (unsigned int i = 0; i<currentlyRegistered; ++i) {
		myfile << managerList[i]->getStats() << endl;
	}
	myfile.close();
}


ThreadManagerRegistry::~ThreadManagerRegistry() {
	pthread_spin_destroy(&spinlock);
}

