/*
 * VexSchedSimCustomer.cpp
 *
 *  Created on: 28 Jun 2011
 *      Author: root
 */

#include "VexSchedSimCustomer.h"
#include <cassert>

/***
 * No locking is required - everything done by schedulers
 */

const char *VexSchedSimCustomer::getThreadName() {
	return state->getName();
}
/***
 * Local resource consumption will be divided into
 * the correct number of scheduler timeslots by the model handler
 * class of thread-state. In each iteration time is decreased
 * by a scheduler timeslot. When the remaining time is zero,
 * the thread simulation is allowed to resume.
 *
 * @return: the action of servicing has NOT actually finished
 */
bool VexSchedSimCustomer::service(const long &time) {
	////cout << state->getName()  << " service time " << time/1000000 << endl;
	state->initiateLocalResourceConsumption(time);

	return false;
}

/***
 * Start queueing
 *
 * @return: the action of queueing has NOT finished after this method has exited
 */
bool VexSchedSimCustomer::blockAtQueue(pthread_mutex_t *q) {
	state->setInModelWaiting();
//	////cout << state->getName()  << " BLOCKING AT QUEUE at " << state->getEstimatedRealTime()/1000000 << endl;
	pthread_mutex_unlock(q);
	return false;
}


/***
 * Resume a currently model-waiting blocked thread
 * Push the thread into the runnables queue with the timestamp
 * of its resuming time.
 * We need to synchronize between the scheduler that set the
 * thread into model waiting and the one reputting it back into the
 * runnables queue
 *
 * Executed by the scheduler itself
 */
void VexSchedSimCustomer::resume(const long long &resumeTime) {
	state->setInModelSimulation();
//	state->setEstimatedRealTime(resumeTime);
	state->leapForwardTo(resumeTime);

	assert((*manager) != NULL);
	(*manager)->pushIntoRunnableQueue(state);
}


/***
 * Wait for a defined time in a (remote) server
 *
 * @return: the action of waiting has NOT actually finished
 */
bool VexSchedSimCustomer::waitRemote(const long &duration) {
	long long duration1 = duration;
//cout << state->getName() << " waiting in model from " << state->getEstimatedRealTime()/1e6 << " to " << (state->getEstimatedRealTime()+duration)/1e6 << endl;
	state->startModelTimedWaiting(duration1);
//	state->leapForwardBy(duration1);
	return false;
}


/***
 * VEX ThreadState wrappers
 */
long long VexSchedSimCustomer::getEstimatedRealTime() {
	return state->getEstimatedRealTime();
}

void VexSchedSimCustomer::setEstimatedRealTime(const long long &estimatedRealTime) {
	state->setEstimatedRealTime(estimatedRealTime);
}

void VexSchedSimCustomer::setLocation( CinqsNode *n ) {
	state->setCurrentQueueingNode(n);
	location = n ;
}

void VexSchedSimCustomer::setFinished() {
	finished = true;
}

void VexSchedSimCustomer::clearFinishedFlag() {
	finished = false;
}
