/*
 * Scheduling.cpp
 *
 *  Created on: 21 Apr 2011
 *      Author: root
 */

#include "Scheduling.h"
#include "ThreadManager.h"
#include "Time.h"

#include <time.h>

ThreadManager *Scheduling::defaultThreadManager;

void Scheduling::haltSuspendForAwhile() {
	doNotSuspendUntil = Time::getRealTime() + 3000000;
}

Scheduling::Scheduling() {
	pthread_mutex_init(&suspendLock, NULL);
	pthread_cond_init(&condSuspendLock, NULL);

	pthread_mutex_init(&sharedAccessKey, NULL);

	pthread_mutex_init(&suspendFlagLock, NULL);
	pthread_cond_init(&condSuspendFlagLock, NULL);
	suspendFlag = CLEAR_SUSPENDFLAG;

	awaken = false;
	vtfInvocationsSinceLastResume = 0;
	forceSuspend = true;			// always suspend the incoming thread

	localManagerId = 0;				// all threads will notify the 0-manager when they log in
	waitingInNativeVTFcode = 0;

	consecutiveTimeslots = 0;

	simulatingModel = false;
	threadCurrentlyControllingManager = NULL;
	previouslyControllingManager = NULL;
	inVex = false;

	currentlyPolledForWhetherItIsNativeWaiting = false;
	awakeningFromJoin = false;

	doNotSuspendUntil = 0;
}

Scheduling::~Scheduling() {
	pthread_mutex_destroy(&suspendLock);
	pthread_mutex_destroy(&suspendFlagLock);
	pthread_mutex_destroy(&sharedAccessKey);
}

/*
 * Setting the status of the suspendflag of a thread to whether it can be suspended or not
 */
void Scheduling::notifySchedulerForIntention(const short& intention) {
	pthread_mutex_lock(&suspendFlagLock);
	suspendFlag = intention;
	pthread_cond_signal(&condSuspendFlagLock);
	pthread_mutex_unlock(&suspendFlagLock);

}



/*
 * Checking the status of the suspendflag of a thread to see whether it could be suspended
 */
short Scheduling::getSuspendingThreadIntention() {
//	timespec ts;
	pthread_mutex_lock(&suspendFlagLock);
	while (suspendFlag == CLEAR_SUSPENDFLAG) {	// Unknown Intentions
		//pthread_cond_wait(&condSuspendFlagLock, &suspendFlagLock);
//		clock_gettime(CLOCK_REALTIME, &ts);
//		++ts.tv_sec;
//		pthread_cond_timedwait(&condSuspendFlagLock, &suspendFlagLock, &ts);
		pthread_cond_wait(&condSuspendFlagLock, &suspendFlagLock);
	}

	short returnValue = suspendFlag;
	suspendFlag = CLEAR_SUSPENDFLAG;
	pthread_mutex_unlock(&suspendFlagLock);

	return returnValue;
}



void Scheduling::notifySchedulerThatThreadResumed() {
	pthread_mutex_lock(&suspendFlagLock);
	suspendFlag = RESUMING_SUSPENDFLAG;
	pthread_cond_signal(&condSuspendFlagLock);
	pthread_mutex_unlock(&suspendFlagLock);
}

void Scheduling::acquireThreadControllingLock() {
	pthread_mutex_lock(&suspendLock);
}
void Scheduling::releaseThreadControllingLock() {
	pthread_mutex_unlock(&suspendLock);
}

/*
 * Checking the status of the suspendflag of a thread to see whether it could be suspended
 */
void Scheduling::waitForResumingThread() {
	pthread_mutex_lock(&suspendFlagLock);
	while (suspendFlag == CLEAR_SUSPENDFLAG) {	// Unknown Intentions
		pthread_cond_wait(&condSuspendFlagLock, &suspendFlagLock);
	}
	suspendFlag = CLEAR_SUSPENDFLAG;
	pthread_mutex_unlock(&suspendFlagLock);
}

/*
 * The most important method: keeping the thread blocked to be notified by a scheduler
 */
void Scheduling::blockHereUntilSignaled() {
	awaken = false;
	while (!awaken) {
		pthread_cond_wait(&condSuspendLock, &suspendLock);
	}
}

/*
 * When a model is being simulated, release the controlling key, since the scheduler
 * cannot stop you any more. This applies only if the method body of the model-described
 * method should be executed.
 */
void Scheduling::startSchedulerControlledModelSimulation() {
	simulatingModel = true;
	releaseThreadControllingLock();
}
/*
 * Block until the model simulation is over
 */
void Scheduling::blockUntilModelSimulationEnd() {
	acquireThreadControllingLock();
	while (simulatingModel) {
		pthread_cond_wait(&condSuspendLock, &suspendLock);
	}
}
/*
 * When the model simulation finishes synchronize with waiting or real-code running thread.
 */
void Scheduling::notifyModelSimulationEnd() {
	acquireThreadControllingLock();
	simulatingModel = false;
	pthread_cond_signal(&condSuspendLock);
	releaseThreadControllingLock();
}

void Scheduling::locklessNotifyModelSimulationEnd() {
	simulatingModel = false;
}



bool Scheduling::isSuspendingAllowed() {
	return doNotSuspendUntil < Time::getRealTime();
}
