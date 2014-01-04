/*
 * VirtualTimeline.cpp
 *
 *  Created on: 28 Jun 2010
 *      Author: nb605
 */

#include "TimeLogger.h"
#include "ThreadState.h"
#include "Time.h"
#include "VirtualTimeline.h"

#include <climits>
#include <cassert>

VirtualTimeline::VirtualTimeline() {
//	pthread_spin_init(&spinlock, 0);
	pthread_mutex_init(&mutex, NULL);
	reset();
}
void VirtualTimeline::reset() {
	globalVirtualTime = 0;
	unknownParallelTime = 0;
}
VirtualTimeline::~VirtualTimeline() {
//	pthread_spin_destroy(&spinlock);
	pthread_mutex_destroy(&mutex);
}

void VirtualTimeline::lock() {
//	pthread_spin_lock(&spinlock);
	pthread_mutex_lock(&mutex);
}

void VirtualTimeline::unlock() {
//	pthread_spin_unlock(&spinlock);
	pthread_mutex_unlock(&mutex);
}

void VirtualTimeline::leapForwardTo(const long long &forwardTime) {
	lock();			// locks used for stupid native waiting threads

	// Since the unknownParallelTime has already been added to the GVT
	// we have to include it to the first thread that will try to commit its time
	// after the unknownParallelTime has been increased.
	// Had we not done this, then the forwardTime (which is calculated as the
	// virtual time progress from lastResumed + localTime = forwardTime)
	// would overlap with the unknownParallelTime (= lastIdentifiedAsNw + rndCpuDuration)
	if (forwardTime + unknownParallelTime > globalVirtualTime) {
		globalVirtualTime = forwardTime + unknownParallelTime;
	}
	unknownParallelTime = 0;
	unlock();
}

// This is used to add to the GVT a duration that does not belong to any VEX monitored thread (f.e. GC time)
void VirtualTimeline::addTimeOfUnknownRealTimeDurationAndSynchronize(const long long &forwardTime) {
	lock();
	globalVirtualTime += forwardTime;
	unlock();
}


// This is used to add to the GVT a duration that belongs to a VEX monitored thread that has
// been allowed to run loosely for a while, due to native waiting issues.
// The difference from the addTimeOfUnknownRealTimeDurationAndSynchronize method is
// that this time is only to be added to a (here THE) single processor,
// while in the other case it should be added to all cores that would have to synchronise
// after that as well
void VirtualTimeline::addTimeOfThreadExecutingAtUnknownTime(const long long &forwardTime) {
	lock();
	globalVirtualTime += forwardTime;
	unknownParallelTime += forwardTime;
	unlock();
}


//
//void VirtualTimeline::printStats(char *filename) {
//	if (filename == NULL) {
//		std::cout << std::endl;
//	} else {
//		std::filebuf fb;
//		fb.open(filename, std::ios::out);
//		std::ostream os(&fb);
//		fb.close();
//	}
//}


MulticoreVirtualTimeline::~MulticoreVirtualTimeline() {
	delete[] localCoreVirtualTimes;
}
void MulticoreVirtualTimeline::leapForwardTo(const long long &time, const int &localTimelineId) {
	lock();
	if (localCoreVirtualTimes[localTimelineId] < time) {
		localCoreVirtualTimes[localTimelineId] = time;
	}
	updateGlobalTimeTo(localCoreVirtualTimes[localTimelineId]);
	unlock();
}
void MulticoreVirtualTimeline::addTimeOfUnknownRealTimeDurationAndSynchronize(const long long &forwardTime) {
	lock();

	// Find maximum local time, add the forwardTime duration to it
	// and update all timelines and the GVT to that virtual timestamp
	long long max = localCoreVirtualTimes[0];
	for (int i = 1; i < processors; i++) {
		if (localCoreVirtualTimes[i] > max) {
			max = localCoreVirtualTimes[i];
		}
	}
	max += forwardTime;
	for (int i = 0; i < processors; i++) {
		localCoreVirtualTimes[i] = max;
	}
	updateGlobalTimeTo(max);
	unlock();
}


void MulticoreVirtualTimeline::addTimeOfThreadExecutingAtUnknownTime(const long long &forwardTime) {
	lock();

	// Find timeline with minimum local virtual time
	long long min = localCoreVirtualTimes[0];
	int minimumLocalTimelineId = 0;
	for (int i = 1; i< processors; i++) {
		if (min > localCoreVirtualTimes[i]) {
			min = localCoreVirtualTimes[i];
			minimumLocalTimelineId = i;
		}
	}

	// add the time of the native waiting thread to that timeline:
	// it makes (some) sense that the native waiting thread was executed at
	// the core with the least load - if was idle even better!
	localCoreVirtualTimes[minimumLocalTimelineId] += forwardTime;
	updateGlobalTimeTo(localCoreVirtualTimes[minimumLocalTimelineId]);
	unlock();
}

// Just make sure that GVT moves forward: no further management
void MulticoreVirtualTimeline::updateGlobalTimeTo(const long long &time) {
	if (globalVirtualTime < time) {
		globalVirtualTime = time;
	}
}

bool MulticoreVirtualTimeline::shouldBlockCoreProgress(const long long &schedulerTimeslot, const int &localTimelineId) {
	bool ret = false;
	lock();
	if (localCoreVirtualTimes[localTimelineId] - globalVirtualTime > schedulerTimeslot) {
		unlock();
		bool areAllCoresDisabled = disableLocalTimeline(localTimelineId);
		ret = !areAllCoresDisabled;	// if some cores are running then return true -> this core should not make pr
	} else {
		unlock();
	}
	return ret;
}
bool MulticoreVirtualTimeline::disableLocalTimeline(const int &localTimelineId) {
	return false;
}

long long const &MulticoreVirtualTimeline::getLocalTimelineActivationTime(const long long &threadTime, const int &localTimelineId) {
	lock();
	if (localCoreVirtualTimes[localTimelineId] < threadTime) {
		localCoreVirtualTimes[localTimelineId] = threadTime;
	}
	updateGlobalTimeTo(threadTime);
	unlock();
	return localCoreVirtualTimes[localTimelineId];
}

void MulticoreVirtualTimeline::reset() {
	lock();
	globalVirtualTime = 0;
	for (int i =0 ; i<processors;i++) {
		localCoreVirtualTimes[i] = 0;//LLONG_MAX;
	}
	unlock();
}

long long &MulticoreVirtualTimeline::getUpdatedGlobalTime() {
	return globalVirtualTime;
}

long long &DisablingMulticoreVirtualTimeline::getUpdatedGlobalTime() {
	lock();
	long long min = LLONG_MAX;
	for (int i = 0; i < processors; i++) {
		if (localCoreActive[i] && localCoreVirtualTimes[i] < min) {
			min = localCoreVirtualTimes[i];
		}
	}
	if (min != LLONG_MAX) {
		globalVirtualTime = min;
//		for (int i = 0; i < processors; i++) {
//			if (!localCoreActive[i]) {
//				localCoreVirtualTimes[i] = globalVirtualTime;
//			}
//		}
	}
	unlock();
	return globalVirtualTime;
}


void DisablingMulticoreVirtualTimeline::reset() {
	MulticoreVirtualTimeline::reset();
	for (int i =0 ; i<processors;i++) {
		localCoreActive[i]=false;
	}
}
//void DisablingMulticoreVirtualTimeline::leapForwardTo(const long long &time, const int &localTimelineId) {
//	lock();
//	if (localCoreVirtualTimes[localTimelineId] < time) {
//		localCoreVirtualTimes[localTimelineId] = time;
//	}
//	updateGlobalTime(localCoreVirtualTimes[localTimelineId]);
//	unlock();
//}
//void DisablingMulticoreVirtualTimeline::addTimeOfUnknownRealTimeDurationAndSynchronize(const long long &forwardTime, const int &id) {
//	lock();
//	localCoreVirtualTimes[localTimelineId] += forwardTime;
//	updateGlobalTime(localCoreVirtualTimes[localTimelineId]);
//	unlock();
//}

// GVT must always be the least time from all cores
void DisablingMulticoreVirtualTimeline::updateGlobalTimeTo(const long long &time) {
	long long min = LLONG_MAX;
	for (int i = 0; i < processors; i++) {
		if (localCoreActive[i] && localCoreVirtualTimes[i] < min) {
			min = localCoreVirtualTimes[i];
		}
	}
	if (min != LLONG_MAX) {
		globalVirtualTime = min;
		// FIX: commented out, because when a local timeline was disabled at a localERT > GVT (global virtual time),
		// then the localERT was reverted to the GVT. This essentially replaced the progress that had already taken place,
		// which led the localERT to the higher value in the first place.
		// The idea of a disabled core is to wait in its localERT, until the GVT is within schedulerTimeslice distance from it.
//		for (int i = 0; i < processors; i++) {
//			if (!localCoreActive[i]) {
//				localCoreVirtualTimes[i] = min;
//			}
//		}
	}
}


bool DisablingMulticoreVirtualTimeline::disableLocalTimeline(const int &localTimelineId) {
	lock();
	localCoreActive[localTimelineId] = false;
	for (int i = 0; i<processors; i++) {
		if (localCoreActive[i]) {
			unlock();
			return false;	// not all schedulers are disabled
		}
	}
	unlock();
	return true;
}

long long const &DisablingMulticoreVirtualTimeline::getLocalTimelineActivationTime(const long long &threadTime, const int &localTimelineId) {
	lock();
	localCoreActive[localTimelineId] = true;
	if (localCoreVirtualTimes[localTimelineId] < threadTime) {
		localCoreVirtualTimes[localTimelineId] = threadTime;
	}
	updateGlobalTimeTo(threadTime);
	unlock();
	return localCoreVirtualTimes[localTimelineId];
}

DisablingMulticoreVirtualTimeline::~DisablingMulticoreVirtualTimeline() {
	delete[] localCoreActive;
}


VirtualTimeSnapshot & VirtualTimeSnapshot::operator-=(const VirtualTimeSnapshot &rhs) {
	if (localTimelineId == rhs.localTimelineId) {
		globalTime -= rhs.globalTime;
		localTime -= rhs.localTime;
	} else {
		cout << "Tried to compare time differences between different cores" << endl;
		assert(false);
	}
	return *this;
}




void SingleVirtualTimelineController::commitCpuTimeProgress(VexThreadState *state) {
	Timers *threadTimer = state->getThreadTimers();
	//virtualTimeline->leapForwardTo(threadTimer->getEstimatedRealTime());//threadTimer->getCurrentVirtualTimestampOfRunningThread());
	virtualTimeline->leapForwardTo(threadTimer->getCurrentVirtualTimestampOfRunningThread());
//	virtualTimeline->leapForwardTo(threadTimer->getEstimatedRealTime());


//	virtualTimeline->leapForwardTo(threadTimer->getEstimatedRealTime());

//	cout << state->getName() << ",leaping to cpu time," << threadTimer->getEstimatedRealTime()/1000000 << ",at," << virtualTimeline->getGlobalTime()/1000000 << state->getResumedLastAt()/1000000 << endl;
}


void SingleVirtualTimelineController::commitIoTimeProgress(VexThreadState *state, const long long &actualIoDuration) {
	Timers *threadTimer = state->getThreadTimers();
//	cout << "commitIoTimeProgress: " << state->getName() << ",leaping to IO time," << threadTimer->getEstimatedRealTime()<< ",at," << virtualTimeline->getGlobalTime() << " = " << (threadTimer->getEstimatedRealTime() - virtualTimeline->getGlobalTime()) << " when actually " << actualIoDuration << endl;
	threadTimer->updateIoWaitingTimeFrom(virtualTimeline->getGlobalTime(), actualIoDuration);
	virtualTimeline->leapForwardTo(threadTimer->getEstimatedRealTime());


//	cout << state->getName() << ",leaping to IO time," << threadTimer->getEstimatedRealTime()/1000000 << ",at," << virtualTimeline->getGlobalTime()/1000000 << "," << state->getResumedLastAt()/1000000 << endl;
//	cout << "after IO: " << state->getName() << " " << threadTimer->getResumedLastAt()/1000000 << " + " << threadTimer->getLocalTimeSinceLastResume()/1000000 << " = " << threadTimer->getCurrentVirtualTimestampOfRunningThread()/1000000 << ", ERT:" << threadTimer->getEstimatedRealTime()/1000000 << " " << virtualTimeline->getGlobalTime()/1000000 << endl;


}

void SingleVirtualTimelineController::commitTimedWaitingProgress(VexThreadState *state) {
	Timers *threadTimer = state->getThreadTimers();
	threadTimer->updateWaitingTimeFrom(virtualTimeline->getGlobalTime());
	virtualTimeline->leapForwardTo(threadTimer->getEstimatedRealTime());
}

void SingleVirtualTimelineController::commitNativeWaitingProgress(VexThreadState *state) {
	Timers *threadTimer = state->getThreadTimers();
//	virtualTimeline->addTimeOfUnknownRealTimeDurationAndSynchronize(threadTimer->getLocalTimeSinceLastResume());

//cout << state->getName() << " " << state->getId() << " NOT NOT commiting its nw time " << threadTimer->getLocalTimeSinceLastResume()/1000000 << " in method " << state->getCurrentMethodId() << " at " << getGlobalTime()/1000000 << endl;
	virtualTimeline->addTimeOfThreadExecutingAtUnknownTime(threadTimer->getLocalTimeSinceLastResume());

	//virtualTimeline->leapForwardTo(threadTimer->getEstimatedRealTime());

//	cout << state->getName() << ",adding nw time," << threadTimer->getLocalTimeSinceLastResume()/1000000 << " at " << virtualTimeline->getGlobalTime()/1000000 << "," << threadTimer->getResumedLastAt()/1000000 << endl;
	//cout << state->getName() << " would add nw time " << threadTimer->getResumedLastAt()/1000000 << "+" << threadTimer->getLocalTimeSinceLastResume()/1000000 << " i prefer leaping to " << threadTimer->getEstimatedRealTime()/1000000 << " at " << virtualTimeline->getGlobalTime()/1000000 << endl;

}

void SingleVirtualTimelineController::commitBackgroundLoadExecutionTime(const long long &backgroundLoadExecutionTime) {
	virtualTimeline->addTimeOfUnknownRealTimeDurationAndSynchronize(backgroundLoadExecutionTime);
}


void SingleVirtualTimelineController::commitModelSimulationProgress(VexThreadState *state) {
	Timers *threadTimer = state->getThreadTimers();
	if (!state->isModelTimedWaiting()) {
		virtualTimeline->leapForwardTo(threadTimer->getCurrentVirtualTimestampOfRunningThread());
	//	virtualTimeline->leapForwardTo(threadTimer->getEstimatedRealTime());
	//	threadTimer->leapForwardTo(virtualTimeline->getGlobalTime());
	}
}

void SingleVirtualTimelineController::commitTimedOutIoProgress(VexThreadState *state) {
	cout << "Invoking  SingleVirtualTimelineController::commitTimedOutIoProgress" << endl;
	assert(false);
	Timers *threadTimer = state->getThreadTimers();
	virtualTimeline->leapForwardTo(threadTimer->getEstimatedRealTime());
}


//void SingleVirtualTimelineController::updateNewThreadTimestamp(VexThreadState *state) {
////	Timers *threadTimer = state->getThreadTimers();
////	threadTimer->setEstimatedRealTime(virtualTimeline->getGlobalTime());
//}
void SingleVirtualTimelineController::updateResumingSuspendedThreadTimestamp(VexThreadState *state) {
	Timers *threadTimer = state->getThreadTimers();
	threadTimer->leapForwardTo(virtualTimeline->getGlobalTime());
}
void SingleVirtualTimelineController::updateResumingSuspendedThreadResumedLastTimestamp(VexThreadState *state) {
	Timers *threadTimer = state->getThreadTimers();
	threadTimer->updateTimesAfterResuming(virtualTimeline->getGlobalTime());
}
void SingleVirtualTimelineController::updateTimedOutWaitingThreadTimestamp(VexThreadState *state) {
	Timers *threadTimer = state->getThreadTimers();
	threadTimer->leapForwardTo(virtualTimeline->getGlobalTime());
}
void SingleVirtualTimelineController::updateInterruptedWaitingThreadTimestamp(VexThreadState *state, const long long &interruptTime) {
	Timers *threadTimer = state->getThreadTimers();
	if (interruptTime == 0 && !threadTimer->shouldNotUpdateToGlobalTime()) {
		threadTimer->setEstimatedRealTime(virtualTimeline->getGlobalTime());
	} else {
		threadTimer->setEstimatedRealTime(interruptTime);
	}
}
void SingleVirtualTimelineController::updateBlockedThreadTimestamp(VexThreadState *state) {
	Timers *threadTimer = state->getThreadTimers();
	long long currentGlobalTime = virtualTimeline->getGlobalTime();
	threadTimer->leapForwardTo(currentGlobalTime);
	threadTimer->setResumedLastAt(currentGlobalTime);
	threadTimer->getAndResetLocalTime();
}

long long SingleVirtualTimelineController::getHowFarAheadInVirtualTimeTheThreadIs(VexThreadState *state) {
	Timers *threadTimer = state->getThreadTimers();
	return threadTimer->getEstimatedRealTime() - virtualTimeline->getGlobalTime();
}
bool SingleVirtualTimelineController::disableCore() {
	return false;
}
bool SingleVirtualTimelineController::shouldBlockCoreProgress(const long long &schedulerTimeslot) {
	return true;
}
long long const &SingleVirtualTimelineController::getGlobalTime() {
	return virtualTimeline->getGlobalTime();
}

void SingleVirtualTimelineController::getCurrentTimeSnapshot(VirtualTimeSnapshot &vts) {
	vts = virtualTimeline->getGlobalTime();
}



void PassiveVirtualTimelineController::commitCpuTimeProgress(VexThreadState *state) {
	Timers *threadTimer = state->getThreadTimers();
	virtualTimeline->addTimeOfUnknownRealTimeDurationAndSynchronize(threadTimer->getAndResetLocalTime());
	threadTimer->setEstimatedRealTime(virtualTimeline->getGlobalTime());
}

void PassiveVirtualTimelineController::commitIoTimeProgress(VexThreadState *state, const long long &actualIoDuration) {
	Timers *threadTimer = state->getThreadTimers();
	threadTimer->updateIoWaitingTimeFrom(virtualTimeline->getGlobalTime(), actualIoDuration);
	virtualTimeline->addTimeOfUnknownRealTimeDurationAndSynchronize(threadTimer->getAndResetLocalTime());
	threadTimer->setEstimatedRealTime(virtualTimeline->getGlobalTime());
}

/*
void MultipleVirtualTimelinesController::commitCpuTimeProgress(ThreadState *state) {
	multipleTimelines->leapForwardTo(threadTimer->getCurrentVirtualTimestampOfRunningThread());
	//tryForwardTimeLeap(state->getResumedLastAt()+state->getLocalTimeSinceLastResume());
//	localTimeLines->tryForwardTimeLeap(state->getResumedLastAt()+state->getLocalTimeSinceLastResume(), managerId);
}
void MultipleVirtualTimelinesController::commitIoTimeProgress(ThreadState *state) {
//	localTimeLines->tryForwardTimeLeap(state->getEstimatedRealTime(), managerId);
}
void MultipleVirtualTimelinesController::commitTimedWaitingProgress(ThreadState *state) {
//	localTimeLines->tryForwardTimeLeap(state->getEstimatedRealTime(), managerId);

}
void MultipleVirtualTimelinesController::commitNativeWaitingProgress(ThreadState *state) {

}
*/
void MultipleVirtualTimelinesController::commitCpuTimeProgress(VexThreadState *state) {
	Timers *threadTimer = state->getThreadTimers();
	multipleTimelines->leapForwardTo(threadTimer->getCurrentVirtualTimestampOfRunningThread(), localTimelineId);
//	multipleTimelines->leapForwardTo(threadTimer->getEstimatedRealTime(), localTimelineId);
}
void MultipleVirtualTimelinesController::commitIoTimeProgress(VexThreadState *state, const long long &actualIoDuration) {
	Timers *threadTimer = state->getThreadTimers();
	threadTimer->updateIoWaitingTimeFrom(multipleTimelines->getLocalTime(localTimelineId), actualIoDuration);
	multipleTimelines->leapForwardTo(threadTimer->getEstimatedRealTime(), localTimelineId);
}

void MultipleVirtualTimelinesController::commitTimedWaitingProgress(VexThreadState *state) {
	Timers *threadTimer = state->getThreadTimers();

//	cout << "commiting multiple timed-waiting progress " << (threadTimer->getEstimatedRealTime() - multipleTimelines->getLocalTime(localTimelineId)) << endl;
	threadTimer->updateWaitingTimeFrom(multipleTimelines->getLocalTime(localTimelineId));
	multipleTimelines->leapForwardTo(threadTimer->getEstimatedRealTime(), localTimelineId);
}

void MultipleVirtualTimelinesController::commitNativeWaitingProgress(VexThreadState *state) {
	Timers *threadTimer = state->getThreadTimers();
	multipleTimelines->addTimeOfThreadExecutingAtUnknownTime(threadTimer->getLocalTimeSinceLastResume());
}

void MultipleVirtualTimelinesController::commitBackgroundLoadExecutionTime(const long long &backgroundLoadExecutionTime) {
	multipleTimelines->addTimeOfUnknownRealTimeDurationAndSynchronize(backgroundLoadExecutionTime);
}

void MultipleVirtualTimelinesController::commitModelSimulationProgress(VexThreadState *state) {
	Timers *threadTimer = state->getThreadTimers();
	if (!state->isModelTimedWaiting()) {
		multipleTimelines->leapForwardTo(threadTimer->getEstimatedRealTime(), localTimelineId);
		threadTimer->leapForwardTo(multipleTimelines->getLocalTime(localTimelineId));
	}

}

void MultipleVirtualTimelinesController::commitTimedOutIoProgress(VexThreadState *state) {
	cout << "Invoking  MultipleVirtualTimelinesController::commitTimedOutIoProgress" << endl;
	assert(false);
	Timers *threadTimer = state->getThreadTimers();
	multipleTimelines->leapForwardTo(threadTimer->getEstimatedRealTime(), localTimelineId);
}





//void MultipleVirtualTimelinesController::updateNewThreadTimestamp(VexThreadState *state) {
////	Timers *threadTimer = state->getThreadTimers();
////	threadTimer->setEstimatedRealTime(multipleTimelines->getLocalTime(localTimelineId));
//}
void MultipleVirtualTimelinesController::updateResumingSuspendedThreadTimestamp(VexThreadState *state) {
	Timers *threadTimer = state->getThreadTimers();
	//threadTimer->leapForwardTo(virtualTimeline->getGlobalTime());
	//threadTimer->setEstimatedRealTime(multipleTimelines->getLocalTime(localTimelineId));
	long long currentLocalTimelineTime = multipleTimelines->getLocalTime(localTimelineId);
	threadTimer->leapForwardTo(currentLocalTimelineTime);
	if (currentLocalTimelineTime != threadTimer->getEstimatedRealTime()) {
		multipleTimelines->leapForwardTo(threadTimer->getEstimatedRealTime(), localTimelineId);
	}

	assert(multipleTimelines->getLocalTime(localTimelineId) == threadTimer->getEstimatedRealTime());
}
void MultipleVirtualTimelinesController::updateResumingSuspendedThreadResumedLastTimestamp(VexThreadState *state) {
	//		long long localSchedulerTime = virtualTimelineController->activateAndGetLocalTimeOfScheduler(managerId);
	//		virtualTimelineController->activateManager(managerId);
	//		if (estimatedRealTime < localSchedulerTime) {
	//			estimatedRealTime = localSchedulerTime;
	//		} else {
	//			virtualTimelineController->setLocalTimeOfScheduler(managerId, estimatedRealTime);
	//		}
	Timers *threadTimer = state->getThreadTimers();
	long long activationTime = multipleTimelines->getLocalTimelineActivationTime(threadTimer->getEstimatedRealTime(), localTimelineId);
//	threadTimer->setEstimatedRealTime(activationTime);
	threadTimer->leapForwardTo(activationTime);
	threadTimer->updateTimesAfterResuming(activationTime);

	// BUG: small glitches due to other (Native Waiting) threads committing at the same time
//	assert(multipleTimelines->getLocalTime(localTimelineId) == threadTimer->getEstimatedRealTime());
}
void MultipleVirtualTimelinesController::updateInterruptedWaitingThreadTimestamp(VexThreadState *state, const long long &interruptTime) {
	Timers *threadTimer = state->getThreadTimers();
	if (interruptTime == 0 && !threadTimer->shouldNotUpdateToGlobalTime()) {
		threadTimer->setEstimatedRealTime(multipleTimelines->getLocalTime(localTimelineId));
	} else {
		threadTimer->setEstimatedRealTime(interruptTime);
	}

	// BUG: small glitches due to other (Native Waiting) threads committing at the same time
//	assert(multipleTimelines->getLocalTime(localTimelineId) == threadTimer->getEstimatedRealTime());
}
void MultipleVirtualTimelinesController::updateTimedOutWaitingThreadTimestamp(VexThreadState *state) {
	Timers *threadTimer = state->getThreadTimers();
	threadTimer->leapForwardTo(multipleTimelines->getLocalTime(localTimelineId));
//	if ((multipleTimelines->getLocalTime(localTimelineId) != threadTimer->getEstimatedRealTime())) {
//		cout << multipleTimelines->getLocalTime(localTimelineId) << " != " << threadTimer->getEstimatedRealTime() << endl;
//		assert(multipleTimelines->getLocalTime(localTimelineId) == threadTimer->getEstimatedRealTime());
//	}

}
void MultipleVirtualTimelinesController::updateBlockedThreadTimestamp(VexThreadState *state) {
	Timers *threadTimer = state->getThreadTimers();
//	multipleTimelines->updateGlobalTimeTo(threadTimer->getEstimatedRealTime());
//	multipleTimelines->getLocalTimelineActivationTime(threadTimer->getEstimatedRealTime(), localTimelineId);
	long long currentGlobalTime = multipleTimelines->getUpdatedGlobalTime();//->getLocalTime(localTimelineId);
	threadTimer->leapForwardTo(currentGlobalTime);
	threadTimer->setResumedLastAt(currentGlobalTime);
	threadTimer->getAndResetLocalTime();
//	assert(multipleTimelines->getLocalTime(localTimelineId) == threadTimer->getEstimatedRealTime());
}

long long MultipleVirtualTimelinesController::getHowFarAheadInVirtualTimeTheThreadIs(VexThreadState *state) {
	Timers *threadTimer = state->getThreadTimers();
	return threadTimer->getEstimatedRealTime() - multipleTimelines->getGlobalTime(); //multipleTimelines->getLocalTime(localTimelineId);
	// in strict manager we were finding the difference between state->ERT and the global time
}
bool MultipleVirtualTimelinesController::shouldBlockCoreProgress(const long long &schedulerTimeslot) {
	return multipleTimelines->shouldBlockCoreProgress(schedulerTimeslot, localTimelineId);
}

bool MultipleVirtualTimelinesController::disableCore() {
	return multipleTimelines->disableLocalTimeline(localTimelineId);
}
long long const &MultipleVirtualTimelinesController::getGlobalTime() {
	return multipleTimelines->getLocalTime(localTimelineId);
}

void MultipleVirtualTimelinesController::getCurrentTimeSnapshot(VirtualTimeSnapshot &vts) {
	vts.set(multipleTimelines->getGlobalTime(), multipleTimelines->getLocalTime(localTimelineId), localTimelineId);
}



VirtualTimeForwardLeapSnapshot::VirtualTimeForwardLeapSnapshot(bool _allowed, long long _timeRemaining, long _timeout, long long _threadERT, int _underCreation, const struct vex_and_system_states &_vass) {
	allowed = _allowed;
	timeRemaining = _timeRemaining;
	timeout = _timeout;
	threadERT = _threadERT;
	underCreation =  _underCreation;
	vass = new struct vex_and_system_states(_vass);
}

VirtualTimeForwardLeapSnapshot::~VirtualTimeForwardLeapSnapshot() {
	delete vass;
}

std::ostream &operator<<(std::ostream &outs, const VirtualTimeForwardLeapSnapshot &record) {
	outs << "VL" << "\t" << "rem" << "\t" << "T/o" << "\t" << "ERT" << "\t" <<
			"SPN"<< "\t" << "W" << "\t" <<
			"NW"  << "\t" << "IO" << "\t" <<
			"IO-bl" << "\t" << "SUSP" << "\t" << "RUN" << "\t" <<
			"Join" << "\t" << "Reg" << "\t" << "Awa" << "\t" <<
			"Mod" << "\t" <<
			"R" << " " << "S"<< " " << "D" << " " <<
			"T" << " " << "Z" << " " << endl;

	outs << record.allowed << "\t" << record.timeRemaining/1e6 << "\t" << record.timeout/1e6 << "\t" << record.threadERT/1e6 << "\t" <<
			record.underCreation << "\t" << record.vass->waitingThreads << "\t" <<
			record.vass->nwThreads << "\t" << record.vass->activeInIoThreads << "\t" <<
			record.vass->blockedInIoThreads << "\t" << record.vass->suspended << "\t" <<
			record.vass->running << "\t" <<
			record.vass->isAwakeningAfterJoin << "\t" << record.vass->isRegistering << "\t" << record.vass->isAwakenInVex << "\t" <<
			record.vass->isWaitingRealCodeToCompleteAfterModelSimulation << "\t" <<
			record.vass->systemRunning << " " <<
			record.vass->systemSleeping << " " << record.vass->systemD << " " <<
			record.vass->systemStopped << " " << record.vass->systemZombies << " " <<
			std::endl;

	return outs;
};




void StatisticsEnabledVirtualTimelineController::commitCpuTimeProgress(VexThreadState *state) {
	VirtualTimeSnapshot before; controller->getCurrentTimeSnapshot(before);
	controller->commitCpuTimeProgress(state);
	VirtualTimeSnapshot after; controller->getCurrentTimeSnapshot(after);
	timeLogging->logCpuTime(state, &before, &after);
}
void StatisticsEnabledVirtualTimelineController::commitIoTimeProgress(VexThreadState *state, const long long &actualIoProgress) {
	VirtualTimeSnapshot before; controller->getCurrentTimeSnapshot(before);
	controller->commitIoTimeProgress(state, actualIoProgress);
	VirtualTimeSnapshot after; controller->getCurrentTimeSnapshot(after);
	timeLogging->logIoTime(state, &before, &after);
}

void StatisticsEnabledVirtualTimelineController::commitTimedWaitingProgress(VexThreadState *state) {
	VirtualTimeSnapshot before; controller->getCurrentTimeSnapshot(before);
	controller->commitTimedWaitingProgress(state);
	VirtualTimeSnapshot after; controller->getCurrentTimeSnapshot(after);
	timeLogging->logTimedWaitingTime(state, &before, &after);
}

void StatisticsEnabledVirtualTimelineController::commitNativeWaitingProgress(VexThreadState *state) {
	VirtualTimeSnapshot before; controller->getCurrentTimeSnapshot(before);
	controller->commitNativeWaitingProgress(state);
	VirtualTimeSnapshot after; controller->getCurrentTimeSnapshot(after);
	timeLogging->logNativeWaitingTime(state, &before, &after);
}

void StatisticsEnabledVirtualTimelineController::commitBackgroundLoadExecutionTime(const long long &backgroundLoadExecutionTime) {
	VirtualTimeSnapshot before; controller->getCurrentTimeSnapshot(before);
	controller->commitBackgroundLoadExecutionTime(backgroundLoadExecutionTime);
	VirtualTimeSnapshot after; controller->getCurrentTimeSnapshot(after);
	timeLogging->logBackgroundLoadExecutionTime(&before, &after);
}


void StatisticsEnabledVirtualTimelineController::commitModelSimulationProgress(VexThreadState *state) {
	VirtualTimeSnapshot before; controller->getCurrentTimeSnapshot(before);
	controller->commitModelSimulationProgress(state);
	VirtualTimeSnapshot after; controller->getCurrentTimeSnapshot(after);
	timeLogging->logModelSimulationTime(state, &before, &after);
}

void StatisticsEnabledVirtualTimelineController::commitTimedOutIoProgress(VexThreadState *state) {
	VirtualTimeSnapshot before; controller->getCurrentTimeSnapshot(before);
	controller->commitTimedOutIoProgress(state);
	VirtualTimeSnapshot after; controller->getCurrentTimeSnapshot(after);
	timeLogging->logTimedOutIoTime(state, &before, &after);
}


void StatisticsEnabledVirtualTimelineController::updateResumingSuspendedThreadTimestamp(VexThreadState *state) {
	controller->updateResumingSuspendedThreadTimestamp(state);
}
void StatisticsEnabledVirtualTimelineController::updateResumingSuspendedThreadResumedLastTimestamp(VexThreadState *state) {
	controller->updateResumingSuspendedThreadResumedLastTimestamp(state);

}
void StatisticsEnabledVirtualTimelineController::updateInterruptedWaitingThreadTimestamp(VexThreadState *state, const long long &interruptTime) {
	controller->updateInterruptedWaitingThreadTimestamp(state, interruptTime);
}
void StatisticsEnabledVirtualTimelineController::updateTimedOutWaitingThreadTimestamp(VexThreadState *state) {
	controller->updateTimedOutWaitingThreadTimestamp(state);
}
void StatisticsEnabledVirtualTimelineController::updateBlockedThreadTimestamp(VexThreadState *state) {
	controller->updateBlockedThreadTimestamp(state);
}

long long StatisticsEnabledVirtualTimelineController::getHowFarAheadInVirtualTimeTheThreadIs(VexThreadState *state) {
	return controller->getHowFarAheadInVirtualTimeTheThreadIs(state);
}
bool StatisticsEnabledVirtualTimelineController::shouldBlockCoreProgress(const long long &schedulerTimeslot) {
	return controller->shouldBlockCoreProgress(schedulerTimeslot);
}

bool StatisticsEnabledVirtualTimelineController::disableCore() {
	return controller->disableCore();
}
long long const &StatisticsEnabledVirtualTimelineController::getGlobalTime() {
	return controller->getGlobalTime();
}
void StatisticsEnabledVirtualTimelineController::getCurrentTimeSnapshot(VirtualTimeSnapshot &vts) {
	controller->getCurrentTimeSnapshot(vts);
}
