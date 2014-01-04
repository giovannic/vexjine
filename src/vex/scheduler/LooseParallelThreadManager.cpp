/*
 * LooseParallelThreadManager.cpp
 *
 *
 *  Created on: 1 Sep 2011
 *      Author: root
 */

#include "LooseParallelThreadManager.h"
#include "ThreadState.h"
#include "Logger.h"
#include "VirtualTimeline.h"
#include "ThreadQueue.h"
#include "Visualizer.h"

#include <cassert>
/**
 * Set a thread as the currently running thread of this processor
 * This method is overridden by the loose synchronization scheduler class,
 * to handle time-scaled threads (state->getTimeScalingFactor()).
 *
 * @lock: assumed manager lock held when entering
 * @return: true if the thread had to be suspended again before releasing
 */
void LooseParallelThreadManager::setRunningThread(VexThreadState *state) {
	if (state != NULL) {

		state->setAwaken(false);
		if (changeThreadStateToRunning(state)) {
			LOG(logger, logINFO) << "Manager " << managerId << " running setRunningThread due to \""<< state->getName() << "\" at " << state->getEstimatedRealTime() / 1e6 << endl;

			virtualTimelineController->updateResumingSuspendedThreadTimestamp(state);
			// Changed order between the time update and the update of the runningThread

			if (state->getTimeScalingFactor() != 1) {
				state->setSynchronizationBarrierTime(schedulerTimeslot);
				LOG(logger, logDEBUG2) << "setRunningThread: " << state->getName() << " SET its blocking barrier to " << state->getEstimatedRealTime()/1e6 << endl;
				if (runnableThreads->find(state)) {
					runnableThreads->update();
				} else {
					runnableThreads->push(state);
				}
			}

		} else {
			short alreadyInRunnable = 0;
			if (runnableThreads->find(state)) {
				alreadyInRunnable = SUSPEND_OPT_THREAD_ALREADY_IN_LIST;
			}

			suspendCurrentThread(state, 0, SUSPEND_OPT_FORCE_SUSPEND | SUSPEND_OPT_DONT_UPDATE_THREAD_TIME | alreadyInRunnable);
		}

	}

}


/*
 * Set a thread into the runnable state:
 * - currentState = SUSPENDED
 * - in the threads list
 *
 * This method is overridden by the loose synchronization scheduler class,
 * to clear the synchronization barrier flag of a time-scaled thread (state->getTimeScalingFactor()).
 * If a thread is already in the threads list it should not be pushed again.
 */
void LooseParallelThreadManager::setSuspendedAndPushToRunnables(VexThreadState *state) {
	if (state != NULL) {

		unsetCurrentThreadFromRunningThread(state);

		if (state->getTimeScalingFactor() == 1) {
			state->setSuspended();
			runnableThreads->push(state);	// put the barrier in the correct spot and then revert to normal time - does not work: what if someone else updates runnableThreads heap
		} else {
			state->clearSynchronizationBarrierTime(schedulerTimeslot);

			LOG(logger, logDEBUG2) << "setSuspendedAndPushToRunnables: " << state->getName() << " CLEARED its blocking barrier to " << state->getEstimatedRealTime()/1e6 << " " << getAllCallingMethodsUntil(4) << endl;
			if (state->isNativeWaiting() || state->isSuspended()) {
				state->setSuspended();
				runnableThreads->push(state);
			} else {
				state->setSuspended();
				runnableThreads->update();
			}
		}

	}
}

/*
 * Notify the scheduler that time-scaling is about to commence or end.
 * If time-scaling starts, then the scheduler adds a synchronization barrier in the runnables queue
 * that will not allow other threads to make progress beyond that point.
 * If time-scaling ends, then the barrier is removed.
 */
void LooseParallelThreadManager::notifySchedulerForVirtualizedTime(VexThreadState *state, const float &scalingFactor) {
	if (scalingFactor == 1) {
		state->clearTimeScalingFactor();

		LOG(logger, logDEBUG2) << " Manager " << managerId << ": clearing / before erasing time scaling factor of " << state->getName() << " (" << state->getCurrentStateName() << ")" << endl;
		runnableThreads->erase(state);

		state->clearSynchronizationBarrierTime(schedulerTimeslot);
		LOG(logger, logDEBUG2) << "notifySchedulerForVirtualizedTime: " << state->getName() << " CLEARED its blocking barrier to " << state->getEstimatedRealTime()/1e6 << endl;

	} else {
		state->setTimeScalingFactor(scalingFactor);
		LOG(logger, logDEBUG2) << " Manager " << managerId << ": setting time scaling factor " << scalingFactor << " to " << state->getName() << " (" << state->getCurrentStateName() << ") at " << state->getEstimatedRealTime()/1e6 << endl;
		setRunningThread(state);
	}
	state->getThreadCurrentlyControllingManager()->_notifySchedulerForVirtualizedTime(state);
}

/*
 * This method is overridden just for the sake of the VISUALIZE_EVENT macro,
 * which is different (logging GVT instead of thread state->ERT)
 */
void LooseParallelThreadManager::setTimedWaitingThread(VexThreadState *state) {
	if (state !=  NULL) {
		unsetCurrentThreadFromRunningThread(state);

		state->setWaiting();

		VISUALIZE_EVENT(WAITING_TIMEOUT, state);	// SPECIAL as in use global virtual time as timestamp

		if (state->getTimeout() > 0) {
			runnableThreads->push(state);
		}

	}
}


/*
 * We avoid speeding up the waiting time of loosely synchronized schedulers after model simulation.
 * In this way we ensure that they remain in sync.
 */
void LooseParallelThreadManager::afterModelSimulation() {

}



/*
 * This is the main time-updating method.
 * It updates the thread and global time called from external updating time methods:
 * remove the schedulerTimeslot added as a barrier to loosely synchronize cores when time-scaled threads are running
 */
void LooseParallelThreadManager::locklessUpdateTimeBy(const long long &timeDiff, VexThreadState *state) {

	if (state->getTimeScalingFactor() != 1) {
		state->clearSynchronizationBarrierTime(schedulerTimeslot);
		LOG(logger, logDEBUG2) << "locklessUpdateTimeBy: " << state->getName() << " CLEARED its blocking barrier. Its ERT is set back to " << state->getEstimatedRealTime()/1e6 << endl;
	}

	state->addLocalTime(timeDiff);	// if you ever replace this with add-elapsed time remember that scaling factor adjustments are in thread class only now
	if (state->isNativeWaiting()) {
		virtualTimelineController->commitNativeWaitingProgress(state);
	} else {
		virtualTimelineController->commitCpuTimeProgress(state);
	}

	// The barrier should be set back, when a thread resumes: either by saying so to the asking core handler or after being suspended

//	if (state->getTimeScalingFactor() != 1) {
//		state->setSynchronizationBarrierTime(schedulerTimeslot);
//		runnableThreads->update();
//		LOG(logger, logDEBUG2) << "locklessUpdateTimeBy: " << state->getName() << " UPDATED its blocking barrier to the estimated TSF completion ERT of " << state->getEstimatedRealTime()/1e6 << endl;
//	}
}


void LooseParallelThreadManager::setSuspended(VexThreadState *state) {

	lastSuspended = runningThread;
	if (runningThread == state) {
		runningThread = NULL;
	}

	if (state->getTimeScalingFactor() != 1) {
		runnableThreads->update();
		state->setSuspended();
	} else {
		state->setSuspended();
		runnableThreads->push(state);	// thread might already be in the suspended state - it would not be normally pushed
	}

}


void LooseParallelThreadManager::onIntentionToKeepOnRunning(VexThreadState *state) {

	// The thread state should not change (RUNNING->RUNNING or IN_SYSTEM_CALL->IN_SYSTEM_CALL)
	lockRunningThread();
	runningThread = state;
	state->setThreadCurrentlyControllingManager(this);
	unlockRunningThread();

	if (state->getTimeScalingFactor() != 1) {
		state->setSynchronizationBarrierTime(schedulerTimeslot);
		runnableThreads->update();
		LOG(logger, logDEBUG2) << "onIntentionToKeepOnRunning: " << state->getName() << " UPDATED its blocking barrier to the estimated TSF completion ERT of " << state->getEstimatedRealTime()/1e6 << endl;
	}
	LOG(logger, logINFO) << "Manager " << managerId << " has now thread \""<< state->getName() << "\" ("<< state->getEstimatedRealTime()/1e6 << ") as running" << endl;// at " << getAllCallingMethodsUntil(5) << endl;

//		state->increaseConsecutiveTimeslots();
//		decreaseSchedulerSleepingTimeBy = state->getEstimatedRealTime() - state->getResumedLastAt();	- crashes, because thread might have already continued and exited

	//decreaseSchedulerSleepingTimeBy += schedulerTimeslot/2;	// NEW in 415: limit timeslot, if thread decides to keep on running
}

void LooseParallelThreadManager::onIntentionToBeDisregarded(VexThreadState *state) {
	// If the time-scaled thread has to be disregarded (became native waiting), then you have to remove it from the queue of runnable threads
	if (state->getTimeScalingFactor() != 1) {
		runnableThreads->erase(state);
		LOG(logger, logINFO) << " thread " << state->getName() << "("<< state->getCurrentStateName() <<") set NATIVE WAITING by " << this->managerId << " AND ERASING TIME SCALED BLOCKER as controller" << endl;
	} else {
		LOG(logger, logINFO) << " thread " << state->getName() << " set NATIVE WAITING by " << this->managerId << " as controller at " << state->getEstimatedRealTime()/1e6 << endl;
	}
}
