
/*
 * ThreadEventsBehaviour.cpp
 *
 *  Created on: 18 Oct 2011
 *      Author: root
 */

#include "Constants.h"
#include "ThreadEventsBehaviour.h"
#include "Statistics.h"
#include "ThreadState.h"
#include "Time.h"
#include "Timers.h"
#include "DebugUtil.h"
#include "ThreadManager.h"
#include "ThreadManagerRegistry.h"
#include "ThreadRegistry.h"
#include "EventLogger.h"
#include "ObjectRegistry.h"
#include "AggregateStateCounters.h"
#include "LockRegistry.h"

#ifdef USING_PAPIPROFILER
#include "PapiProfiler.h"
#else
class PapiProfiler;
#endif

#include <cassert>
#include <climits>
#include <iostream>
#include <algorithm>
using namespace std;


ThreadEventsBehaviour::ThreadEventsBehaviour(ThreadManagerRegistry *_managers, ThreadRegistry *_registry, EventLogger *_eventLogger, ObjectRegistry *_waitingOnObjectRegistry, LockRegistry *_lockRegistry, AggregateStateCounters *_aggregateStateTransitionCounters, const bool &_stackTraceMode, const bool &_usingVtfScheduler, PapiProfiler *_hpcProfiler) {
	managers = _managers;
	registry = _registry;
	eventLogger = _eventLogger;
	waitingOnObjectRegistry = _waitingOnObjectRegistry;
	lockRegistry = _lockRegistry;
	aggregateStateTransitionCounters = _aggregateStateTransitionCounters;
	stackTraceMode = _stackTraceMode;
	usingVtfScheduler = _usingVtfScheduler;
	hpcProfiler = _hpcProfiler;
}


ThreadEventsBehaviour::~ThreadEventsBehaviour() {

}


void ThreadEventsBehaviour::onThreadMainStart(const long &threadId) {
	registry->newThreadBeingSpawned(0, threadId);		// let the registry know that main is starting
	onStart(threadId, "main");
}

// This method should be called when a thread with unique id (in its language) threadId and name threadName is started.
void ThreadEventsBehaviour::onStart(const long &threadId, const char *threadName) {

#ifdef USING_PAPIPROFILER
	if (hpcProfiler != NULL) {
		hpcProfiler->onThreadStart();
	}
#endif

	if (isThreadInExcludedThreadNamesList(threadName) || !registry->hasThreadParentRegisteredThisThread(threadId)) {
//		cout << " excluding " << threadName << endl;
		return;
	}

	long newThreadId = (unsigned long) syscall(SYS_gettid);
//	long newThreadId = threadId;
//	cout << "staring thread " << newThreadId << endl;
	VexThreadState *state = registry->getCurrentThreadState(newThreadId);

	if (state == NULL) {
		if (!stackTraceMode) {
			state = new VexThreadState(threadId, const_cast<char *>(threadName));
		} else {
			state = new StackTraceThreadState(threadId, const_cast<char *>(threadName));
		}
		if (state != NULL) {
			managers->onThreadSpawn(state);
		}

		//cout << "********starting state " << state->getName() << " " << state->getId() << " at " << state->getEstimatedRealTime()/1e6 << endl;
	} else {
		managers->suspendCurrentThread(state, 0, ThreadManager::SUSPEND_OPT_FORCE_SUSPEND);
	}
	LOG_LAST_VEX_METHOD(state)

}


// This method should be called when a waiting call that is about to be executed in the original runtime.
// It should be matched with a onWrappedWaitingEnd, like
// VEX driver: onWrappedWaitingStart()
// Program: 			pthread_cond_wait(...);
// VEX driver: onWrappedWaitingEnd()
void ThreadEventsBehaviour::onWrappedWaitingStart() {
	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state != NULL) {
		state->onWrappedWaitingStart();
		LOG_LAST_VEX_METHOD(state)
	}
}

void ThreadEventsBehaviour::onWrappedWaitingEnd() {
	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state != NULL) {
		state->onWrappedWaitingEnd();
		LOG_LAST_VEX_METHOD(state)
	}
}


// This method should be called before a timed-waiting-type method is replaced by a virtual-time wait within VEX.
// Because the mutex lock of the original wait will have to be released,
// this call adds the id of the mutex in the registry of waiting objects, before this happens
// This allows VEX to use the original semantics of the program to guarantee mutual exclusion
// between a notifying and a waiting thread
bool ThreadEventsBehaviour::beforeReleasingLockOfAnyReplacedWaiting(const long &objectId, const bool &updateLockRegistry) {
	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state != NULL) {
		state->onVexEntry();

		waitingOnObjectRegistry->insert(objectId, state->getUniqueId());

		if (updateLockRegistry) {
			onLocklessReleasingLock(state, objectId);
		}

		state->onVexExitWithoutTimeUpdate();
		return true;
	} else {
		return false;
	}
}

// This method should be called when a timed-waiting-type method is replaced by a virtual-time wait within VEX.
bool ThreadEventsBehaviour::onReplacedTimedWaiting(const long &objectId, const long &timeout, const int &nanoTimeout, const bool &updateLockRegistry) {
	VexThreadState *state = VexThreadState::getCurrentThreadState();//registry->getCurrentThreadState(threadId);

	if (state != NULL) {

		long correctTimeout = ensureTimeoutIsLessThanMax(timeout, nanoTimeout);
		bool hasReplacedTimedWaitingTimedOut = state->onReplacedTimedWaiting(objectId, correctTimeout);

		if (updateLockRegistry) {
			onLocklessRequestingLock(state, objectId);
		}

		LOG_LAST_VEX_METHOD(state)
		state->onVexExitWithCpuTimeUpdate();

		return hasReplacedTimedWaitingTimedOut;
	} else {
		return false;
	}
}


bool ThreadEventsBehaviour::onReplacedWaiting(const long &objectId, const bool &updateLockRegistry) {
	VexThreadState *state = VexThreadState::getCurrentThreadState();//registry->getCurrentThreadState(threadId);

	if (state != NULL) {

		bool hasReplacedTimedWaitingTimedOut = state->onReplacedWaiting(objectId, updateLockRegistry);

		if (updateLockRegistry) {
			onLocklessRequestingLock(state, objectId);	// try to re-acquire the lock after waiting
		}

		LOG_LAST_VEX_METHOD(state)
		state->onVexExitWithCpuTimeUpdate();

		return hasReplacedTimedWaitingTimedOut;
	} else {
		return false;
	}
}



bool ThreadEventsBehaviour::onSleep(const long &timeout, const int &nanoTimeout) {
	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state != NULL) {
		long correctTimeout = ensureTimeoutIsLessThanMax(timeout, nanoTimeout);
		bool hasReplacedTimedWaitingTimedOut = state->onReplacedTimedWaiting(0, correctTimeout);

		LOG_LAST_VEX_METHOD(state)

		state->onVexExitWithCpuTimeUpdate();

		return hasReplacedTimedWaitingTimedOut;
	}

	return false;

}


bool ThreadEventsBehaviour::onSignallingOnObject(pthread_mutex_t * mutex) {
	long objectId = reinterpret_cast<long>(const_cast<pthread_mutex_t *>(mutex));
	return onSignallingOnObject(objectId);
}

bool ThreadEventsBehaviour::onBroadcastingOnObject(pthread_mutex_t * mutex) {
	long objectId = reinterpret_cast<long>(const_cast<pthread_mutex_t *>(mutex));
	return onBroadcastingOnObject(objectId);
}

//TODO: remove me
bool ThreadEventsBehaviour::beforeTimedWaitingOn(pthread_mutex_t * mutex, const bool &updateLockRegistry) {
	return beforeReleasingLockOfAnyReplacedWaiting(mutex, updateLockRegistry);
}
//TODO: remove me
bool ThreadEventsBehaviour::onTimedWaitingOn(pthread_mutex_t * mutex, const long &timeout, const int &nanoTimeout, const bool &updateLockRegistry) {
	return onReplacedTimedWaiting(mutex, timeout, nanoTimeout, updateLockRegistry);
}



bool ThreadEventsBehaviour::beforeReleasingLockOfAnyReplacedWaiting(pthread_mutex_t * mutex, const bool &updateLockRegistry) {
	long objectId = reinterpret_cast<long>(const_cast<pthread_mutex_t *>(mutex));
	return beforeReleasingLockOfAnyReplacedWaiting(objectId, updateLockRegistry);
}

bool ThreadEventsBehaviour::onReplacedTimedWaiting(pthread_mutex_t * mutex, const long &timeout, const int &nanoTimeout, const bool &updateLockRegistry) {
	long objectId = reinterpret_cast<long>(const_cast<pthread_mutex_t *>(mutex));
	return onReplacedTimedWaiting(objectId, timeout, nanoTimeout, updateLockRegistry);
}

void ThreadEventsBehaviour::onRequestingLock(pthread_mutex_t *mutex) {
	long objectId = reinterpret_cast<long>(const_cast<pthread_mutex_t *>(mutex));
	return onRequestingLock(objectId);
}

void ThreadEventsBehaviour::onReleasingLock(pthread_mutex_t * mutex) {
	long objectId = reinterpret_cast<long>(const_cast<pthread_mutex_t *>(mutex));
	return onReleasingLock(objectId);
}



bool ThreadEventsBehaviour::onSignallingOneThreadOnObject(const long &objectId, ThreadManager *stateManager, const long long &interruptionTime) {

	long nextWaitingThreadId = waitingOnObjectRegistry->findNext(objectId);

	if (nextWaitingThreadId != 0) {
		VexThreadState *stateOfThreadToInterrupt = registry->getCurrentThreadState(nextWaitingThreadId);
		return interruptThread(stateOfThreadToInterrupt, stateManager, interruptionTime, false);

	} else {
		stateManager->increasePendingNotifications();
	}
	return false;

}


/*
 * isThreadInterruptCall: is used to set the timedOut flag so that when a thread is interrupted
 * in knows whether it was notified or interrupted (in the latter case it might have to throw an exception - send an interrupt)
 */
bool ThreadEventsBehaviour::interruptThread(VexThreadState *stateOfThreadToInterrupt, ThreadManager *stateManager, const long long &interruptionTime, bool isThreadInterruptCall) {
	if (stateOfThreadToInterrupt != NULL && stateOfThreadToInterrupt->isWaiting()) {
		// We do not set timedOut = false here, because this only makes sense, when the thread itself changes its state later
		stateManager->onAnyReplacedWaitingInterrupt(stateOfThreadToInterrupt, interruptionTime);
		stateOfThreadToInterrupt->setTimedOut(!isThreadInterruptCall);
		return true;
	}

	return false;
}

bool ThreadEventsBehaviour::onSignallingAllThreadsOnObject(const long &objectId, ThreadManager *stateManager, const long long &interruptionTime) {

	long nextWaitingThreadId = waitingOnObjectRegistry->getNext(objectId);
	if (nextWaitingThreadId == 0) {
		stateManager->increasePendingNotifications();
	} else {

		bool anyThreadsInterrupted = false;
		while (nextWaitingThreadId != 0) {
			VexThreadState *stateOfThreadToInterrupt = registry->getCurrentThreadState(nextWaitingThreadId);
			anyThreadsInterrupted |= interruptThread(stateOfThreadToInterrupt, stateManager, interruptionTime, false);
			nextWaitingThreadId = waitingOnObjectRegistry->getNext(objectId);
		}

		return anyThreadsInterrupted;
	}

	return false;
}

bool ThreadEventsBehaviour::onSignallingOnObject(const long &objectId) {
	long long startingTime = Time::getThreadTimeBeforeInteractionPoint(); // used to account for as lost time

	VexThreadState *state = VexThreadState::getCurrentThreadState();

	if (state != NULL) {
		// Update the time of this thread, in order to set it as the interrupt time of the waiting thread
		ThreadManager *stateManager = state->getCurrentSchedulerOnVexEntry(startingTime);
		bool aThreadWasSignalled = onSignallingOneThreadOnObject(objectId, stateManager, state->getEstimatedRealTime());

		LOG_LAST_VEX_METHOD(state)

		state->onVexExitWithCpuTimeUpdate();

		return aThreadWasSignalled;
	} else {

		return onSignallingOneThreadOnObject(objectId, managers->getDefaultManager(), 0);
	}

}



bool ThreadEventsBehaviour::onBroadcastingOnObject(const long &objectId) {
	long long startingTime = Time::getThreadTimeBeforeInteractionPoint(); // used to account for as lost time

	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state != NULL) {
		// Suspend this thread first to update its virtual timestamp, in order to set it as the interrupt time of the waiting thread
		ThreadManager *stateManager = state->getCurrentSchedulerOnVexEntry(startingTime);

		bool atLeastOneThreadWasSignalled = onSignallingAllThreadsOnObject(objectId, stateManager, state->getEstimatedRealTime());

		LOG_LAST_VEX_METHOD(state)
		state->onVexExitWithCpuTimeUpdate();
		return atLeastOneThreadWasSignalled;

	} else {
		return onSignallingAllThreadsOnObject(objectId, managers->getDefaultManager(), 0);
	}

}

/*
 * isThreadInterruptCall: is used to set the timedOut flag so that when a thread is interrupted
 * in knows whether it was notified or interrupted (in the latter case it might have to throw an exception - send an interrupt)
 */
bool ThreadEventsBehaviour::interruptThread(const long &interruptedThreadId, ThreadManager *stateManager, const long long &interruptionTime, bool isThreadInterruptCall) {
	VexThreadState *stateOfThreadToInterrupt = registry->getCurrentThreadState(interruptedThreadId);
	return interruptThread(stateOfThreadToInterrupt, stateManager, interruptionTime, isThreadInterruptCall);
}

bool ThreadEventsBehaviour::onInterrupt(const long &interruptedThreadId) {
	long long startingTime = Time::getThreadTimeBeforeInteractionPoint();

	VexThreadState *state = VexThreadState::getCurrentThreadState();

	if (state != NULL) {

		ThreadManager *stateManager = state->getCurrentSchedulerOnVexEntry(startingTime);
		long interruptionTime = state->getEstimatedRealTime();

		bool theThreadWasInterrupted = interruptThread(interruptedThreadId, stateManager, interruptionTime, true);
	//cout << "zzzzzzzzz: thread " << state->getName() << " wants to intrrupt " << interruptedThreadId << "??" << theThreadWasInterrupted<< endl;
		LOG_LAST_VEX_METHOD(state)
		state->onVexExitWithCpuTimeUpdate();

		return theThreadWasInterrupted;
	} else {
		return interruptThread(interruptedThreadId,  managers->getDefaultManager(), 0, true);

	}
}


void ThreadEventsBehaviour::onRequestingLock(const long &lockId) {
	VexThreadState *state = VexThreadState::getCurrentThreadState();//registry->getCurrentThreadState(threadId);
	if (state != NULL) {
		state->onVexEntry();

		onLocklessRequestingLock(state, lockId);

		LOG_LAST_VEX_METHOD(state)
		state->onVexExitWithCpuTimeUpdate();
	}
}

void ThreadEventsBehaviour::onReleasingLock(const long &lockId) {
	VexThreadState *state = VexThreadState::getCurrentThreadState();//registry->getCurrentThreadState(threadId);
	if (state != NULL) {
		state->onVexEntry();

		onLocklessReleasingLock(state, lockId);
		LOG_LAST_VEX_METHOD(state)

		state->onVexExitWithCpuTimeUpdate();
	}
}



void ThreadEventsBehaviour::onJoin(const long &joiningThreadId) {
	VexThreadState *state = VexThreadState::getCurrentThreadState();

	if (state != NULL) {
		long long startingTime = state->getTimeOnVexEntry(); //locks the registry

		ThreadManager *manager = getCurrentlyControllingManagerOf(state);
		manager->setCurrentThreadVT(startingTime, state);

		//Gio - For some reason the registry is locked here
		//not the lock that's causing the issue
//		state->unlockShareResourceAccessKey();

		// The point here is to coordinate with any joining threads
		// We iteratively execute the method to ensure that no threads are currently being spawned
		// otherwise this might lead to the unborn thread with id joiningThreadId to be mistaken for dead
		while (!registry->coordinateJoiningThreads(VexThreadState::getCurrentThreadStatePtr(), joiningThreadId)) {
//			manager->suspendCurrentThread(state, startingTime,
//					ThreadManager::SUSPEND_OPT_FORCE_SUSPEND |
//					ThreadManager::SUSPEND_OPT_DONT_UPDATE_THREAD_TIME); //| ThreadManager::SUSPEND_OPT_THREAD_ALREADY_IN_LIST );
			//Gio - do not add me to the list, let the exiting thread wake me up
			manager->onThreadYield(state, startingTime);
			manager = getCurrentlyControllingManagerOf(state); // state thread calls thread.join() with thread = thread with joiningThreadId
		}

		state->onVexExitWithCpuTimeUpdate();
	}
}


void ThreadEventsBehaviour::onWrappedTimedWaitingStart(const long &objectId, long &timeout, const int &nanoTimeout) {
	const long long startingTime =  Time::getThreadTimeBeforeInteractionPoint();
	VexThreadState *state = VexThreadState::getCurrentThreadState();//registry->getCurrentThreadState(threadId);
	if (state != NULL) {
//		cout << state->getName() << " starting wrapped timed-waiting at " << state->getEstimatedRealTime()/1e6 << endl;
		long correctTimeout = ensureTimeoutIsLessThanMax(timeout, nanoTimeout);

		state->onVexEntry();
		waitingOnObjectRegistry->insert(objectId, state->getUniqueId());
		LOG_LAST_VEX_METHOD(state)

		state->onWrappedTimedWaitingStart(objectId, startingTime, correctTimeout);
	}
}


void ThreadEventsBehaviour::onWrappedTimedWaitingEnd() {
	VexThreadState *state = VexThreadState::getCurrentThreadState();//registry->getCurrentThreadState(threadId);
	if (state != NULL) {
		state->onWrappedTimedWaitingEnd(waitingOnObjectRegistry);
//		cout << state->getName() << " ended wrapped timed-waiting at " << state->getEstimatedRealTime()/1e6 << endl;
		LOG_LAST_VEX_METHOD(state)
	}
}


void ThreadEventsBehaviour::onWrappedTimedWaitingInterrupt(const long &objectId) {
	VexThreadState *state = VexThreadState::getCurrentThreadState();//registry->getCurrentThreadState(threadId);
	if (state != NULL) {

		state->onVexEntry();

		long nextWaitingThreadId = 0;
		do {
			ThreadManager *stateManager = getCurrentlyControllingManagerOf(state);

//			interruptThread(nextWaitingThreadId, stateManager, state->getEstimatedRealTime());
			VexThreadState *stateOfThreadToInterrupt = registry->getCurrentThreadState(nextWaitingThreadId);
			if (stateOfThreadToInterrupt != NULL && stateOfThreadToInterrupt->isWaiting()) {
				stateOfThreadToInterrupt->lockShareResourceAccessKey();
				stateOfThreadToInterrupt->setTimedOut(false);
				stateManager->onWrappedTimedWaitingInterrupt(stateOfThreadToInterrupt, state->getEstimatedRealTime());
				stateOfThreadToInterrupt->unlockShareResourceAccessKey();

			}
			nextWaitingThreadId = waitingOnObjectRegistry->getNext(objectId);

		} while (nextWaitingThreadId != 0);

		LOG_LAST_VEX_METHOD(state)
		state->onVexExitWithCpuTimeUpdate();
	}
}


/*
 * Called before a thread starts parking - essentially it is a selection between a wrapped normal and a wrapped-timed waiting.
 */
void ThreadEventsBehaviour::onPark(const bool &isAbsoluteTimeout, const long &timeout) {
//	assert(!isAbsoluteTimeout);

	VexThreadState *state = VexThreadState::getCurrentThreadState();//registry->getCurrentThreadState(threadId);
	if (state != NULL) {

		long long startingTime = state->getTimeOnVexEntry();

		// If you are already unparked, just skip this
		if (!state->getAndSetUnparkedState()) {
			if (timeout > 0) {
				state->onWrappedTimedWaitingStart(0, startingTime, timeout);
			} else {
				state->unlockShareResourceAccessKey();
				state->onWrappedWaitingStart();
				state->onVexExitWithCpuTimeUpdate();
			}
		} else {
			state->onVexExitWithCpuTimeUpdate();
		}
		LOG_LAST_VEX_METHOD(state)

	}
}

/*
 * Called after the return from the park() call to return the thread under VEX control
 */
void ThreadEventsBehaviour::onParked() {
	VexThreadState *state = VexThreadState::getCurrentThreadState();//registry->getCurrentThreadState(threadId);
	if (state != NULL) {

		if (state->getTimeout() > 0) {
			state->onWrappedTimedWaitingEnd(NULL);
		} else {
			state->onWrappedWaitingEnd();
		}
		LOG_LAST_VEX_METHOD(state)
	}
}

/*
 * Called to trigger a (timed-)parking thread to unpark - like an interrupt for waits
 */
void ThreadEventsBehaviour::onUnpark(const long &unparkingThreadId) {

	VexThreadState *state = VexThreadState::getCurrentThreadState();//registry->getCurrentThreadState(threadId);
	if (state != NULL) {
		state->onVexEntry();
	}

	VexThreadState *threadToUnparkState = registry->getCurrentThreadState(unparkingThreadId);
	if (threadToUnparkState != NULL) {
		threadToUnparkState->lockShareResourceAccessKey();
		if (threadToUnparkState->getAndSetParkedState()) { // if parked == true, then unpark the thread and unblock it, else set flag for next parking

			long long unParkingTime;
			ThreadManager *manager;
			if (state == NULL) { 		// Even a VEX unsimulated thread should unpark another thread
				manager = managers->getDefaultManager();
				unParkingTime = 0;
			} else {
				manager = getCurrentlyControllingManagerOf(state);
				unParkingTime = state->getEstimatedRealTime();
			}

			if (threadToUnparkState->getTimeout() > 0) {
				threadToUnparkState->setTimedOut(false);
				manager->onWrappedTimedWaitingInterrupt(threadToUnparkState, unParkingTime);
			} else {
				manager->onWrappedWaitingInterrupt(threadToUnparkState, unParkingTime);
			}
		}
		threadToUnparkState->unlockShareResourceAccessKey();
	} else {
		cout << "FAILED TO FIND A THREAD TO UNPARK BUT " << registry->getThreadsBeingSpawned() << endl;
	}

	if (state != NULL) {
		state->onVexExitWithCpuTimeUpdate();
		LOG_LAST_VEX_METHOD(state)
	}
}



/*
 * A parent thread denotes its relationship to its child, which:
 * - enables the new thread to identify whether it is registered or not
 * - stores the estimated real time of the parent thread when the thread was spawned
 */
void ThreadEventsBehaviour::beforeCreatingThread(long threadToBeSpawnedApplicationLanguageId) {
	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state != NULL) {
		long long startingTime = state->getTimeOnVexEntry();

//cout << state->getName() << " " << state->getUniqueId() << " is the father of " <<  threadToBeSpawnedApplicationLanguageId << endl;

		registry->newThreadBeingSpawned(state->getUniqueId(), threadToBeSpawnedApplicationLanguageId);  // counter used to avoid leaps in virtual time
		ThreadManager *manager = getCurrentlyControllingManagerOf(state);
		manager->setCurrentThreadVT(startingTime, state);
//		state->doNotSuspend();  // used to avoid interrupting thread holding the malloc-related lock
		LOG_LAST_VEX_METHOD(state)

		//state->onVexExitWithCpuTimeUpdate();
		state->onVexExitWithBothClocksUpdate();
	}
}

void ThreadEventsBehaviour::afterCreatingThread() {

	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state != NULL) {

//		state->getRealTimeOnVexEntry();

		state->getTimeOnVexEntry();
		getCurrentlyControllingManagerOf(state);

		LOG_LAST_VEX_METHOD(state)
		state->onVexExitWithCpuTimeUpdate();
	}

}




void ThreadEventsBehaviour::onInteractionPoint() {
	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state != NULL) {
		state->onInvocationPoint();
		LOG_LAST_VEX_METHOD(state)
	}
}


void ThreadEventsBehaviour::onYield() {
	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state != NULL) {
		state->onYield();
		LOG_LAST_VEX_METHOD(state)
	}
}


// Thse are system events so do not need to identify a particular thread - maybe could be put under a SystemEventsBehaviour class
void ThreadEventsBehaviour::onBackgroundLoadExecutionStart() {

	managers->getDefaultManager()->onWrappedBackgroundLoadExecutionStart();
}

void ThreadEventsBehaviour::onBackgroundLoadExecutionEnd() {
	onBackgroundLoadExecutionEndAt(Time::getBackgroundLoadExecutionRealDuration());
}


void ThreadEventsBehaviour::onBackgroundLoadExecutionEndAt(const long long &executionDuration) {
	managers->getDefaultManager()->onWrappedBackgroundLoadExecutionEnd(executionDuration);
}


void ThreadEventsBehaviour::onEnd() {

	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state != NULL) {
#ifdef USING_PAPIPROFILER
		if (hpcProfiler != NULL) {
			hpcProfiler->onThreadEnd(state->getName());	// update hardware performance counters here before merging your profiling results
		}
#endif
		state->onVexEntry();

//		cout << "********ending state " << state->getName() << " " << state->getId() << endl;
		ThreadManager *manager = getCurrentlyControllingManagerOf(state);
		manager->onThreadEnd(state);

		// The thread is no longer part of VEX - it doesn't need to take the thread lock to access the following shared resources
#if COUNT_STATE_TRANSITIONS == 1
		aggregateStateTransitionCounters->addTransitionsOf(state);
#endif

//		double temp = 1.0;
//		for(double i = 0; i<10000000; i++) {
//			temp += pow(1.0+1.0/i, i);
//		}
//		cout << temp << endl;
		eventLogger->onThreadEnd(state);

		registry->cleanup(state);
	} else {

#ifdef USING_PAPIPROFILER
		if (hpcProfiler != NULL) {
			hpcProfiler->onThreadEnd("VM thread");
		}
#endif
	}
}

long long ThreadEventsBehaviour::getTime() {
	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state == NULL) {
		long returnValue = 1;
		returnValue = managers->getGlobalTime();
		return (returnValue>0)?returnValue:1;

	} else {

		// Update time
		long long startingTime = state->getThreadTimeBeforeMethodInstrumentation();

		long long returnValue = 1;

		if (!usingVtfScheduler) {
			state->lockShareResourceAccessKey();
			ThreadManager *manager = getCurrentlyControllingManagerOf(state);
			manager->setCurrentThreadVT(startingTime, state);
			state->unlockShareResourceAccessKey();

//			state->updateThreadLocalTimeSinceLastResumeTo(startingTime);
//			returnValue = Time::getRealTime(); 
//cout << "Returning time real time : " << returnValue << endl;
		} 
//else {
			state->updateThreadLocalTimeSinceLastResumeTo(startingTime);
			returnValue = state->getEstimatedRealTime();

//		}

		LOG_LAST_VEX_METHOD(state)
		state->onVexExit();
		return returnValue;

	}
}


/***
 * The public version of the ensure thread is not in native waiting state
 */
void ThreadEventsBehaviour::ensureThreadIsNotInNativeWaitingStateWhenEnteringVex() {
	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state != NULL) {
		state->ensureThreadIsNotInNativeWaitingStateWhenEnteringVex(0);
//		ensureThreadIsNotInNativeWaitingStateWhenEnteringVex(state, 0);
		state->onVexExit();
	}


}

long ThreadEventsBehaviour::ensureTimeoutIsLessThanMax(const long &timeout, const int &nanoTimeout) {
	if (timeout >= (LONG_MAX/1000000)) {
		return LONG_MAX;
	} else {
		return timeout * 1000000 + nanoTimeout;
	}
}

//bool ThreadEventsBehaviour::ensureThreadIsNotInNativeWaitingStateWhenEnteringVex(VexThreadState *state, const long long &startingTime) {
//	bool threadIsNotInNativeWaiting = true;
//
//	state->onVexEntry();
//
//	if (!state->isRunning() || state->getThreadCurrentlyControllingManager() == NULL) {
//		state->setCustom2();
//		state->lockShareResourceAccessKey();
//		managers->getDefaultManager()->suspendLooseCurrentThread(state, startingTime);
//		//managers->getDefaultManager()->suspendCurrentThread(state, startingTime, ThreadManager::SUSPEND_OPT_EXTERNALLY_LOCKED | ThreadManager::SUSPEND_OPT_FORCE_SUSPEND | ThreadManager::SUSPEND_OPT_DONT_WAKE_UP_SCHEDULER);
//		state->unlockShareResourceAccessKey();
//		threadIsNotInNativeWaiting = false;
//
//		assert(state->getThreadCurrentlyControllingManager() != NULL);
//	}
//
//	if (!state->isRunning()) {
//		cout << "Illegal state for thread " << state->getName() << ": " << state->getCurrentStateName() << endl;
//	}
//
//	assert(state->getThreadCurrentlyControllingManager() != NULL);
//	//		assert(state->isRunning());
//	return threadIsNotInNativeWaiting;
//}


ThreadManager *ThreadEventsBehaviour::getCurrentlyControllingManagerOf(VexThreadState *state) {

	ThreadManager *manager = state->getThreadCurrentlyControllingManager();
	while (manager == NULL) {
		managers->getDefaultManager()->suspendLooseCurrentThread(state, 0);
		manager = state->getThreadCurrentlyControllingManager();
	}
	return manager;
}












void printout(string & x)
{
    std::cout << "Excluding thread: " << x << std::endl;
}


// Read the thread names from a file and do not simulate these threads in VEX.
// Note: Clearly this only makes sense if "thread name" is something known/predictable
void ThreadEventsBehaviour::registerThreadsToBeExcludedFromSimulation(char *excludedThreadsFile) {
	ifstream excludedThreads(excludedThreadsFile);
	if (excludedThreads.is_open()) {
		string line;
		do {
			getline(excludedThreads, line);
			addThreadToBeExcludedFromSimulation(line.c_str());
		} while (!excludedThreads.eof());
		excludedThreads.close();

		std::for_each(excludedThreadsList.begin(), excludedThreadsList.end(), printout);

	} else {
		fprintf(stderr, "\nError: Unable to open excluded threads file: %s\n", excludedThreadsFile);
		fflush(stderr);

	}

}

void ThreadEventsBehaviour::addThreadToBeExcludedFromSimulation(const char *threadName) {
	excludedThreadsList.push_back(threadName);
}

struct thread_is_excluded : std::unary_function<string, bool> {
	thread_is_excluded(const char *_threadName) : threadName(_threadName) {};

	bool operator() (string &excludedThread) const {
		return excludedThread == threadName || excludedThread.find(threadName) != string::npos;	// the excluded suffix belongs to the thread name
	}
	string threadName;
};


bool ThreadEventsBehaviour::isThreadInExcludedThreadNamesList(const char *threadName) {
	//TODO: move this - this is not VEX core material
//	if (strcmp(threadName, "DestroyJavaVM") == 0) {
//		return true;
//	} else

	if (std::find_if(excludedThreadsList.begin(), excludedThreadsList.end(), thread_is_excluded(threadName)) != excludedThreadsList.end()) {
		registry->newThreadStarted();	// This counter is used to stop leaps forward when threads are under creation
		return true;
	}
	return false;
}



void ThreadEventsBehaviour::onLocklessRequestingLock(VexThreadState *state, const long &lockId) {
	if (!lockRegistry->tryAcquiringLock(state, lockId)) {
		// the lock is already held
		ThreadManager *stateManager = state->getThreadCurrentlyControllingManager();
//		long noTimeout = 0;
//		cout << state->getName() << " " << state->getId() <<  " tried and will block before acquiring monitor " << lockId << endl;
		stateManager->onReplacedWaiting(state);	//stateManager->onThreadTimedWaitingStart(state, noTimeout);
//		cout << state->getName() << " " << state->getId() <<  " is now unblocked and will acquire monitor " << lockId << endl;
	} else {
//		cout << state->getName() << " " << state->getId() <<  " tried and acquired monitor " << lockId << endl;
	}
}

void ThreadEventsBehaviour::onLocklessReleasingLock(VexThreadState *state, const long &lockId) {

	VexThreadState *threadBlockedOnMonitor = lockRegistry->releaseAndGetNext(lockId);
	if (threadBlockedOnMonitor != NULL) {
//cout << state->getName() << " " << state->getId() <<  " releasing monitor " << lockId << " and getNext on " << lockId << " was " << threadBlockedOnMonitor->getName() << endl;

		// unblock the previously blocked thread
		ThreadManager *stateManager = state->getThreadCurrentlyControllingManager();
		threadBlockedOnMonitor->lockShareResourceAccessKey();
//		stateManager->pushIntoRunnableQueue(threadBlockedOnMonitor);

		state->setTimeout(0); // Used to trigger push back in onThreadTimedWaitingEnd - TODO: this is another hack for the same problem
//		stateManager->onThreadTimedWaitingEnd(threadBlockedOnMonitor, state->getEstimatedRealTime());
		stateManager->onAnyReplacedWaitingInterrupt(threadBlockedOnMonitor, state->getEstimatedRealTime());

		threadBlockedOnMonitor->unlockShareResourceAccessKey();
	} else {
//cout << state->getName() << " " << state->getId() <<  " releasing monitor " << lockId << " and getNext on " << lockId << " was null" << endl;
	}
}


void ThreadEventsBehaviour::haltSuspendForAwhile() {
	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state != NULL) {
		state->haltSuspendForAwhile();
//		cout << "lockin34g " << state->getName() << " " << state->getId() << endl;
//		state->onVexEntry();
	}
}

void ThreadEventsBehaviour::beforeReinstrumentation() {
	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state != NULL) {
		cout << "lockin34g " << state->getName() << " " << state->getId() << endl;
//		state->onVexEntry();
	}
}

void ThreadEventsBehaviour::afterReinstrumentation() {
	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state != NULL) {
//		state->onVexExit();
		cout << "unlo43cking " << state->getName() << " " << state->getId() << endl;
	}
}
