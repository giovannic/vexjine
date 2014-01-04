#include "ThreadQueue.h"
#include <cassert>

#ifdef USING_LIBUNWIND
#include <libunwind.h>
#endif

ThreadQueue::ThreadQueue() {
	threadsQueue = new AdaptiblePQueue<VexThreadState *, deque<VexThreadState *>, threadStatePtr_compare>;
	pthread_mutex_init(&mutex, NULL);
	highestRunnableTime = 0;
}

void ThreadQueue::lockMutex() {
	pthread_mutex_lock(&mutex);
}

void ThreadQueue::unlockMutex() {
	pthread_mutex_unlock(&mutex);
}

/*
 * Insert element into the queue - it will find its own place in the priority queue
 */
void ThreadQueue::push(VexThreadState *state) {

	lockMutex();

	LOG(queueLogger, logWARNING) << "QQQQQ: pushing " << state->getName() << " (" << state->getCurrentStateName() << ") " << getAllCallingMethodsUntil(12) << endl;
	assert ( state->getTimeScalingFactor() != 1 || threadsQueue -> find(state) == false );
	threadsQueue -> push(state);
	assert ( state->getTimeScalingFactor() != 1 || threadsQueue -> find(state) == true );
	unlockMutex();
}


/*
 * Return head of queue without removing it
 */
VexThreadState *ThreadQueue::top() {
	lockMutex();
	if (threadsQueue -> empty()) {
		unlockMutex();
		return NULL;
	} else {
		VexThreadState *state = threadsQueue -> top();
		unlockMutex();
		return state;
	}
}

/*
 * Remove and return the head of the queue
 */
VexThreadState *ThreadQueue::getNext() {
	lockMutex();
	if (threadsQueue -> empty()) {
		unlockMutex();
		return NULL;
	} else {
		VexThreadState *state = threadsQueue -> top();
		LOG(queueLogger, logWARNING) << "QQQQQ: popping () " << state->getName() << " (" << state->getCurrentStateName() << ")" << endl;//" " << getAllCallingMethodsUntil(2) << endl;
		threadsQueue -> pop();
		unlockMutex();
		return state;
	}
}


/*
 * Return the top thread from the queue of runnables if it is equal to lockedState
 */
VexThreadState *ThreadQueue::getNextIfEqualsElseReturnTop(VexThreadState *lockedState, ThreadManager *requestingManager) {
	lockMutex();
	if (threadsQueue -> empty()) {
		unlockMutex();
		return NULL;
	} else {
		VexThreadState *state = threadsQueue -> top();
		if (state == lockedState) {
			// It is not wise to allow IN_IO threads to be handled by other processors as we might pop them right as they start
			// RUNNING threads in the queue mean that they are accelerated and every other manager should wait for them to finish in virtual time
			if (state->isRunning()) {
				if(state->getThreadCurrentlyControllingManager() != requestingManager) {
					LOG(queueLogger, logWARNING) << "QQQQQ (getNextIfEquals): found blocking " << state->getName() <<  endl;
				} else {
					LOG(queueLogger, logWARNING) << "QQQQQ (getNextIfEquals): found MY OWN blocker " << state->getName() <<  endl;
				}
				unlockMutex();
				return NULL;

			} else if (state->inIoPredictionPhase()) {
				LOG(queueLogger, logWARNING) << "QQQQQ (getNextIfEquals): " << state->getName() <<  " is in IO or RUNNING-block but he is my thread " << requestingManager << " so I can pop him" << endl;
				unlockMutex();
				return state;

			} else if (state->isWaitingRealCodeToCompleteAfterModelSimulation()) {
				if(state->getThreadCurrentlyControllingManager() != requestingManager) {
					unlockMutex();
					return NULL;
				} else {
					// WE DO NOT POP THE STATE HERE
					unlockMutex();
					return state;
				}
			}

			LOG(queueLogger, logWARNING) << "QQQQQ: popping (getNextIfEquals) " << state->getName() << " (" << state->getCurrentStateName() << ") " << getAllCallingMethodsUntil(12) << endl;
			threadsQueue -> pop();
		}
		unlockMutex();
		return state;

	}
}

/*
 * Remove and return the head of the queue from manager requestingManager
 */
VexThreadState *ThreadQueue::getNext(ThreadManager *requestingManager) {
	lockMutex();
	if (threadsQueue -> empty()) {
		unlockMutex();
		return NULL;
	} else {
		VexThreadState *state = threadsQueue -> top();
		// It is not wise to allow IN_IO threads to be handled by other processors as we might pop them right as they start
		// RUNNING threads in the queue mean that they are accelerated and every other manager should wait for them to finish in virtual time
		if (state->isRunning()) {
			if(state->getThreadCurrentlyControllingManager() != requestingManager) {
				LOG(queueLogger, logWARNING) << "QQQQQ (getNextIfEquals): found blocking " << state->getName() <<  endl;
			} else {
				LOG(queueLogger, logWARNING) << "QQQQQ (getNextIfEquals): found MY OWN blocker " << state->getName() <<  endl;
			}
			unlockMutex();
			return NULL;

		} else if (state->inIoPredictionPhase()) {
			LOG(queueLogger, logWARNING) << "QQQQQ (getNextIfEquals): " << state->getName() <<  " is in IO or RUNNING-block but he is my thread " << requestingManager << " so I can pop him" << endl;
			unlockMutex();
			return state;

		} else if (state->isWaitingRealCodeToCompleteAfterModelSimulation()) {
			if(state->getThreadCurrentlyControllingManager() != requestingManager) {
				unlockMutex();
				return NULL;
			} else {
				// WE DO NOT POP THE STATE HERE
				unlockMutex();
				return state;
			}
		}

		LOG(queueLogger, logWARNING) << "QQQQQ: popping (manager) " << state->getName() << " (" << state->getCurrentStateName() << ") " << getAllCallingMethodsUntil(12) << endl;
		threadsQueue -> pop();
		unlockMutex();
		return state;
	}
}


/*
 * Invalidate all expired IO predictions
 */
void ThreadQueue::invalidateExpiredIoPredictions(const long long &currentRealTime) {
	lockMutex();
	if (!threadsQueue -> empty()) {
		VexThreadState *state = threadsQueue->top();
		if (state != NULL) {
			LOG(queueLogger, logWARNING) << "QQQQQ: popping () " << state->getName() << " (" << state->getCurrentStateName() << ") in invalidateExpiredIoPredictions" << endl;
			threadsQueue->pop();

			recursivelyUpdatePendingIoRequests(state, currentRealTime);

			LOG(queueLogger, logWARNING) << "QQQQQ: pushing () " << state->getName() << " (" << state->getCurrentStateName() << ") in invalidateExpiredIoPredictions" << endl;
			threadsQueue->push(state);
		}
	}
	unlockMutex();
}

/*
 * Recursively invalidate all expired IO predictions
 */
void ThreadQueue::recursivelyUpdatePendingIoRequests(VexThreadState *state, const long long &currentRealTime) {
	if (!threadsQueue -> empty()) {
		VexThreadState *nextThread = threadsQueue -> top();
		if (nextThread->inIoPredictionPhase() && !nextThread->isIoPredictionStillValid(currentRealTime)) {
			LOG(queueLogger, logWARNING) << "QQQQQ: popping () " << state->getName() << " (" << state->getCurrentStateName() << ") (" << state->getCurrentStateName() << ") in recursivelyUpdatePendingIoRequests" << endl;
			threadsQueue -> pop();

			recursivelyUpdatePendingIoRequests(nextThread, currentRealTime);

			LOG(queueLogger, logWARNING) << "QQQQQ: pushing () " << state->getName() << " (" << state->getCurrentStateName() << ") (" << state->getCurrentStateName() << ") in recursivelyUpdatePendingIoRequests" << endl;
			threadsQueue->push(nextThread);
		}
	}
	state->invalidateIoPrediction();
}


/*
 * Search for an element in the priority queue
 */
bool ThreadQueue::find(VexThreadState *state) {
	lockMutex();
	bool result = threadsQueue -> find(state);
	unlockMutex();
	return result;
}

/*
 * Get the most advanced time stamp from all threads in the queue
 */
long long ThreadQueue::getHighestRunnableTime(VexThreadState *askingThread) {
	lockMutex();
	std::deque<VexThreadState *>::iterator it = threadsQueue -> getQueueStart();
	std::deque<VexThreadState *>::iterator itEnd = threadsQueue -> getQueueEnd();

	long long highestRunnableTime = askingThread -> getEstimatedRealTime();
	VexThreadState *state;
	while (it != itEnd) {
		state = (*it);
		if (state->getEstimatedRealTime() > highestRunnableTime && (state->isSuspended() || state->isNativeWaiting())) {
			highestRunnableTime = state->getEstimatedRealTime();
		}
		++it;
	}
	unlockMutex();
	return highestRunnableTime;

}

/*
 * Erasing a thread anywhere in the list
 */
void ThreadQueue::erase(VexThreadState *state) {
	lockMutex();
	LOG(queueLogger, logWARNING) << "QQQQQ: erasing " << state->getName() << " (" << state->getCurrentStateName() << ") (" << state->getCurrentStateName() << ") " << getAllCallingMethodsUntil(12) << endl;
	threadsQueue -> erase(state);
	unlockMutex();
}


void ThreadQueue::print() {
	lockMutex();
	threadsQueue -> print();
	unlockMutex();
}

/*
 * Update the heap structure of the priority queue - useful after manually changing the time of a thread
 * that resides inside the queue
 */
void ThreadQueue::update() {
	lockMutex();
	LOG(queueLogger, logWARNING) << "QQQQQ: updating queue " << endl;
	threadsQueue -> update();
	unlockMutex();
}

/*
 * Forward the virtual timestamp of thread with structure state to the timestamp
 * of the thread with the next highest timestamp that is not in modelling code.
 * This is used to force the next thread to make progress if the current thread
 * is blocked executing the body of a model described method:
 * in that case allowing another thread to proceed resolves the deadlock
 */
void ThreadQueue::setThreadToNextBiggestErt(VexThreadState *state) {
	lockMutex();
	std::deque<VexThreadState *>::iterator it = threadsQueue -> getQueueStart();
	std::deque<VexThreadState *>::iterator itEnd = threadsQueue -> getQueueEnd();

	while (it != itEnd) {
		if (state == *it) {
			++it;

			while (it != itEnd && ((*it)->isSimulatingModel() || (*it)->isWaitingRealCodeToCompleteAfterModelSimulation())) {
				++it;
			}

			if (it == itEnd) {
				--it;
			}

			long long nextErtTimePlusOne = (*it)->getEstimatedRealTime()+1;
			state->leapTo(nextErtTimePlusOne);
			threadsQueue -> update();
			break;
		}
		++it;
	}
	unlockMutex();

}


/*
 * Check if the head of the queue has a virtual timestamp less than the timestamp of the argument
 */
bool ThreadQueue::isNextRunnableThreadBefore(const long long &timestamp) {
	lockMutex();
	if (threadsQueue -> empty()) {
		unlockMutex();
		return false;
	} else {
		VexThreadState *nextRunnable = threadsQueue -> top();
		bool returnValue = (nextRunnable != NULL && (timestamp > nextRunnable->getEstimatedRealTime()));
		unlockMutex();
		return returnValue;
	}
}


/*
 * Get the size of the queue
 */
unsigned int ThreadQueue::size() {
	lockMutex();
	unsigned int size = threadsQueue -> size();
	unlockMutex();
	return size;
}

bool ThreadQueue::empty() {
	lockMutex();
	bool isEmpty = threadsQueue -> empty();
	unlockMutex();
	return isEmpty;
}

ThreadQueue::~ThreadQueue()
{
	delete threadsQueue;
	pthread_mutex_destroy(&mutex);
}


/*
 * Used for debugging reasons to print the stack trace of the queue's updating points
 */
std::string ThreadQueue::getAllCallingMethodsUntil(short maxStack) {
#ifdef USING_LIBUNWIND
	std::stringstream stemp;

	unw_cursor_t    cursor;
	unw_context_t   context;

	unw_getcontext(&context);
	unw_init_local(&cursor, &context);
	short current_stack_depth = 0;
	while (unw_step(&cursor) > 0 && current_stack_depth++ < maxStack) {
		unw_word_t  offset;
		unw_word_t  pc, eax, ebx, ecx, edx;
		char        fname[64];

		unw_get_reg(&cursor, UNW_REG_IP,  &pc);
		unw_get_reg(&cursor, LIBUNWIND_CONSTANT_RAX, &eax);
		unw_get_reg(&cursor, LIBUNWIND_CONSTANT_RDX, &edx);
		unw_get_reg(&cursor, LIBUNWIND_CONSTANT_RCX, &ecx);
		unw_get_reg(&cursor, LIBUNWIND_CONSTANT_RBX, &ebx);

		fname[0] = '\0';
		unw_get_proc_name(&cursor, fname, sizeof(fname), &offset);
		if (current_stack_depth > 1) {
			stemp << fname;
		}

	}
	return stemp.str();
#else
	return std::string("Cannot return stack-trace without libunwind");
#endif
}
