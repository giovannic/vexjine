/*
 * VexThreadCustomer.cpp
 *
 *  Created on: 26 May 2011
 *      Author: root
 */

#include "VexThreadCustomer.h"
#include "ThreadManager.h"
#include <cassert>

#include <sys/time.h>
double VexThreadCustomer::iterationsPerNs;


void VexThreadCustomer::lock() {
	state->lockShareResourceAccessKey();
}

void VexThreadCustomer::unlock() {
	state->unlockShareResourceAccessKey();
}

const char *VexThreadCustomer::getThreadName() {
	return state->getName();
}

/***
 * Simulate the execution of the method with busy waiting
 * Update the GVT when finished so that queueing threads
 * can update their resuming times correctly
 *
 * Preconditions: Don't hold scheduler lock when entering
 * Postconditions: Lock IS held after exit
 *
 * @return: the action of servicing has actually finished
 */
bool VexThreadCustomer::service(const long &time) {

	////cout << state->getName() << " service time " << time/1000000 << " (APPROX from " << state->getEstimatedRealTime()/1000000 << " to " << (time + state->getEstimatedRealTime())/1000000 << " )" << endl;

	state->updateCpuTimeClock();
	simulateExecutionOnCpuFor(time);

	lock();
	(*manager)->updateCurrentThreadVT(state);
	// key released in onCompletingService()

	////cout << state->getName() << " finished service EXACTLY at " << state->getEstimatedRealTime()/1000000 << endl;

	return true;
}

/***
 * Block at the queue described by lock mutex
 * Wrap an actual blocking conditional variable
 * TODO: improve a bit the design taking the mutex out?
 *
 * Preconditions: Should NOT hold scheduler lock when entering
 * Postconditions: Lock is NOT held after exit
 *
 * @return: the action of queueing has actually finished
 */
bool VexThreadCustomer::blockAtQueue(pthread_mutex_t *mutex) {

	assert(false);
	////cout << state->getName()  << " BLOCKING AT QUEUE at " << state->getEstimatedRealTime()/1000000 << endl;/
	//TODO: deprecated calls of a deprecated approach
//	(*manager)->onThreadWaitingStartLockless(state);

	//TODO: add guardian loop?
	pthread_cond_wait(&customerQueueingCond, mutex);
	pthread_mutex_unlock(mutex);	//don't hold this while waiting for manager lock in waitingEnd

	// After getting resumed by currently serviced thread
	//TODO: deprecated calls of a deprecated approach
	//(*manager)->onThreadWaitingEnd(state);

	unlock();	// unlock the customer mutex
	////cout << state->getName() << " RESUMED after queueing at " << (state->getEstimatedRealTime()/1000000) << endl;

	return true;
}

/***
 * The thread that has just completed service, sends
 * a signal to the queueing thread "state"
 */
void VexThreadCustomer::resume(const long long &resumeTime) {
//	////cout << "just serviced thread will resume thread " << state->getName() << "("<< state->getId() << ") at " << resumeTime << endl;
	pthread_cond_broadcast(&customerQueueingCond);
}


/***
 * Wait for a defined time in a (remote) server
 * Initiate a timed waiting simulation
 *
 * Preconditions: Should hold scheduler lock when entering
 * Postconditions: Lock is held after exit
 * @return: the action of waiting has actually finished
 */
bool VexThreadCustomer::waitRemote(const long &duration) {
	state->setDontUpdateToGlobalTime(true);

	////cout << state->getName() << " started waiting at " << state->getEstimatedRealTime()/1000000 << " for " << duration/1000000 << endl;
	long duration1 = duration;
	(*manager)->onReplacedTimedWaiting(state, duration1);
	//(*manager)->onThreadTimedWaitingStart(state, duration1);
	////cout << state->getName() << " ended waiting at " << state->getEstimatedRealTime()/1000000 << endl;

	state->setDontUpdateToGlobalTime(false);
	return true;
}

/***
 * VEX ThreadState wrappers
 */
long long VexThreadCustomer::getEstimatedRealTime() {
	return state->getEstimatedRealTime();
}

void VexThreadCustomer::setEstimatedRealTime(const long long &estimatedRealTime) {
	state->setEstimatedRealTime(estimatedRealTime);
}

void VexThreadCustomer::setFinished() {
	finished = true;
}

void VexThreadCustomer::clearFinishedFlag() {
	finished = false;
}

VexThreadCustomer::~VexThreadCustomer() {

}

/*
 * Busy waiting simulation support method:
 * calculate the iterations that will be executed
 * during the simulation of each nanosecond
 */
void VexThreadCustomer::calculateIterationsPerNs() {
	return;
	struct timeval totalstart;
	gettimeofday(&totalstart, NULL);

	struct timeval start, end, diff;
	double totalTime;
	double previousIterationsPerNs = 0;
	iterationsPerNs = 1.0;
	long long iterations = 10000000;
	VexThreadCustomer *c = new VexThreadCustomer();
	while (true) {
		gettimeofday(&start, NULL);
		c->simulateExecutionOnCpuFor(iterations);
		gettimeofday(&end, NULL);
		timersub(&end, &start, &diff);
		totalTime = diff.tv_sec * 1000000000 + diff.tv_usec * 1000;

		if (previousIterationsPerNs == 0) {
			previousIterationsPerNs = totalTime / iterations;
			////cout << totalTime << "/" << iterations << "=" << previousIterationsPerNs << endl;
		} else {
			iterationsPerNs = totalTime / (iterationsPerNs * iterations);

			////cout << totalTime << "/" << iterations << "=" << iterationsPerNs << endl;
			if (fabs(previousIterationsPerNs - iterationsPerNs)/previousIterationsPerNs > 0.05) {
				previousIterationsPerNs	= (previousIterationsPerNs + iterationsPerNs)/2;
				iterations *= 2;
			} else {
				iterationsPerNs	= (previousIterationsPerNs + iterationsPerNs)/2;
				break;
			}
		}
	}

	int count = 0;
	double factor = 0.0;
	for (int i = 1000; i<15000000; i *= 2) {
		iterations = i;
		gettimeofday(&start, NULL);
		c->simulateExecutionOnCpuFor(iterations);
		gettimeofday(&end, NULL);
		timersub(&end, &start, &diff);
		factor += (double)i/(double)(diff.tv_usec * 1000);
		++count;
	}
	factor /= count;
	iterationsPerNs *= factor;
cout << "final " <<iterationsPerNs << endl;
	//testServiceSimulationError(c);
	delete c;

//	gettimeofday(&totalend, NULL);
//	timersub(&totalend, &totalstart, &totaldiff);
//	totalTime = totaldiff.tv_sec * 1000000000 + totaldiff.tv_usec * 1000;
//	////cout << "Total simulateExecutionOnCpuFor testing time: " << totalTime / 1000000000.0 << endl;
}

void VexThreadCustomer::testServiceSimulationError(VexThreadCustomer *c) {
	struct timeval start, end, diff;
	int count = 0;
	double factor = 0.0;
	int nsecs;
	for (int i = 0; i<4000; i++) {
		nsecs = rand() % 100000;

		gettimeofday(&start, NULL);
		c->simulateExecutionOnCpuFor(nsecs);
		gettimeofday(&end, NULL);
		timersub(&end, &start, &diff);

		if (diff.tv_usec == 0) {
			diff.tv_usec = 1;
		}
		factor += (double)nsecs/(double)(diff.tv_usec * 1000);
		++count;
	}

	////cout << "average error " << factor/count << endl;
	assert(factor/count < 1.25 && factor/count > 0.75);

}


/*
 * Busy waiting for duration of service time
 */
void VexThreadCustomer::simulateExecutionOnCpuFor(const long &time) {
	long long iterations = time * iterationsPerNs;
	for (long long j=0; j<iterations; j++) {
		servicingResult += j;
	}
}
