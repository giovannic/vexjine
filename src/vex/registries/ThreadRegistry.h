/*
 * ThreadRegistry.h
 *
 *  Created on: 28 Jun 2010
 *      Author: nb605
 */

#ifndef THREADREGISTRY_H_
#define THREADREGISTRY_H_

#include "ThreadState.h"
#include "Logger.h"
#include "AdaptiblePQueue.h"

#include <map>
using namespace std;

enum {
	AT_LEAST_ONE_ALIVE = 0,
	MAYBE_ONE_ALIVE = 1,
	NONE_ALIVE = 2
};

class ThreadRegistry {
public:
	ThreadRegistry(int processors);

	//ThreadState* getCurrentThreadState(long &threadId);
	/*
	 * Get the Thread state as provided by the id of the Java thread:
	 * Actually parse the ThreadStates list to find or not the requested thread
	 * @lock: protecting threadStates
	 */
	inline VexThreadState* getCurrentThreadState(const long &threadId) {
		pthread_mutex_lock(&threadStateIndex_mutex);
		unordered_map<long, VexThreadState *>::iterator stateIt = threadStateIndex.find(threadId);
		pthread_mutex_unlock(&threadStateIndex_mutex);

		if (stateIt != threadStateIndex.end()) {
			return stateIt->second;	//
		} else {
			return NULL;
		}
	};

//	ThreadState* getCurrentThreadState(JNIEnv* jni_env, jthread &thread);
	inline int getSize() {
		return threadStateIndex.size();
	};
	inline bool isEmpty() {
		return threadStateIndex.empty();
	};
	void add(VexThreadState *state);
	void remove(VexThreadState *state);

	void setLog(Log *logger) {
		registryLogger = logger;
	}

	inline void lockRegistry() {
		pthread_mutex_lock(&threadStateIndex_mutex);
	}

	inline void unlockRegistry() {
		pthread_mutex_unlock(&threadStateIndex_mutex);
	}

	bool areAllNativeWaitingThreadsBlockedAccordingToSystemState(VexThreadState *state);

	short areAnyOtherThreadsActiveInFormerTime();
	long long getSummedErtOfAllThreads();
	short areAnyOtherThreadsActiveInFormerTime(VexThreadState *state, long long *timeOfLeapingThread);
	void newThreadBeingSpawned();
	void newThreadBeingSpawned(long parentThreadId, long threadToBeSpawnedApplicationLanguageId);
	bool hasThreadParentRegisteredThisThread(long newlySpawnedThreadId);

	int getThreadsBeingSpawned();
	void newThreadStarted();
	bool isMainStillAlive();

	void notifyParentWaitingToJoin(VexThreadState *state);
	bool noProcessThreadsLeft();

	// Test VTF sanity
	bool sanityTest(const int &runnableThreadSize, bool ioMethodInQueue);
	void getStatesSnapshot(int *states_count);
	int getRunningThreadsCount();
	void printRunningThreads();

	void resetThreads();	// set all threads to an initial stage (used for debugging)
	void printThreadStates(const int &runnableThreadSize, bool ioMethodInQueue);
	virtual ~ThreadRegistry();

	unsigned int getTotalThreadsControlled() {
		return totalThreadsControlled;
	};
	bool coordinateJoiningThreads(VexThreadState **state, const long &joiningThreadId);
	void cleanup(VexThreadState *state);

	void forbidForwardLeaps() {
		forwardLeapsAllowed = false;
	}
	void allowForwardLeaps() {
		forwardLeapsAllowed = true;
	}

	bool atLeastOneThreadBeingSpawned();
	void forceNativeWaitingPrintTheirStackTraces();
	virtual void pollNativeWaitingThreads();
	virtual void setNativeWaiting(VexThreadState *state);
	virtual void unsetNativeWaiting(VexThreadState *state);

	bool leapForbiddingRulesApply(VexThreadState *state);
	int getRegistryThreadsSystemAndVexStates(VexThreadState *state, struct vex_and_system_states &vass);

protected:
	bool existsThreadThatIsSuspendedOnHigherErtThanTimeoutAndMightUnblockNativeWaitingThread(VexThreadState *state);
	/*
	jclass javaLangThreadClass;
	jmethodID currentThreadMethodId;
	jmethodID getIdMethodId;
	jmethodID getVtfProfiledMethodId;
*/
	unordered_map<long, VexThreadState *> threadStateIndex;				// thread system id to thread state
	vector<VexThreadState *> sortableVectorOfThreadStateIndices;
	unordered_map<long, long long> parentThreadIdsToTheirERTs;	// parent thread language id to parent thread state

	pthread_mutex_t threadStateIndex_mutex;

	int threadsBeingSpawned; 	// counter denoting how many threads have been set to be spawned
	VexThreadState *mainThreadState;
	bool mainDefined;
	bool forwardLeapsAllowed;
	unsigned int totalThreadsControlled;
	int processors;
	Log *registryLogger;
};


class NwPollingThreadRegistry : public ThreadRegistry {
public:
	NwPollingThreadRegistry(int processors) : ThreadRegistry(processors) {
		nativeWaitingThreadsPQueue = new AdaptiblePQueue<VexThreadState *, deque<VexThreadState *>, threadStatePtr_compare>;
	};
	virtual void pollNativeWaitingThreads();
	virtual void setNativeWaiting(VexThreadState *state);
	virtual void unsetNativeWaiting(VexThreadState *state);

protected:
	AdaptiblePQueue<VexThreadState *, deque<VexThreadState *>, threadStatePtr_compare> *nativeWaitingThreadsPQueue;
};


#endif /* THREADREGISTRY_H_ */
