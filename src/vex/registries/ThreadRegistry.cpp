/*
 * ThreadRegistry.cpp: Class used to store the threads participating in a Java VTF simulation
 *
 *  Created on: 28 Jun 2010
 *      Author: nb605
 */

#include "ThreadRegistry.h"

#include <signal.h>
#include <algorithm>
#include <cassert>

ThreadRegistry::ThreadRegistry(int _processors) {
	processors = _processors;
	pthread_mutex_init(&threadStateIndex_mutex, NULL);
	threadsBeingSpawned = 0;
	mainThreadState = NULL;
	mainDefined = false;
	totalThreadsControlled = 0;
	forwardLeapsAllowed = true;
}

ThreadRegistry::~ThreadRegistry() {
	pthread_mutex_destroy(&threadStateIndex_mutex);
}

/*
 * Get the Thread state as provided by the id of the Java thread - 2
 * @assumptions: lock is NOT held when the method is called
 * @lock: no-locks
 *
ThreadState* ThreadRegistry::getCurrentThreadState(JNIEnv* jni_env, jthread &thread) {

	// the following two JNI calls will be made only once for the entire execution
	if (getIdMethodId == NULL) {

		if (javaLangThreadClass == NULL) {
			javaLangThreadClass = jni_env->FindClass("java/lang/Thread");
			if (jni_env->ExceptionCheck()) {
				fprintf(stderr, "ThreadManager constructor could not find java/lang/Thread\n");fflush(stderr);
				return NULL;
			}
		}
		getIdMethodId = jni_env->GetMethodID(javaLangThreadClass, "getId", "()J");
		if (jni_env->ExceptionCheck()) {
			fprintf(stderr, "ThreadManager constructor could not find method id for getId\n");fflush(stderr);
			return NULL;
		}
	}

	long threadId= (long) jni_env->CallLongMethod(thread, getIdMethodId);
	if (jni_env->ExceptionCheck()) {
		fprintf(stderr, "ThreadManager getCurrentThreadState could not call object method for Thread.currentThread().getId()\n");fflush(stderr);
		return NULL;
	}

	return getCurrentThreadState(threadId);
}
*/

void ThreadRegistry::add(VexThreadState *state) {
	lockRegistry();
	long stateUniqueLanguageLevelId = state->getUniqueId();


	unordered_map<long, long long>::iterator parentTimeIt = parentThreadIdsToTheirERTs.find(stateUniqueLanguageLevelId);
	if (parentTimeIt != parentThreadIdsToTheirERTs.end()) {
		long long estimatedRealTimeWhenYourParentSpawnedYou = parentTimeIt -> second;
		parentThreadIdsToTheirERTs.erase(parentTimeIt);
		state->setEstimatedRealTime(estimatedRealTimeWhenYourParentSpawnedYou);
	} else {
		// In Java all threads apart from  main should find the thread that spawned them - in other languages depends on the implementation
		// assert((strcmp(state->getName(), "main") == 0));
	}

	threadStateIndex[stateUniqueLanguageLevelId] = state;

	sortableVectorOfThreadStateIndices.push_back(state);
	if (strcmp(state->getName(), "main") == 0) {
		mainThreadState = state;
		mainDefined = true;
	}
	++totalThreadsControlled;
	if (threadsBeingSpawned) {
		--threadsBeingSpawned;
	}
	unlockRegistry();
}

void ThreadRegistry::remove(VexThreadState *state) {
	lockRegistry();
	state->notifyParentWaitingForYouToJoin();
	threadStateIndex.erase(state->getUniqueId()); // no-one will be able to access the thread now apart from the thread itself - the scheduler cannot suspend you anymore

	sortableVectorOfThreadStateIndices.erase(std::remove(sortableVectorOfThreadStateIndices.begin(), sortableVectorOfThreadStateIndices.end(), state), sortableVectorOfThreadStateIndices.end());

	if (state == mainThreadState) {
		mainThreadState = NULL;
	}
	unlockRegistry();
}


void ThreadRegistry::notifyParentWaitingToJoin(VexThreadState *state) {
	lockRegistry();
	state->notifyParentWaitingForYouToJoin();
	unlockRegistry();
}


bool ThreadRegistry::noProcessThreadsLeft() {
	bool response = false;
	lockRegistry();
	if (threadsBeingSpawned == 0 && mainThreadState == NULL && isEmpty()) {
		response = true;
	}
	unlockRegistry();
	return response;
}

void ThreadRegistry::newThreadBeingSpawned() {
	lockRegistry();
	++threadsBeingSpawned;
	unlockRegistry();
}

void ThreadRegistry::newThreadBeingSpawned(long parentThreadId, long threadToBeSpawnedApplicationLanguageId) {

	lockRegistry();
	unordered_map<long, VexThreadState *>::iterator stateIt = threadStateIndex.find(parentThreadId);
	if (stateIt != threadStateIndex.end()) {
		parentThreadIdsToTheirERTs[threadToBeSpawnedApplicationLanguageId] = (stateIt->second)->getEstimatedRealTime();
	} else if (parentThreadId == 0) {
		parentThreadIdsToTheirERTs[threadToBeSpawnedApplicationLanguageId] = 0;
	} else {
		assert(false);	// we only allow main to bypass this like this: otherwise it is not possible that the current parent is not part of VEX
	}
	++threadsBeingSpawned;
	unlockRegistry();
}

// Get the parent thread that logged your id here - if you cannot find it, then you should not be part of VEX
// @Registry assumed to be locked
bool ThreadRegistry::hasThreadParentRegisteredThisThread(long newlySpawnedThreadId) {
	lockRegistry();

	unordered_map<long, long long>::iterator stateIt = parentThreadIdsToTheirERTs.find(newlySpawnedThreadId);
	if (stateIt != parentThreadIdsToTheirERTs.end()) {
		unlockRegistry();

		return true;
	} else {
		unlockRegistry();

		return false;
	}
}


int ThreadRegistry::getThreadsBeingSpawned() {
	int temp;
	lockRegistry();
	temp = threadsBeingSpawned;
	unlockRegistry();
	return temp;
}

bool ThreadRegistry::atLeastOneThreadBeingSpawned() {
	lockRegistry();
	bool result = (threadsBeingSpawned != 0);
	unlockRegistry();
	return result;
}

void ThreadRegistry::newThreadStarted() {
	lockRegistry();
	if (threadsBeingSpawned) {
		--threadsBeingSpawned;
	}
	unlockRegistry();
}
/*
 * areAnyOtherThreadsActiveInFormerTime LiveThreads method
 *
 * Function to let the scheduler know whether there are any live threads externally from the
 * runnable threads list. This can only happen in the IO_PARALLEL_LAX scheme, where threads
 * performing I/O are alive externally from the list or threads may remain live (RUNNING) for
 * a brief time until they suspend when they return from an I/O call.
 */
//short ThreadRegistry::areAnyOtherThreadsActiveInFormerTime(VexThreadState *state, long long *minimumMaybeAliveTime) {
//	if (threadsBeingSpawned > 0) {
//		LOG(registryLogger, logINFO) << "VFL blocked because threadsBeingSpawned =  " << threadsBeingSpawned << endl;
//		return AT_LEAST_ONE_ALIVE;
//	}
//
//	if (!isEmpty()) {
//		bool otherThreadsActiveInFormerTime = false;
//		bool maybeOneAlive = false;
//		unordered_map<long, VexThreadState*>::iterator i = threadStateIndex.begin();
//		VexThreadState *tempState;
//		while (i != threadStateIndex.end()) {
//
//			tempState = (*i).second;
//			if (tempState != NULL && tempState->getEstimatedRealTime() < state->getEstimatedRealTime()) {
//				if (tempState->isDefinitelyActive()) {
//					LOG(registryLogger, logINFO) << "VFL definitely blocked because of: " << *tempState << endl;
//					return AT_LEAST_ONE_ALIVE;
//
//				} else if (tempState->maybeActive()) {
//					otherThreadsActiveInFormerTime = true;
//
//					if (minimumMaybeAliveTime != NULL) {
//						if (tempState->getEstimatedRealTime() < *minimumMaybeAliveTime) {
//							*minimumMaybeAliveTime = tempState->getEstimatedRealTime();
//
//							LOG(registryLogger, logINFO) << "VFL maybe blocked because of: " << *tempState << endl;
//
//							//cout << "MAYBE WAITING BECAUSE OF " << *tempState << endl;
//							//syscall(SYS_tkill, tempState->getId(), SIGALRM);
//							//sleep(2);
//
//							maybeOneAlive = true;
//						}
//					}
//
//				}
//			}
//
//			++i;
//		}
//
//		if (maybeOneAlive) {
//			return MAYBE_ONE_ALIVE;
//		}
//	}
//	return NONE_ALIVE;
//}


bool ThreadRegistry::existsThreadThatIsSuspendedOnHigherErtThanTimeoutAndMightUnblockNativeWaitingThread(VexThreadState *state) {
	unordered_map<long, VexThreadState*>::iterator i = threadStateIndex.begin();
	VexThreadState *tempState;
	while (i != threadStateIndex.end()) {

		tempState = (*i).second;
		//Gio: allow for timed waiting thread as well
		if (tempState != NULL && (tempState->isTimedWaiting() || tempState->isSuspended()) && tempState->getEstimatedRealTime() >= state->getEstimatedRealTime()) {
			return true;
		}
		++i;
	}
	return false;
}

bool ThreadRegistry::areAllNativeWaitingThreadsBlockedAccordingToSystemState(VexThreadState *state) {
	lockRegistry();
	if (forwardLeapsAllowed) {
		unordered_map<long, VexThreadState*>::iterator i = threadStateIndex.begin();
		VexThreadState *tempState;
		while (i != threadStateIndex.end()) {
			tempState = (*i).second;
			if (tempState != NULL && tempState->getEstimatedRealTime() <= state->getEstimatedRealTime() && (tempState->isNativeWaiting() || tempState->isInAnyIo())) {
				if (!tempState->isThreadSystemStateBlocked()) {
					unlockRegistry();
					return false;
				}
			}
			++i;
		}
		unlockRegistry();
		return true;
	} else {
		unlockRegistry();
		return false;
	}
}


long long ThreadRegistry::getSummedErtOfAllThreads() {
	long long totalErt = 0;
	lockRegistry();

	unordered_map<long, VexThreadState*>::iterator i = threadStateIndex.begin();
	VexThreadState *tempState;
	while (i != threadStateIndex.end()) {
		tempState = (*i).second;
		if (tempState != NULL) {
			totalErt += tempState->getEstimatedRealTime() + tempState->getLocalTime() + tempState->getCurrentState() + tempState->getIoCPUTime();
		}
		++i;
	}
	unlockRegistry();
	return totalErt;
}

short ThreadRegistry::areAnyOtherThreadsActiveInFormerTime(VexThreadState *state, long long *remainingTime) {

	lockRegistry();
	if (threadsBeingSpawned > 0) {
		LOG(registryLogger, logINFO) << "VFL blocked because threadsBeingSpawned =  " << threadsBeingSpawned << endl;
		unlockRegistry();
		return AT_LEAST_ONE_ALIVE;
	}

	if (!forwardLeapsAllowed) {
		LOG(registryLogger, logINFO) << "VFL blocked because forward leaps are not allowed (probably due to a garbage collection enabled flag)" << endl;
		unlockRegistry();
		return AT_LEAST_ONE_ALIVE;
	}

	if (!isEmpty()) {
//		bool otherThreadsActiveInFormerTime = false;
//		bool maybeOneAlive = false;

//		unordered_map<long, VexThreadState*>::iterator i = threadStateIndex.begin();
//		VexThreadState *tempState;
//		while (i != threadStateIndex.end()) {

		unordered_map<long, VexThreadState*>::iterator i = threadStateIndex.begin();
		VexThreadState *tempState;
		while (i != threadStateIndex.end()) {

			tempState = (*i).second;
			if (tempState != NULL && tempState->getEstimatedRealTime() < state->getEstimatedRealTime()) {
				if (tempState->isDefinitelyActive()) {
					LOG(registryLogger, logINFO) << "VFL definitely blocked because of: " << *tempState << endl;
					unlockRegistry();
					return AT_LEAST_ONE_ALIVE;
				} else if (tempState->isNativeWaiting()) {//  || tempState->isInPossiblyBlockingIo()) {
//					return MAYBE_ONE_ALIVE;
					if (existsThreadThatIsSuspendedOnHigherErtThanTimeoutAndMightUnblockNativeWaitingThread(state)) {
						unlockRegistry();
						return MAYBE_ONE_ALIVE; 	// if such a thread exists, let the leap happen to unblock the native waiting thread
					} else {
						unlockRegistry();
						return AT_LEAST_ONE_ALIVE;	 // the native waiting thread should be resumed differently
					}
				} else if (remainingTime != NULL && (state->getEstimatedRealTime() - tempState->getEstimatedRealTime()) < (*remainingTime * 2)) {
					// Logic: if you want to leap remainingTime in the future, check all WAITING/-1 threads that
					// started waiting at most remainingTime ago. The idea: the longer the leap, the more threads are checked as mutable
					// (i.e. capable of changing their state at any point).
					unlockRegistry();
					return MAYBE_ONE_ALIVE;

				}
//				else {
//					return MAYBE_ONE_ALIVE;
//				}

//				else if (tempState->maybeActive()) {
//					otherThreadsActiveInFormerTime = true;
//
//					if (minimumMaybeAliveTime != NULL) {
//						if (tempState->getEstimatedRealTime() < *minimumMaybeAliveTime) {
//							*minimumMaybeAliveTime = tempState->getEstimatedRealTime();
//
//							LOG(registryLogger, logINFO) << "VFL maybe blocked because of: " << *tempState << endl;
//
//							//cout << "MAYBE WAITING BECAUSE OF " << *tempState << endl;
//							//syscall(SYS_tkill, tempState->getId(), SIGALRM);
//							//sleep(2);
//
//							maybeOneAlive = true;
//						}
//					}
//
//				}
			}

			++i;
		}

//		if (maybeOneAlive) {
//			return MAYBE_ONE_ALIVE;
//		}
	}
	unlockRegistry();
	return NONE_ALIVE;
}

bool ThreadRegistry::isMainStillAlive() {
	return !mainDefined || (mainThreadState != NULL);
}


/*
 * Utility function to update the states of all threads
 * @lock: no lock - should be held by the calling method
 */
void ThreadRegistry::resetThreads() {
	if (!isEmpty()) {
		unordered_map<long, VexThreadState*>::iterator i = threadStateIndex.begin();
		VexThreadState *tempState;
		while (i != threadStateIndex.end()) {
			tempState = (*i).second;
			if (tempState != NULL) {
				tempState->setInvocationPoints(0);
			}
			++i;
		}

	}
}


bool ThreadRegistry::sanityTest(const int &runnableThreadSize, bool ioMethodInQueue) {

	int countThreadListSize = 0;	// count how many should actually be in the thread queue
	int countRunningThreads = 0;


	lockRegistry();

	if (!isEmpty()) {
		unordered_map<long, VexThreadState*>::iterator i = threadStateIndex.begin();

		VexThreadState *tempState;
		while (i != threadStateIndex.end()) {

			tempState = (*i).second;
			if (tempState != NULL && tempState->getState() != VexThreadStates::REGISTERING && tempState->getState() != VexThreadStates::VEX_ZOMBIE) {

				if (ioMethodInQueue) {
					if (!((tempState->getState() == VexThreadStates::LEARNING_IO && tempState->getTimeout()==-1) || tempState->getState() == VexThreadStates::NATIVE_WAITING || tempState->isRunning() || tempState->wasAwaken() || (tempState->getState() == VexThreadStates::WAITING && tempState->getTimeout() <= 0))) {
						countThreadListSize++;
					}
				} else {
					if ((tempState->getState() == VexThreadStates::SUSPENDED && !tempState->wasAwaken()) || (tempState -> getState() == VexThreadStates::WAITING && tempState -> getTimeout() > 0)) {
						countThreadListSize++;
					}
				}

				if (tempState->getState() == VexThreadStates::RUNNING) {
					countRunningThreads++;
				}
			}
			++i;
		}

		if (abs(runnableThreadSize - countThreadListSize) > (processors+1)) {
			cerr << "runnableThreadSize - countThreadListSize is > " << (processors+1)<<" -> " << runnableThreadSize << " - " << countThreadListSize << endl;
			unlockRegistry();
			return false;
//			vtflog(true, stderr, "xxxxxxxxxxxxxxxxxx - ERROR in states - difference in qsze: q->size()=%d - should be %d\n", runnableThreadSize, countThreadListSize);
		}

		if (countRunningThreads > (processors+1)) {
			cerr << "More than one RUNNING threads!!! RUNNING threads = " << countRunningThreads << endl;
			unlockRegistry();
			return false;
//			vtflog(true, stderr, "xxxxxxxxxxxxxxxxxx - ERROR in states - %d threads simultaneously in RUNNING state\n", countRunningThreads);
		}


	}

	unlockRegistry();

	return true;
}


void ThreadRegistry::getStatesSnapshot(int *states_count) {

	for (int i =0 ; i<POSSIBLE_THREADSTATES; i++) {
		states_count[i] = 0;
	}

	if (!isEmpty()) {
		unordered_map<long, VexThreadState*>::iterator i = threadStateIndex.begin();

		VexThreadState *tempState;
		while (i != threadStateIndex.end()) {

			tempState = (*i).second;
			if (tempState != NULL) {
				++states_count[tempState->getState()];
			}
			++i;
		}
	}

}


int ThreadRegistry::getRunningThreadsCount() {
	int count = 0;
	if (!isEmpty()) {
		unordered_map<long, VexThreadState*>::iterator i = threadStateIndex.begin();

		VexThreadState *tempState;
		while (i != threadStateIndex.end()) {

			tempState = (*i).second;
			if (tempState != NULL && tempState->isRunning()) {
				++count;
			}
			++i;
		}
	}
	return count;

}


void ThreadRegistry::printRunningThreads() {

	lockRegistry();

	if (!isEmpty()) {
		unordered_map<long, VexThreadState*>::iterator i = threadStateIndex.begin();

		VexThreadState *tempState;
		while (i != threadStateIndex.end()) {

			tempState = (*i).second;
			if (tempState != NULL && tempState->isRunning()) {
				cout << "RUNNING: " << tempState->getName() << " " << tempState->getEstimatedRealTime() << endl;
			}
			++i;
		}
	}

	unlockRegistry();

}



/*
 * Utility function to print the thread states linked list
 * Note: Do not call any jvmti methods to avoid scheduler suspending
 * int runnableThreadSize: how many threads are currently in the runnable thread list
 * bool ioMethodInQueue: are threads performing I/O methods supposed to be in the queue or not
 * The parameters are used to check whether the state of VTF is valid
 * @lock: no lock - should only be called by scheduler
 */

void ThreadRegistry::printThreadStates(const int &runnableThreadSize, bool ioMethodInQueue) {
	int countThreadListSize = 0;	// count how many should actually be in the thread queue
	int countRunningThreads = 0;

	if (!isEmpty()) {
		std::sort(sortableVectorOfThreadStateIndices.begin(), sortableVectorOfThreadStateIndices.end(), threadStatePtr_compare_ascending());
		//unordered_map<long, VexThreadState*>::iterator i = threadStateIndex.begin();
		vector<VexThreadState*>::iterator i = sortableVectorOfThreadStateIndices.begin();

		int count = 0;
		VexThreadState *tempState;

		string *tempString;
		string *prevStateString;
//		while (i != threadStateIndex.end()) {
//
//			tempState = (*i).second;
		while (i != sortableVectorOfThreadStateIndices.end()) {

			tempState = (*i);
			if (tempState != NULL) {

				tempString = tempState->stateToString(true);
				prevStateString = tempState->stateToString(false);
				if (tempString != NULL) {

					//cout << count++ << ") " << *tempState << " " << tempString->c_str() << ((tempState->isAwakeningAfterJoin())?"* ":" ") << "(" << prevStateString->c_str() << ") " << tempState -> getResumedLastAt()/1000000 << "/" << tempState -> getResumedLastAtReal()/1000000 << " ";
					cout << count++ << ") " << tempState->getEstimatedRealTime()/1e6 << " " << tempState->getCurrentStateName() << " " << *tempState << " " << tempString->c_str() << ((tempState->isAwakeningAfterJoin())?"* ":" ") << "(" << prevStateString->c_str() << ") " << tempState -> getResumedLastAt()/1000000 << "/" << tempState -> getResumedLastAtReal()/1000000 << " ";
					cout << endl;

					delete tempString; 	// allocated within stateToString
					delete prevStateString; 	// allocated within stateToString
				}

				if (ioMethodInQueue) {
					if (!(tempState->getState() == VexThreadStates::NATIVE_WAITING || tempState->getState() == VexThreadStates::RUNNING || tempState->wasAwaken() || (tempState->getState() == VexThreadStates::WAITING && tempState->getTimeout() <= 0))) {
						countThreadListSize++;
					}
				} else {
					if ((tempState->getState() == VexThreadStates::SUSPENDED && !tempState->wasAwaken()) || (tempState->getState() == VexThreadStates::WAITING && tempState->getTimeout() != -1)) {
						countThreadListSize++;
					}
				}

				if (tempState->getState() == VexThreadStates::RUNNING) {
					countRunningThreads++;
				}

			}
			++i;

		}

		if (runnableThreadSize != -1 && runnableThreadSize != countThreadListSize) {
			//vtflog(true, stderr, "xxxxxxxxxxxxxxxxxx - ERROR in states - difference in qsze: q->size()=%d - should be %d\n", runnableThreadSize, countThreadListSize);
		}

		if (countRunningThreads > 1) {
			//vtflog(true, stderr, "xxxxxxxxxxxxxxxxxx - ERROR in states - %d threads simultaneously in RUNNING state\n", countRunningThreads);
		}


	}

	fprintf(stderr, "*****************************\n");
}



/***
 * Coordinate joining threads -
 * returns false if threads are still being created,
 * so that if we don't find a thread in the registry we know that it has died.
 * Otherwise, a thread that was never born might wrongly set the join flag to true
 */
bool ThreadRegistry::coordinateJoiningThreads(VexThreadState **state, const long &joiningThreadId) {
	lockRegistry();
	if (threadsBeingSpawned > 0) {
		unlockRegistry();
		return false;
	}

	unordered_map<long, VexThreadState *>::iterator stateIt = threadStateIndex.find(joiningThreadId);

	if (stateIt != threadStateIndex.end()) {
		// If the thread is still running then register yourself to be notified when it dies
		VexThreadState *otherThread = stateIt->second;
		if (!otherThread->isDead()) {
			otherThread->setParentThreadWaitingYouToJoin(state);
//			(*state)->setAwakeningFromJoin(false);
			//and send me to sleep - Gio
//			unlockRegistry();
//			return false;
		} else {
			(*state)->setAwakeningFromJoin(true);
		}
	} else {
		// If the thread is already dead or has notified VEX that it is about to die
		(*state)->setAwakeningFromJoin(true);
	}
	unlockRegistry();
	return true;
}


void ThreadRegistry::cleanup(VexThreadState *state) {
	lockRegistry();
	delete state;
	state = NULL;
	unlockRegistry();
}


void ThreadRegistry::forceNativeWaitingPrintTheirStackTraces() {
	lockRegistry();
	unordered_map<long, VexThreadState *>::iterator stateIt = threadStateIndex.begin();
	while (stateIt != threadStateIndex.end()) {
		VexThreadState *state = stateIt->second;
		if (state->isNativeWaiting()) {
			syscall(SYS_tkill, state->getId(), SIGALRM);
			sleep(2);
		}
		++stateIt;
	}
	unlockRegistry();
}



bool ThreadRegistry::leapForbiddingRulesApply(VexThreadState *state) {
//	lockRegistry();

	// RULE 1: No threads being spawned
	if (threadsBeingSpawned > 0) {
//		unlockRegistry();
		LOG(registryLogger, logINFO) << "VFL is not allowed because threadsBeingSpawned =  " << threadsBeingSpawned << endl;
//		cout << "VFL is not allowed because threadsBeingSpawned =  " << threadsBeingSpawned << endl;
		return true;	// the forbidding rules apply: a thread is being spawned
	}

	if (!isEmpty()) {

		// RULE 2: No threads performing non-blocking I/O
		VexThreadState *tempState;
		unordered_map<long, VexThreadState*>::iterator i = threadStateIndex.begin();

		while (i != threadStateIndex.end()) {
			tempState = (*i).second;
			if (tempState->getEstimatedRealTime() <= state->getEstimatedRealTime() && (tempState->isDefinitelyActiveInIo() || tempState->isInTransitionFromOneStateToTheNextBeforeVexKnowsAboutIt2() || tempState->isNativeWaiting())) {
//				unlockRegistry();
				LOG(registryLogger, logINFO) << "VFL to " << state->getEstimatedRealTime()/1e6 << " is not allowed because of: " << *tempState << endl;
//				cout << "VFL to " << state->getEstimatedRealTime()/1e6 << " is not allowed because of: " << *tempState << endl;
				return true;
			}
			++i;
		}

		// Estimations 1-2: The waiting and native waiting threads have NOT changed their state without VEX knowing about it
		i = threadStateIndex.begin();
		while (i != threadStateIndex.end()) {
			tempState = (*i).second;
			if (tempState->getEstimatedRealTime() <= state->getEstimatedRealTime() && tempState->blockedStateIsNotVerifiedBySystemState()) {
//				unlockRegistry();
				LOG(registryLogger, logINFO) << "VFL to " << state->getEstimatedRealTime()/1e6 << " is not allowed because a blocked thread does not seem to be blocked anymore: " << *tempState << endl;
//				cout << "VFL to " << state->getEstimatedRealTime()/1e6 << " is not allowed because a blocked thread does not seem to be blocked anymore: " << *tempState << endl;
				return true;
			}
			++i;
		}
	}

//	unlockRegistry();
	return false;
}


int ThreadRegistry::getRegistryThreadsSystemAndVexStates(VexThreadState *state, struct vex_and_system_states &vass) {
//	lockRegistry();

	if (!isEmpty()) {

		// RULE 2: No threads performing non-blocking I/O
		VexThreadState *tempState;
		unordered_map<long, VexThreadState*>::iterator i = threadStateIndex.begin();
		cout << "------------ Leap of " << state->getName() << " (" << state->getId() << ") ------------" << endl;
		while (i != threadStateIndex.end()) {
			tempState = (*i).second;
			if (tempState->getEstimatedRealTime() <= state->getEstimatedRealTime()) {
				if (tempState->isDefinitelyActiveInIo()) {
					++vass.activeInIoThreads;
				} else if (tempState->isSuspended()) {
					++vass.suspended;
				} else if (tempState->isRunning()) {
					++vass.running;
				} else if (tempState->isNativeWaiting()) {
					++vass.nwThreads;

				} else if (tempState->isWaiting()) {
					++vass.waitingThreads;

				} else if (tempState->isBlocked()) {	// this is hack: it identifies a blocking I/O thread only because it follows the isNative() and isWaiting() conditions which have been evaluated to false
					++vass.blockedInIoThreads;
				}

				if (tempState->isAwakeningAfterJoin()) {
					++vass.isAwakeningAfterJoin;
				}
				if (tempState->isRegistering()) {
					++vass.isRegistering;
				}
				if (tempState->wasAwaken()) {
					++vass.isAwakenInVex;
				}
				if (tempState->isWaitingRealCodeToCompleteAfterModelSimulation()) {
					++vass.isWaitingRealCodeToCompleteAfterModelSimulation;
				}

				char systemStateOfTemp = tempState->getSystemThreadState();
				switch (systemStateOfTemp) {
					case 'R': ++vass.systemRunning; break;
					case 'S': ++vass.systemSleeping; break;
					case 'D': ++vass.systemD; break;
					case 'T': ++vass.systemStopped; break;
					case 'Z': ++vass.systemZombies; break;
					default: break;
				}

				cout << tempState->getName() << " (" << tempState->getId() << ") " << tempState->getCurrentStateName() << " " << systemStateOfTemp << " " << tempState->isAwakeningAfterJoin() << " " << tempState->getWaitingInNativeVTFcode() << endl;

			}

			++i;
		}
		cout << "------------" << endl;
	}
//	unlockRegistry();

	return threadsBeingSpawned;
}


void ThreadRegistry::setNativeWaiting(VexThreadState *state) {

}
void ThreadRegistry::unsetNativeWaiting(VexThreadState *state) {

}
void ThreadRegistry::pollNativeWaitingThreads() {

}

void NwPollingThreadRegistry::pollNativeWaitingThreads() {
	lockRegistry();
//	unordered_map<long, VexThreadState *>::iterator stateIt = threadStateIndex.begin();
//	while (stateIt != threadStateIndex.end()) {
//		VexThreadState *state = stateIt->second;
//		// cannot really hold thread lock in here
//		if (state->isNativeWaiting()) {
//			syscall(SYS_tkill, state->getId(), SIGHUP);
//		}
//		++stateIt;
//	}
	if (!nativeWaitingThreadsPQueue->empty()) {
		VexThreadState *state = nativeWaitingThreadsPQueue->top();

		// We use this flag based scheme to avoid synchronizing with the response from the thread,
		// whilst ensuring that no multiple signals are sent from more than one schedulers (thread handlers in multicore)
		// or the same scheduler
		if (state != NULL && !state->isThreadBeingCurrentlyPolledForNativeWaiting()) {
			state->flagThreadBeingCurrentlyPolledForNativeWaiting();
			syscall(SYS_tkill, state->getId(), SIGHUP);
		}
	}
	unlockRegistry();
}

void NwPollingThreadRegistry::setNativeWaiting(VexThreadState *state) {
	lockRegistry();
	state->clearThreadBeingCurrentlyPolledForNativeWaiting();
	nativeWaitingThreadsPQueue->push(state);
	unlockRegistry();
}
void NwPollingThreadRegistry::unsetNativeWaiting(VexThreadState *state) {
	lockRegistry();
	nativeWaitingThreadsPQueue->erase(state);
	unlockRegistry();
}

