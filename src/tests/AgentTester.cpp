/*
 * AgentTester.cpp
 *
 *  Created on: 12 Oct 2010
 *      Author: root
 */

#include "AgentTester.h"
#include "ThreadManager.h"

#include <cassert>

bool AgentTester::onlySelectedProfiling= false;
AgentTester::AgentTester() {
	

}

AgentTester::~AgentTester() {
	 
}

string AgentTester::testName() {
	return "Agent testing";
}

bool AgentTester::test() {
	return true;//registryAddAndRetrieve();
}

void AgentTester::profileOnlySelected() {
	onlySelectedProfiling = true;
}

//llllllllllllllllll
pthread_spinlock_t lock;
int pint;

void AgentTester::methodEntry(int methodId) {
	if (Tester::virtualExecution) {
		if (!onlySelectedProfiling || methodId == 100) {
		VEX::methodEventsBehaviour->afterMethodEntry(methodId);
		}
	}
}


/****
 * beforeMethodExit: a thread (with threadId) exits the profiled method (methodId)
 *
 * Add the difference between current (virtual) time and method entry point to the results for this methodId
 */
void AgentTester::methodExit(int methodId) {
	if (Tester::virtualExecution) {
		if (!onlySelectedProfiling || methodId == 100) {
			VEX::methodEventsBehaviour->beforeMethodExit(methodId);
		}
	}
}

void AgentTester::afterMethodIoEntry(int methodId, int invocationPointHashValue) {
	if (Tester::virtualExecution) {
		VEX::methodEventsBehaviour->afterIoMethodEntry(methodId, invocationPointHashValue, false);
//		ThreadState *state = state->getCurrentThreadState();
//		state->inIO = true;
//		state->setIoInvocationPointHashValue( invocationPointHashValue);
//		manager-> onThreadIoStart(state);
	}
}

void AgentTester::beforeMethodIoExit(int methodId) {
	if (Tester::virtualExecution) {
		VEX::methodEventsBehaviour->beforeIoMethodExit(methodId);
//		long long realTimeValueOnExit  = Time::getRealTimeBeforeMethodInstrumentation();
//		// Get state of current thread
//		ThreadState *state = ThreadState::getCurrentThreadState();
//		manager-> onThreadIoEnd(realTimeValueOnExit, state, methodId);
//
//		state-> updateCpuTimeClock();	// next update will be based back again on CPU time
	}
}


void AgentTester::onThreadWaitingStart(VexThreadState * state, long timeout) {
	if (Tester::virtualExecution) {
		assert(false);
//		if (timeout > 0) {
//			threadEventsBehaviour->onReplacedTimedWaitingStart();
//		} else {
//			threadEventsBehaviour->onWrappedWaitingStart();
//		}

	}
}

void AgentTester::onThreadContendedEnter(VexThreadState * state, pthread_mutex_t *mutex) {
	if (Tester::virtualExecution) {
		assert(false);
//		state-> setTimeout(-1);
//
//		threadEventsBehaviour->onWrappedWaitingStart();
//		manager->onThreadContendedEnter(state, Time::getVirtualTime());
	}
}

void AgentTester::onThreadContendedEntered(VexThreadState * state) {
	if (Tester::virtualExecution) {
		assert(false);
//		manager->onThreadContendedEntered(state);
	}
}


void AgentTester::onThreadWaitingEnd(VexThreadState * state) {
	if (Tester::virtualExecution) {
		assert(false);
//		manager->onThreadWaitingEnd(state);
	}
}

void AgentTester::onThreadWaitingStart(long timeout) {
	if (Tester::virtualExecution) {
		assert(false);
//		VexThreadState *state = VexThreadState::getCurrentThreadState();
//		AgentTester::onThreadWaitingStart(state, timeout);
	}
}

void AgentTester::onThreadWaitingEnd() {
	if (Tester::virtualExecution) {
		assert(false);
//		VexThreadState *state = VexThreadState::getCurrentThreadState();
//		AgentTester::onThreadWaitingEnd(state);
	}
}

void AgentTester::onThreadSpawn(VexThreadState * state) {
	if (Tester::virtualExecution) {
		manager->onThreadSpawn(state);
	}
}

void AgentTester::onThreadEnd(VexThreadState * state) {
	if (Tester::virtualExecution) {
		threadEventsBehaviour->onEnd();
	}
}


bool AgentTester::interruptOnVirtualTimeout(VexThreadState * state, pthread_mutex_t *mutex, pthread_cond_t *cond, long timeout) {
	if (Tester::virtualExecution) {
		assert(false);
//		manager->lockMutex();
//
//		// Distinguish between Object.wait(t) and Thread.sleep(t)
//		if (mutex != 0) {
//			state->setWaitingObjectId((long)mutex);
////			waitingOnObjectRegistry-> insert(state->getTimedWaitingObjectId(), state->getUniqueId());
//			pthread_mutex_unlock(mutex);
//		} else {
//			state->setWaitingObjectId(0);
//		}
//
//		manager->onThreadTimedWaitingStart(state, timeout);
//		manager->unlockMutex();
//
//		// Resume here after timeout or after being interrupted
//		if (mutex != 0) {
//			manager->onThreadContendedEnter(state, Time::getVirtualTime());
//			pthread_mutex_lock(mutex);
//			manager->onThreadContendedEntered(state);
//
//		}
//
//		state->updateCpuTimeClock();
//		return state->getTimedOut();

	}
	return false;
}


void AgentTester::onThreadInteractionPointEncounter(VexThreadState * state) {
	if (Tester::virtualExecution) {
		assert(false);
//		long long startingTime = Time::getVirtualTime();
//		manager->onThreadInteractionPointEncounter(state, startingTime);
	}
}


bool AgentTester::notifyAllTimedWaitingThreads(VexThreadState * state, pthread_mutex_t *mutex) {
	bool returnVal = false;
	if (Tester::virtualExecution) {
		assert(false);
//		long long startingTime = Time::getVirtualTime();
////		vtflog(managerDebug & mypow2(15), managerLogfile, "INTERACTION POINT: suspending thread %s (%lld)\n", state->getName(), state->tid);
//
////		manager->suspendCurrentThread(state, Time::getVirtualTime(), 0);
//
//		manager->lockMutex();
//
//		manager->setCurrentThreadVTLockless(startingTime, state);
//
//
//
//		// Update the time of this thread, in order to set it as the interrupt time of the waiting thread
////		manager-> onThreadInteractionPointEncounter(state, startingTime);
//
//		state->addInvocationPoints();
//
//		long nextWaitingThreadId = waitingOnObjectRegistry->getNext((long)mutex);
//		while (nextWaitingThreadId != 0) {
//			VexThreadState *threadToInterruptState = registry->getCurrentThreadState(nextWaitingThreadId);
//
//			if (threadToInterruptState != NULL && threadToInterruptState->isWaiting()) {
//				threadToInterruptState->setTimedOut(true);
//				returnVal = true;
//				manager->onThreadTimedWaitingEnd(threadToInterruptState, state->getEstimatedRealTime());
////				cout << *state << " ***notifies*** " << *threadToInterruptState << endl;
//			} else {
////				cout << *state << " will not notify anyone " << endl;
//				returnVal = false;	// the thread to be notified was not found - probably finished execution
//			}
//			nextWaitingThreadId = waitingOnObjectRegistry->getNext((long)mutex);
//		}
//
//		manager->unlockMutex();
//		state-> updateCpuTimeClock();
	}

	return returnVal;
}



