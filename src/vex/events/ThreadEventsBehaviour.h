/*
 * ThreadEventsBehaviour.h
 *
 *  Created on: 18 Oct 2011
 *      Author: root
 */

#ifndef THREADEVENTSBEHAVIOUR_H_
#define THREADEVENTSBEHAVIOUR_H_

#include <vector>
#include <string>
#include <pthread.h>

// Forward declarations
class ThreadManagerRegistry;
class ThreadManager;
class VexThreadState;
class ThreadRegistry;
class EventLogger;
class ObjectRegistry;
class AggregateStateCounters;
class LockRegistry;
class PapiProfiler;

class ThreadEventsBehaviour {
public:
	ThreadEventsBehaviour(ThreadManagerRegistry *_managers, ThreadRegistry *_registry, EventLogger *_eventLogger, ObjectRegistry *_waitingOnObjectRegistry, LockRegistry *_lockRegistry, AggregateStateCounters *_aggregateStateTransitionCounters, const bool &_stackTraceMode, const bool &_usingVtfScheduler, PapiProfiler *_hpcProfiler);
	virtual ~ThreadEventsBehaviour();

	void onThreadMainStart(const long &threadId);
	void onStart(const long &threadId, const char *threadName);
	void registerThreadsToBeExcludedFromSimulation(char *excludedThreadsFile);
	void addThreadToBeExcludedFromSimulation(const char *threadName);


	// Wrapped waiting: calls to be invoked before and after a waiting call
	void onWrappedWaitingStart();
//	void onWrappedWaitingStart(const long &objectId);	// waiting on some unique id allows correct handling of signalling on that id
	void onWrappedWaitingEnd();

	void onWrappedTimedWaitingStart(const long &objectId, long &timeout, const int &nanoTimeout);
	void onWrappedTimedWaitingEnd();
	void onWrappedTimedWaitingInterrupt(const long &objectId);

	bool beforeReleasingLockOfAnyReplacedWaiting(const long &objectId, const bool &updateLockRegistry);		// invoke this before releasing the lock of the waiting call to be replaced
	bool beforeReleasingLockOfAnyReplacedWaiting(const long &cond, const long &mutex, const bool &updateLockRegistry);
	bool onReplacedWaiting(const long &objectId, const bool &updateLockRegistry);
	bool onReplacedWaiting(const long &cond, const long &mutex, const bool &updateLockRegistry);
	bool onReplacedTimedWaiting(const long &objectId, const long &timeout, const int &nanoTimeout, const bool &updateLockRegistry);
	bool onReplacedTimedWaiting(const long &cond, const long &mutex, const long &timeout, const int &nanoTimeout, const bool &updateLockRegistry);

	bool onSignallingOnObject(const long &objectId);
	bool onBroadcastingOnObject(const long &objectId);

	void onRequestingLock(const long &objectId);
	void onReleasingLock(const long &objectId);

//	void onWaitingStart();
//	void onWaitingEnd();
//
//	void onNativeWaitingStart();
//	void onNativeWaitingEnd();
//
//	void onContendedLockEnter();
//	void onContendedLockEntered();

	bool onSleep(const long &timeout, const int &nanoTimeout);
	bool onInterrupt(const long &threadId);


	// Used for explicit locking - overloaded to allow easy integration with POSIX synchronization primitives
	//TODO: remove me only for JAVEX
	bool beforeTimedWaitingOn(pthread_mutex_t * mutex, const bool &updateLockRegistry);
	bool onTimedWaitingOn(pthread_mutex_t * mutex, const long &timeout, const int &nanoTimeout, const bool &updateLockRegistry);


	bool beforeReleasingLockOfAnyReplacedWaiting(pthread_mutex_t * mutex, const bool &updateLockRegistry);
	bool onReplacedWaiting(pthread_mutex_t * mutex, const bool &updateLockRegistry);
	bool onReplacedTimedWaiting(pthread_mutex_t * mutex, const long &timeout, const int &nanoTimeout, const bool &updateLockRegistry);
	bool onSignallingOnObject(pthread_mutex_t * mutex);
	bool onBroadcastingOnObject(pthread_mutex_t *mutex);
	void onRequestingLock(pthread_mutex_t * mutex);
	void onReleasingLock(pthread_mutex_t * mutex);

	void beforeCreatingThread(long threadToBeSpawnedApplicationLanguageId);
	void afterCreatingThread();

	void onJoin(const long &joiningThreadId);

	// Explicit calls mean that the code will block externally of VEX, but VEX will note the thread state changes
//	void onExplicitWaitingStart();
//	void onExplicitWaitingEnd();

//	void onExplicitTimedWaitingStart(const long &objectId, const long &timeout);


	void onPark(const bool &isAbsoluteTimeout, const long &timeout);
	void onParked();

	void onUnpark(const long &unparkingThreadId);

	void onInteractionPoint();
	void onYield();

	void onBackgroundLoadExecutionStart();									// let VEX know that a background process (supposedly CPU occupying) is about to start - its duration is measured in wallclock time
	void onBackgroundLoadExecutionEnd();									// let VEX know that a background process (supposedly CPU occupying) has finished - its duration is measured in wallclock time
	void onBackgroundLoadExecutionEndAt(const long long &executionDuration);// let VEX know that a background process (supposedly CPU occupying) has finished - its (possibly virtual) duration is provided by argument executionDuration

	void onEnd();

	long long getTime();
	void ensureThreadIsNotInNativeWaitingStateWhenEnteringVex();
//	bool ensureThreadIsNotInNativeWaitingStateWhenEnteringVex(VexThreadState *state, const long long &startingTime);
	ThreadManager *getCurrentlyControllingManagerOf(VexThreadState *state);


	void haltSuspendForAwhile();

	void beforeReinstrumentation();
	void afterReinstrumentation();

	void setProfilerType(EventLogger *_eventLogger, const bool &_stackTraceMode) {
		eventLogger = _eventLogger;
		stackTraceMode = _stackTraceMode;
	}

protected:
	long ensureTimeoutIsLessThanMax(const long &timeout, const int &nanoTimeout);
	bool isThreadInExcludedThreadNamesList(const char *threadName);

	void onLocklessReleasingLock(VexThreadState *state, const long &lockId);
	void onLocklessRequestingLock(VexThreadState *state, const long &lockId);

	bool onSignallingOneThreadOnObject(const long &objectId, ThreadManager *stateManager, const long long &interruptionTime);
	bool onSignallingAllThreadsOnObject(const long &objectId, ThreadManager *stateManager, const long long &interruptionTime);

	bool interruptThread(VexThreadState *stateOfThreadToInterrupt, ThreadManager *stateManager, const long long &interruptionTime, bool isThreadInterruptCall);
	bool interruptThread(const long &interruptedThreadId, ThreadManager *stateManager, const long long &interruptionTime, bool isThreadInterruptCall);


	ThreadManagerRegistry *managers;
	EventLogger *eventLogger;
	ThreadRegistry *registry;

	ObjectRegistry *waitingOnObjectRegistry;		// a registry of threads waiting on an Object (used for wait - notify sync)
	LockRegistry *lockRegistry;					// a registry of threads requesting or holding locks (monitors)

	AggregateStateCounters *aggregateStateTransitionCounters;
	bool stackTraceMode;
	bool usingVtfScheduler;
	PapiProfiler *hpcProfiler;
	std::vector<std::string> excludedThreadsList;
};

#endif /* THREADEVENTSBEHAVIOUR_H_ */
