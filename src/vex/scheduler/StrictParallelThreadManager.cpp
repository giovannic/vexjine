#include "StrictParallelThreadManager.h"
#include "ThreadState.h"
#include "Logger.h"
#include "VirtualTimeline.h"
#include "ThreadQueue.h"
#include <signal.h>

bool StrictParallelThreadManager::isThreadTooFarAheadInVirtualTime(VexThreadState *state) {
	return (virtualTimelineController->getHowFarAheadInVirtualTimeTheThreadIs(state) > schedulerTimeslot);
}

/**
 * Scheduler resumes a thread
 * @return: 1 if the thread has been successfully resumed, 0 otherwise
 * @assumption: the lock should be held when this is invoked
 * @lock: locked
 */
void StrictParallelThreadManager::resumeThread(VexThreadState *state) {
	// always resume suspended threads regardless of running variable value

	// 2. Wait for the thread to release its controlling key when it atomically enters wait...

	if (isThreadTooFarAheadInVirtualTime(state) && !isValidToLeapInVirtualTimeTo(state)) {
		bool isEveryoneDisabled = virtualTimelineController->disableCore();
		if (isEveryoneDisabled) {
			virtualTimelineController->updateResumingSuspendedThreadTimestamp(state);
			//tryForwardTimeLeap(state->getEstimatedRealTime());
			state->waitForThreadToBlock();

			lastResumed = state;
			// NOTE: forbidden to set running here - this might lead to locking a locked thread
			virtualTimelineController->updateResumingSuspendedThreadResumedLastTimestamp(state);
			state->onThreadResumeByManager(managerId);//OnLocalTimelineOf(managerId);
			state->setThreadCurrentlyControllingManager(this);
			LOG(logger, logINFO) << " thread " << state->getName() << " RESUMED by " << this->managerId << " as controller at " << state->getResumedLastAt()/1e6 << endl;
			state->signalBlockedThreadToResume();
			state->allowSignalledThreadToResume();
		} else {
			runnableThreads->push(state);
			++blockedByErt;
			limitNextTimeslice();
		}

	} else {
		state->waitForThreadToBlock();

		lastResumed = state;

		// NOTE: forbidden to set running here - this might lead to locking a locked thread
		virtualTimelineController->updateResumingSuspendedThreadResumedLastTimestamp(state);
		state->onThreadResumeByManager(managerId);

		//state->onThreadResumeByManagerOnLocalTimelineOf(managerId);
		state->setThreadCurrentlyControllingManager(this);
		LOG(logger, logINFO) << " thread " << state->getName() << " RESUMED by " << this->managerId << " as controller at " << state->getResumedLastAt()/1e6 << endl;

		state->signalBlockedThreadToResume();
		state->allowSignalledThreadToResume();
	}
	//vtflog(managerDebug & mypow2(14), managerLogfile,	"SCHEDULER*THREAD: resumeThread: RESUMED SUCCESFFULLY %s (%ld) after receiving ack\n", state->getName(), state->tid );

}

bool StrictParallelThreadManager::noThreadsToSchedule() {
	if (ThreadManager::noThreadsToSchedule()) {
		virtualTimelineController->disableCore();
		return true;
	} else {
		return false;
	}
}


/*
 * We avoid speeding up the waiting time of loosely synchronized schedulers after model simulation.
 * In this way we ensure that they remain in sync.
 */
void StrictParallelThreadManager::afterModelSimulation() {

}


/**
 * Internal protected by suspending lock thread suspend method
 * @return: true if the thread was suspended or no thread was found running, false otherwise
 * @lock: locking/unlocking runningThread lock
 */
bool StrictParallelThreadManager::_suspendThread(VexThreadState *state) {
	if (state != NULL && state == getRunningThread()) {
		//vtflog(managerDebug & mypow2(14), managerLogfile, "***SUSPENDING THREAD %s (%lld) at %lld\nSCHEDULER THREAD: before sending suspend signal to %s (%lld)\n", state->getName(), state->tid, getRealTime(jvmti_env), state->getName(), state->tid);
		// 1. Send async signal to make thread call its suspend routine
		if (virtualTimelineController->shouldBlockCoreProgress(schedulerTimeslot)) {
			state->forceThreadSuspend();

			// TODO: this is only used for logging purposes - remove
			VirtualTimeSnapshot vts;
			virtualTimelineController->getCurrentTimeSnapshot(vts);
			LOG(logger, logINFO) << "TIME UPDATE: SYNCHRONIZATION2 - controller " << managerId << " is going to wait at " << vts.getGlobalTime()/1e6 << " " << vts.getLocalTimelineId() << " " << vts.getLocalTime()/1e6 <<  endl;
//			VirtualTimeSnapshot vts;
//			virtualTimelineController->getCurrentTimeSnapshot(vts);
//			cout << "***********" << managerId << " handler has to forcibly suspend thread \""<< state->getName() << "\" with at GVT=" << vts.getGlobalTime()/1e6 << "ms LocalId=" << vts.getLocalTimelineId() << " LocalTime=" << vts.getLocalTime()/1e6 << "ms " << endl;
			limitNextTimeslice();
		}

		if (tkill(state->getId(), SIGHUP) == 0) {
			return isSignalledThreadGoingToSuspend(state);

		} else {
			printError("suspendThread: problem sending async signal");
			runnableThreads->erase(state);
			return false;
		}
	} else {
		return true; // since no thread was found you can schedule someone else
	}
}



void StrictParallelThreadManager::suspendRunningResumeNext() {
	VexThreadState *currentRunningThread = getRunningThread();

	if ((currentRunningThread == NULL || suspendThread(currentRunningThread))) {

		if (virtualTimelineController->shouldBlockCoreProgress(schedulerTimeslot)) {

			// TODO: this is only used for logging purposes - remove
			VirtualTimeSnapshot vts;
			virtualTimelineController->getCurrentTimeSnapshot(vts);
			LOG(logger, logINFO) << "TIME UPDATE: SYNCHRONIZATION2 - controller " << managerId << " is going to wait at " << vts.getGlobalTime()/1e6 << " " << vts.getLocalTimelineId() << " " << vts.getLocalTime()/1e6 <<  endl;

//			decreaseSchedulerSleepingTimeBy = localTimelinesDifferenceFromGlobalVirtualTime / 2;
			++synchronizationWaits;

			limitNextTimeslice();

		} else {

			VexThreadState *nextThread = runnableThreads->top();
//			ThreadState *nextThread = runnableThreads->getNext(this);
			if (nextThread != NULL) {
				continueThread(nextThread);
			} else {
				LOG(logger, logINFO) << "NO THREAD LEFT TO EXECUTE IN CORE " << managerId << endl;
				virtualTimelineController->disableCore();
			}
			++normalContinuations;
		}

	}

}

//void StrictParallelThreadManager::onIntentionToKeepOnRunning(VexThreadState *state) {
//
//	// check whether the thread that wants to keep on running violates the strict synchronization principle
//	if (virtualTimelineController->shouldBlockCoreProgress(schedulerTimeslot)) {
//		state->forceThreadSuspend();		// thread has to be suspended even if it doesn't know why :)
//		VirtualTimeSnapshot vts;
//		virtualTimelineController->getCurrentTimeSnapshot(vts);
//		cout << "***********" << managerId << " handler is so strict that it forces willing thread \""<< state->getName() << "\" with ert = " << state->getEstimatedRealTime()/1e6 << "ms to suspend anyway at GVT=" << vts.getGlobalTime()/1e6 << "ms LocalId=" << vts.getLocalTimelineId() << " LocalTime=" << vts.getLocalTime()/1e6 << "ms " << endl;
//		_suspendThread(state);
//
////		forceMinimumTimeslotSelection = true;
////		decreaseSchedulerSleepingTimeBy = 0.9 * schedulerTimeslot;
//	} else {
//
//		// The thread state should not change (RUNNING->RUNNING or IN_SYSTEM_CALL->IN_SYSTEM_CALL)
//		lockRunningThread();
//		runningThread = state;
//		state->setThreadCurrentlyControllingManager(this);
//		unlockRunningThread();
//		LOG(logger, logINFO) << "Manager " << managerId << " has now thread \""<< state->getName() << "\" ("<< state->getEstimatedRealTime()/1e6 << ") as running" << endl;// at " << getAllCallingMethodsUntil(5) << endl;
//
//		decreaseSchedulerSleepingTimeBy += schedulerTimeslot/2;	// NEW in 415: limit timeslot, if thread decides to keep on running
////	}
//}








std::string StrictParallelThreadManager::getStats() {
	stringstream str;
	double sum = synchronizationWaits + normalContinuations;
	str << "Scheduler #,Normal-Blocked,Normal-Blocked %,Blocked),SynchronizationWaits,SynchronizationWaits %, Blocked, Blocked %" << endl;
	str << managerId << "," << (normalContinuations-blockedByErt) << "," << ((double)(normalContinuations-blockedByErt)/sum) << "," <<synchronizationWaits << "," << ((double)synchronizationWaits/sum) << "," << blockedByErt << ","<< ((double)blockedByErt/normalContinuations);
	return str.str();
}


StrictParallelThreadManager::~StrictParallelThreadManager() {

}
