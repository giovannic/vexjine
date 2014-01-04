/*
 * IoProtocol.cpp
 *
 *  Created on: 12 Oct 2010
 *      Author: root
 */

#include "IoProtocol.h"
#include "VirtualTimeline.h"
#include "ThreadManagerRegistry.h"
#include "ThreadManager.h"

#include <cassert>

IoProtocolEnforcer::IoProtocolEnforcer(IoProtocol *_serialSystemCallRealTimeProtocol, IoProtocol *_parallelIoRealTimeProtocol) {
	serialSystemCallRealTimeProtocol = _serialSystemCallRealTimeProtocol;
	parallelIoRealTimeProtocol = _parallelIoRealTimeProtocol;
}

void IoProtocolEnforcer::onStart(VexThreadState *state) {
	if (state->isIgnoringIo()) {
		// do nothing
	} else if (state->callRecognizedAsCached()) {
		serialSystemCallRealTimeProtocol -> onStart(state);
	} else {
		parallelIoRealTimeProtocol -> onStart(state);
	}
}
void IoProtocolEnforcer::onEnd(VexThreadState *state, const long long &actualIoDuration) {
	if (state->isIgnoringIo()) {
		// do nothing
	} else if (state->callRecognizedAsCached()) {
		serialSystemCallRealTimeProtocol -> onEnd(state, actualIoDuration);
	} else {
		parallelIoRealTimeProtocol -> onEnd(state, actualIoDuration);
	}
	state->resetIoStateFlags();
}


IoProtocol::IoProtocol() {

}

IoProtocol::~IoProtocol() {

}

void IoProtocol::onStart(VexThreadState *state) {
	state->onVexExit();
	state->updateClocks();
}
void IoProtocol::onEnd(VexThreadState *state, const long long &actualIoDuration) {

}
const char *IoProtocol::getTitle() {
	return "";
}
bool IoProtocol::areInIoThreadsInRunnablesQueue() {
	return false;
}

/***
 * I/O serial: only one thread runs at each time - the manager gets frozen at the time
 **/
IoProtocolSerial::IoProtocolSerial() {

}

const char *IoProtocolSerial::getTitle() {
	return "Serial";
}

void IoProtocolSerial::onStart(VexThreadState *state) {

	state->lockShareResourceAccessKey();
	ThreadManager *stateManager = state->getThreadCurrentlyControllingManager();

	// This suspend plays the role of commit time. We could use a commit time method, but this way we establish that the thread is ok to continue
	stateManager->updateCurrentThreadVT(state);//suspendCurrentThread(state, dummyTimeToGenerateTimeDiffEqualToOne, 0);
	stateManager->setSystemCallThread(state);

	state->onVexExitWithBothClocksUpdate();
}

void IoProtocolSerial::onEnd(VexThreadState *state, const long long &actualIoDuration) {

	state->addGlobalTime(actualIoDuration);

	state->lockShareResourceAccessKey();

	state->setIoFinishedBeforeLogging(false);	// in a next I/O the thread will be able to be invalidated again

	ThreadManager *stateManager = state->getThreadCurrentlyControllingManager();
	if (stateManager == NULL) {
		cout << "ontws ayti einai i malakia " << *state << endl;
		assert(state->getThreadCurrentlyControllingManager() != NULL);
	}
	state->setRunning();
	stateManager->updateCurrentThreadVT(state);

	state->unlockShareResourceAccessKey();

}



/***
 * I/O strict: 	threads are put on the top of the runnable threads. When they resume they execute their I/O, while allowing other threads
 * 				to resume. The real time of the I/O is used for the simulation time progress. No prediction
 **/
IoProtocolStrict::IoProtocolStrict() {

}
const char *IoProtocolStrict::getTitle() {
	return "Strict";
}
void IoProtocolStrict::onStart(VexThreadState *state) {

}

void IoProtocolStrict::onEnd(VexThreadState *state, const long long &actualIoDuration) {


}


/***
 * I/O lax: threads execute the I/O immediately while allowing otherare put on the top of the runnable threads. When they resume they execute their I/O, while allowing other threads
 * 			to resume. The real time of the I/O is used for the simulation time progress. No prediction
 **/
IoProtocolLax::IoProtocolLax() {

}

const char *IoProtocolLax::getTitle() {
	return "Lax";
}

void IoProtocolLax::onStart(VexThreadState *state) {

	long long dummyTimeToGenerateTimeDiffEqualToOne = state->getLastCPUTime() + 1;

	state->lockShareResourceAccessKey();
	ThreadManager *stateManager = state->getThreadCurrentlyControllingManager();

	// This suspend plays the role of commit time. We could use a commit time method, but this way we establish that the thread is ok to continue
	stateManager->suspendCurrentThread(state, dummyTimeToGenerateTimeDiffEqualToOne, 0);

	stateManager->setIoThread(state, true);
	stateManager->conditionalWakeup(state);

	state->onVexExitWithBothClocksUpdate();

}

void IoProtocolLax::onEnd(VexThreadState *state, const long long &actualIoDuration) {
	state->addElapsedTime(actualIoDuration);

	state->lockShareResourceAccessKey();
	state->setIoFinishedBeforeLogging(false);	// in a next I/O the thread will be able to be invalidated again

	ThreadManager **stateManager = state->getThreadCurrentlyControllingManagerPtr();
	(*stateManager)->suspendCurrentThread(state, 0, (*stateManager)->SUSPEND_OPT_DONT_UPDATE_THREAD_TIME);
	// The manager of the thread might be different than the one before the suspendCurrentThread call
	(*stateManager)->commitIoDuration(state, actualIoDuration);
	state->unlockShareResourceAccessKey();
}


/***
 * I/O normal: a thread may only run if its execution is before the expected predicted end of a previous I/O
 */
IoProtocolNormal::IoProtocolNormal() {
	pthread_spin_init(&spinlock, 0);
	pendingIoCalls = 0;
}

bool IoProtocolNormal::areInIoThreadsInRunnablesQueue() {
	return true;
}

const char *IoProtocolNormal::getTitle() {
	return "Normal";
}

void IoProtocolNormal::minimizeSchedulerTimeslot(ThreadManager *stateManager) {
	pthread_spin_lock(&spinlock);
	if (pendingIoCalls++ == 0) {
		stateManager->minimizeSchedulerTimeslot();
	}
	pthread_spin_unlock(&spinlock);
}



void IoProtocolNormal::resetSchedulerTimeslot(ThreadManagerRegistry *allThreadManagers) {
	pthread_spin_lock(&spinlock);
	if (--pendingIoCalls == 0) {
		allThreadManagers->resetDefaultSchedulerTimeslot();
	}
	pthread_spin_unlock(&spinlock);
}

void IoProtocolNormal::onStart(VexThreadState *state) {

//	long long dummyTimeToGenerateTimeDiffEqualToOne = state->getLastCPUTime() + 1;

	state->lockShareResourceAccessKey();

	ThreadManager **stateManager = state->getThreadCurrentlyControllingManagerPtr();
	// This suspend plays the role of the committing time prior to the I/O execution. We could use a commit time method, but this way we establish that the thread is ok to continue
//	(*stateManager)->suspendCurrentThread(state, dummyTimeToGenerateTimeDiffEqualToOne, 0);
	(*stateManager)->updateCurrentThreadVT(state);

	if (state->getLastIoPrediction() != 0) {
		state-> addPredictionToEstimatedRealTime();

		if (state->getTimeScalingFactor() > 1.0) {
			//cout << "Delayed I/O: execute only after suspend " << state->getCurrentMethodId() << " at " << (double)state->getEstimatedRealTime() / 1e9 << endl;
//			long long currentTime = state->getEstimatedRealTime();
			(*stateManager)->suspendCurrentThread(state, 0, ThreadManager::SUSPEND_OPT_DONT_UPDATE_THREAD_TIME | ThreadManager::SUSPEND_OPT_FORCE_SUSPEND);
			// Now that you are resumed, you have the least GVT, so you can run the delayed I/O correctly
			// (or better according to the accuracy I/O prediction)
		}

		(*stateManager)->setIoThread(state, false);				// Only in prediction, otherwise you kill the parallelism
	} else {
		if (state->getTimeScalingFactor() > 1.0) {
			//cout << "Delayed I/O men prediciton oxi from method " << state->getCurrentMethodId() << endl;
		}
		(*stateManager)->setIoThread(state, true);
	}


	minimizeSchedulerTimeslot(*stateManager);

	(*stateManager)->conditionalWakeup(state);

	state->onVexExitWithBothClocksUpdate();
}


void IoProtocolNormal::onEnd(VexThreadState *state, const long long &actualIoDuration) {

	state->lockShareResourceAccessKey();
	ThreadManager *stateManager = state->getThreadCurrentlyControllingManager();

	state->setIoFinishedBeforeLogging(false);	// in a next I/O the thread will be able to be invalidated again

	if (state->inIoLearningPhase()) {
		// LEARNING	PHASE

		// Compensate for time-out handling
		short options = 0;
		if (state->getTimeout() != -1) {
			// @deprecated?
			//cout << state->getName() << " in the code to deal socketRead timeouts " << state->getTimeout() << endl;
			state->rollbackFromTimeOutValue();
			state->addElapsedTime(actualIoDuration);
			stateManager->updateRunnableQueue();
			options = ThreadManager::SUSPEND_OPT_THREAD_ALREADY_IN_LIST;

		} else {
			state->addLocalIoTime(actualIoDuration);
		}

		// Lock released within suspendCurrentThread
		stateManager->suspendCurrentThread(state, 0, ThreadManager::SUSPEND_OPT_DONT_UPDATE_THREAD_TIME | options);

	} else if (state->inIoPredictionPhase()) {

		state->updateEstimatedRealTimeAfterIoPrediction(actualIoDuration);
		stateManager->updateRunnableQueue();
		stateManager->suspendCurrentThread(state, 0, ThreadManager::SUSPEND_OPT_DONT_UPDATE_THREAD_TIME | ThreadManager::SUSPEND_OPT_THREAD_ALREADY_IN_LIST);
	}

	// Used for multicore VEX: all handlers should return to their default timeslices, when all I/O is done
	resetSchedulerTimeslot(state->getAllThreadManagers());

	if (state-> isManagedLocally()) {
		// The manager of the thread might be different than the one before the suspendCurrentThread call
		state->getThreadCurrentlyControllingManager()->commitIoDuration(state, actualIoDuration);
	}

	state->unlockShareResourceAccessKey();
}




