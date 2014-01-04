/*********************
 *
 * PassiveManager class: used to profile Java applications using the rest of the VTF just for profiling reasons.
 * In other words, we are just profiling the app using the VTF hooks.
 *
 */

#include "PassiveManager.h"
#include "VirtualTimeline.h"

#include <sys/time.h>
#include <sys/errno.h>
#include <syscall.h>
#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <cassert>


/*************************************************************************
 ****
 **** CONSTRUCTOR / DESTRUCTOR
 ****
 ************************************************************************/
PassiveManager::~PassiveManager() {

}


/*************************************************************************
 **** 
 **** THREAD FUNCTIONS: mostly called by threads from the agent callbacks
 **** 
 ************************************************************************/

//---------------- THREAD STARTING FUNCTIONS ----------------------------//

void PassiveManager::onThreadInteractionPointEncounter(VexThreadState *state, const long long& startingTime) {
	if (state != NULL) {
		state->addInvocationPoints();
	}
}


void PassiveManager::onThreadSpawn(VexThreadState *state) {
	
	if (state == NULL) {
		return;
	}
	// Jump into current time - internal locking
//	virtualTimelineController->updateNewThreadTimestamp(state);

	state->setRegistering();

	VISUALIZE_EVENT(THREAD_START, state);

	threadRegistry->add(state);			// Populate the index data structure for this Thread state - scheduler still unaware of this thread

	// Always wake-up scheduler - what if the running thread gets into a while(condition==true) loop (with the condition becoming false by the other threads)

	// The next two statements make sure that no native waiting takes place
	state->setThreadCurrentlyControllingManager(this);
	state->setRunning();
	state->updateCpuTimeClock();
}


//----------------------- THREAD RUNNING FUNCTIONS ----------------------//
bool PassiveManager::shouldCurrentThreadSuspend(VexThreadState *state) {
	return false;
}


void PassiveManager::blockCurrentThread(VexThreadState *state) {	/* Immediately block thread - suspend after awaking*/

}

/*
 * Yield the CPU to another thread
 *
 * Linux behaviour: wait for all other threads to be scheduled before you re-schedule yourself (according to javamex)
 * Behaviour here: check number of all runnable threads and wait for timeslot * their number
 */
void PassiveManager::onThreadYield(VexThreadState *state, const long long &startingTime) {


}

/**
 * Set a thread as the currently running thread of this processor
 * @lock: assumed manager lock held when entering
 */
void PassiveManager::setRunningThread(VexThreadState *state) {

}

void PassiveManager::setSuspendedAndPushToRunnables(VexThreadState *state) {

}


/*
 * Set waiting without re-inserting into the thread list
 */
void PassiveManager::setWaitingThread(VexThreadState *state) {

}

void PassiveManager::setNativeWaiting(VexThreadState *state) {

}

/*
 * Set waiting and re-insert into the thread list
 */
void PassiveManager::setTimedWaitingThread(VexThreadState *state) {

}

/*
 * Set a thread into an I/O performing state
 *
 * The policies are the following:
 * - IO_SEQUENTIAL		: don't push back into the queue, but freeze scheduler
 * - IO_PARALLEL_STRICT : push back into the queue, when finished just update the queue.
 * - IO_PARALLEL_LAX	: don't push back into the queue, do that when finished
 */
void PassiveManager::setIoThread(VexThreadState *state, bool learning) {

}

/*
 * suspendCurrentThread method: Disabled
 */
void PassiveManager::suspendCurrentThread(VexThreadState *state, const long long & startingTime, const char & options) {

}


/*
 * Function used when a thread is found to perform an I/O operation inside a vtf-monitored method
 * Handles this event according to the setting of the IoSimulator
 * - IO_SEQUENTIAL: the world shall hold until this thread finished execution
 * - IO_PARALLEL_STRICT: other threads are allowed to continue <=> their eRT is less than the least prediction
 * - IO_PARALLEL_LAX: any thread able to perform I/O does so in parallel
 *
void PassiveManager::onThreadIoStart(ThreadState *state) {
//	updateCurrentThreadVT(state);
//	state->updateClocks();
}



 * Called at the end of an I/O operation
 *
 * @return: bool whether the I/O operation finished normally in VTF and a new measurement was appended
 *
void PassiveManager::onThreadIoEnd(const long long& realTimeValueOnExit, ThreadState *state, const int &methodId) {														//TODO: GC time during I/O
//	long long actualIoDuration = (realTimeValueOnExit - state->getLastRealTime()); // - (virtualTimelineController->accGCtime - state->lastGCTimeBeforeIO));
//	state->addElapsedTime(actualIoDuration);
//	lockMutex();
//	virtualTimelineController->commitIoTimeProgress(state);
//	unlockMutex();
}


 * Virtual Time logging function - Probably the most important part of the code
 * @lock: should be externally locked 
 */
void PassiveManager::updateCurrentThreadVT(VexThreadState *state) {
	long long currentTime = state->getVirtualTime();
	long long timeDiff = currentTime - state->getLastCPUTime();// - state->lost_time;
	// Check whether the Virtual Time has progressed
	if (timeDiff > 0) {
		state->addLocalTime(timeDiff);
		virtualTimelineController->commitCpuTimeProgress(state);
	}
}

/***
 * Virtual Time logging function II - Set a previously acquired measurement as the current time
 * of the thread
 * @lock: should be externally locked
 */
void PassiveManager::setCurrentThreadVT(const long long &presetTime, VexThreadState *state) {
	long long timeDiff = presetTime - state->getLastCPUTime();// - state->lost_time;
	// Check whether the Virtual Time has progressed
	if (timeDiff > 0) {
		lockMutex();
		state->addLocalTime(timeDiff);
		virtualTimelineController->commitCpuTimeProgress(state);
		unlockMutex();
	}
}


void PassiveManager::setCurrentThreadVTLockless(const long long &startingTime, VexThreadState *state) {
	long timeDiff = startingTime - state->getLastCPUTime();
	if (timeDiff > 0) {
		locklessUpdateTimeBy(timeDiff, state);
	}
}


//----------------------- THREAD WAITING FUNCTIONS ----------------------//
/*
 * A thread sets itself into the WAITING state
 */
void PassiveManager::onThreadWaitingStart(const long long &startingTime, VexThreadState *state) {
	lockMutex();
	setCurrentThreadVTLockless(startingTime, state);
	unlockMutex();
	state->updateCpuTimeClock();
}

/*
 * A previously WAITING thread sets itself wakes up and sets itself into the suspended threads list
 */
void PassiveManager::onThreadWaitingEnd(VexThreadState *state) {
	if (state == NULL) {
		return;
	}

	long long startingTime = state->getVirtualTime();
	setCurrentThreadVTLockless(startingTime, state);

	lockMutex();
	virtualTimelineController->commitTimedWaitingProgress(state);
	virtualTimelineController->updateBlockedThreadTimestamp(state);
	unlockMutex();

	state->updateCpuTimeClock();
}

void PassiveManager::onThreadContendedEntered(VexThreadState *state) {
	if (state == NULL) {
		return;
	}

	long long startingTime = state->getVirtualTime();
	setCurrentThreadVTLockless(startingTime, state);

	lockMutex();
	virtualTimelineController->commitTimedWaitingProgress(state);
	virtualTimelineController->updateBlockedThreadTimestamp(state);
	unlockMutex();

	state->updateCpuTimeClock();
}




void PassiveManager::suspendLooseCurrentThread(VexThreadState *state, const long long & startingTime) {
	state->setRunning();
}

/**
 * Register a thread to be interrupted on a virtual timeout (nanoseconds).
 * The thread will add the expected timeout to its estimated real time in order to be scheduler by the scheduler only
 * when the timeout has expired. If
 * @assumptions: lock is NOT held when the method is called
 * @lock: lock
 */
void PassiveManager::onThreadTimedWaitingStart(VexThreadState *state, long &timeout) {
	cout << "Error: entering onThreadTimedWaitingStart" << endl;
	assert(false);
}
void PassiveManager::onThreadTimedWaitingEnd(VexThreadState *state, const long &interruptTime) {
	cout << "Error: entering onThreadTimedWaitingEnd" << endl;
	assert(false);
}



//----------------------- THREAD ENDING FUNCTIONS ----------------------//


/****
 * Very critical method that clear-ups the thread structures on its end
 * 
 * First suspect for bug-causes...
 * @lock: lock
 */ 
void PassiveManager::onThreadEnd(VexThreadState *state) {

	if (state != NULL) {
		lockMutex();
		updateCurrentThreadVT(state);

		state->setDead();
		VISUALIZE_EVENT(THREAD_END, state);

		unlockMutex();

		threadRegistry->remove(state);
	}		
}



int PassiveManager::unconditionalWakeup() {
	return 0;
}

void PassiveManager::onWrappedTimedWaitingEnd(VexThreadState *state) {
	if (state != NULL) {
		virtualTimelineController->updateTimedOutWaitingThreadTimestamp(state);
	}
}

void PassiveManager::onReplacedWaiting(VexThreadState *state) {

}

void PassiveManager::onReplacedTimedWaiting(VexThreadState *state, const long &timeout) {
	state->setRunning();
}

void PassiveManager::start() {

}

void PassiveManager::end() {
    running = false;
}
