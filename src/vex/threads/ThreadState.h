/*
 * Objects of this class store the state of Java threads monitored by VTF
 */

#ifndef THREADSTATE_H_
#define THREADSTATE_H_

#include <pthread.h>
#include <unistd.h>

#include <vector>
#include <map>
#include <tr1/unordered_map>
#include <climits>
#include "Constants.h"
#include "DebugUtil.h"
#include "ObjectRegistry.h"
#include "MethodCallInfo.h"
#include "PerformanceMeasure.h"

#include "Timers.h"
#include "Scheduling.h"
#include "MethodLog.h"

class ModelHandler;
class CinqsNode;
class Customer;

#include "IoHandler.h"

#include "Statistics.h"
#include "NativeWaitingCriteria.h"

#include <sstream>
using namespace std;
using namespace tr1;

// Get thread id through system call
inline unsigned long gettid() {
	return (unsigned long) syscall(SYS_gettid);
}


struct vex_and_system_states {
	vex_and_system_states() {
		waitingThreads = 0;
		nwThreads = 0;
		activeInIoThreads = 0;
		blockedInIoThreads = 0;
		suspended = 0;
		running = 0;
		systemSleeping = 0;
		systemRunning = 0;
		systemD = 0;
		systemZombies = 0;
		systemStopped = 0;

		isWaitingRealCodeToCompleteAfterModelSimulation = 0;
		isAwakeningAfterJoin = 0;
		isAwakenInVex = 0;
		isRegistering= 0;
	}

	int waitingThreads;
	int nwThreads;
	int activeInIoThreads;
	int blockedInIoThreads;
	int suspended;
	int running;

	int isAwakeningAfterJoin;
	int isAwakenInVex;
	int isRegistering;
	int isWaitingRealCodeToCompleteAfterModelSimulation;

	int systemSleeping;
	int systemRunning;
	int systemD;
	int systemZombies;
	int systemStopped;

};

#define MAXIMUM_CPU_TIME_PROGRESS_WHILE_IN_NATIVE_WAITING 300000

class Visualizer;
class ThreadManagerRegistry;

class VexThreadState {

public:
	pthread_t pthread_id;
	VexThreadState();
	VexThreadState(const long &_threadId, char *_name);
	virtual ~VexThreadState();

	void init();


	// FINAL VERSION
	static void setDefaultThreadManager(ThreadManager *_defaultThreadManager) {
		defaultThreadManager = _defaultThreadManager;
		Scheduling::setDefaultThreadManager(_defaultThreadManager);
	}

	static void setThreadManagerRegistry(ThreadManagerRegistry *_registry) {
		threadManagerRegistry = _registry;
	}


	ThreadManagerRegistry *getAllThreadManagers() {
		return threadManagerRegistry;
	}

	void haltSuspendForAwhile();

	bool onVexMethodEntry(const int & methodId, const float &methodTimeScalingFactor);
	PerformanceMeasure *onVexMethodExit(const int &methodId);
	void onVexIoMethodEntry(const int &methodId, const int &invocationPointHashValue, const bool &possiblyBlocking);
	void onVexIoMethodExit();

	long long getTimeOnVexEntry();
	long long getRealTimeOnVexEntry();

	bool logMethodEntry(const int &methodId, const bool &shouldChangeVTFactorOnExit);
	PerformanceMeasure *logMethodExit();
	void onSynchronizeOnModelExit(const int &methodId);



	void onWrappedWaitingStart();
	void onWrappedWaitingEnd();

	void onVexEntry();

	void onVexExitWithRealTimeUpdate();

	bool ensureThreadIsNotInNativeWaitingStateWhenEnteringVex(const long long &startingTime);

	bool onReplacedTimedWaiting(const long &objectId, const long &timeout);
	bool onReplacedWaiting(const long &objectId, const bool &resumeShouldBeHandledInternally);
	void onBlockedWaitingInVex();

	void onWrappedTimedWaitingStart(const long &objectId, const long long &startingTime, const long &timeout);
	void onWrappedTimedWaitingEnd(ObjectRegistry *objectRegistry);
	void onInvocationPoint();
	void onYield();
	void onSignalledFromScheduler();

	bool setIoAndCheckIfOperationShouldBlockSimulationProgress(const bool &learning);



	//STATE
	// Return the current thread state unless performing model simulation
	static inline VexThreadState *getCurrentThreadState() {
		return currentThreadState;
	};

	static inline VexThreadState **getCurrentThreadStatePtr() {
		return &currentThreadState;
	};

	// Return the current thread if it exists
	static inline VexThreadState *forceGetCurrentThreadState() {
		return permanentThreadState;
	};

	static string *stateToString(VexThreadStates::Code state);
	static const char *getStateName(VexThreadStates::Code stateOfInterest);

	inline int getId() {
		return tid;
	};
	inline void setId(const int &_id) {
		tid = _id;
	};
	inline char *getName() {
		return name;
	};
	inline void setName(const char *_name) {
		name = new char[strlen(_name)+1];
		strcpy(name, _name);
	};
	inline VexThreadStates::Code getState() {
		 return currentState;
	};
	inline void setUniqueId(long _threadId) {
		jthreadId = _threadId;
	};

	const char *getCurrentStateName();
	const char *getPreviousStateName();

	inline long getUniqueId() {
		return (long)jthreadId;
	};
	string *stateToString(bool printCurrentState);

	inline bool const & getTimedOut() const {
		return timedOut;
	};
	inline void setTimedOut(const bool & _timedOut) {
		timedOut = _timedOut;
	};


	inline void setWaitingObjectId(const long & _timedWaitingObjectId) {
		if (_timedWaitingObjectId > 0) {
			timedWaitingObjectId = _timedWaitingObjectId;
		}
	};
	inline long const & getTimeout() const {
		return timers->getTimeout();
	};
	inline void setTimeout(const long & _timeout) {
		timers->setTimeout(_timeout);
	};

	inline void setTimeoutFlagToDenotePossiblyResumingThread() {
		timers->setTimeoutFlagToDenotePossiblyResumingThread();
	};
	bool hasTimeoutExpiredInRealTime(const long long &minimumMaybeAliveTime, const long long &remainingTimeToExpireTimeout) {
		return timers->hasTimeoutExpiredInRealTime(minimumMaybeAliveTime, remainingTimeToExpireTimeout);
	}

	inline bool isRegistering() {
		 return currentState == VexThreadStates::REGISTERING;
	};

	inline bool isSuspended() {
		 return currentState == VexThreadStates::SUSPENDED;
	};

	inline bool inIo() {
		return currentState == VexThreadStates::IN_IO || currentState == VexThreadStates::LEARNING_IO;
	};

	bool isDefinitelyActive() {
		 //																	The isRunning is necessary for multicore synchronization of VLFs
		return isDefinitelyActiveInIo() || isRegistering() || isSuspended() || isRunning() || wasAwaken() ||
			  isAwakeningAfterJoin() || isSomewhereInVex();// || (isNativeWaiting()); // && getWaitingInNativeVTFcode() != 0);
	}

	bool isSomewhereInVex();
	long long getCurrentState();

	// We need more checks to identify whether the threads are active and forbid a leap forward in virtual time
	bool maybeActive() {
//		return (isNativeWaiting() && scheduling->maximumPossibleNativeWaitingExpired()) || (currentState == WAITING && timers->isThreadStillBlockedInMonitor()) || (isActiveInModel() && getTimeout() == -1);

		return (currentState == VexThreadStates::WAITING && timers->isThreadStillBlockedInMonitor()) || (isActiveInModel() && getTimeout() == -1);
	}


	// VFL Rule 1 - XXXXXXXXXXXX
	bool isDefinitelyActiveInIo() {
		return isInAnyIo() && !ioHandler->executingPossiblyBlockingIo();
	}
	bool isInPossiblyBlockingIo() {
		return isInAnyIo() && ioHandler->executingPossiblyBlockingIo();
	}
	bool isInAnyIo() {
		return (inIoLearningPhase() || inIoPredictionPhase());
	}

	// VFL Rule 2 - XXXXXXXXXXXX
	bool isInTransitionFromOneStateToTheNextBeforeVexKnowsAboutIt() {
		return scheduling->isAwakeningAfterJoin() || isRegistering() || wasAwaken() || isWaitingRealCodeToCompleteAfterModelSimulation();
	}

	bool isInTransitionFromOneStateToTheNextBeforeVexKnowsAboutIt2() {
		return isRegistering() || wasAwaken() || isWaitingRealCodeToCompleteAfterModelSimulation() || isRunning() || (isNativeWaiting() && getWaitingInNativeVTFcode() != 0) || scheduling->isAwakeningAfterJoin();
	}
	bool isBlocked() {
		return currentState == VexThreadStates::WAITING || isNativeWaiting() || ((inIoLearningPhase() || inIoPredictionPhase()) && ioHandler->executingPossiblyBlockingIo());
	}
	bool blockedStateIsNotVerifiedBySystemState() {
		return isBlocked() && !isThreadSystemStateBlocked();
	}


	bool isActiveInIo() {
		if (inIoLearningPhase() || inIoPredictionPhase()) {
			if (ioHandler->executingPossiblyBlockingIo() && timers->maximumPossibleIoTimeExpired()) {
				return false;
			}
			return true;
		} else {
			return false;
		}
	}

	bool inIoLearningPhase() {
		return currentState == VexThreadStates::LEARNING_IO;
	};
	bool inIoPredictionPhase() {
		return currentState == VexThreadStates::IN_IO;
	};
	void rollbackFromTimeOutValue() {
		timers->rollbackFromTimeOutValue();
	};
	bool isTimedWaiting();
	bool isModelTimedWaiting();

	inline bool isNativeWaiting() {
		return currentState == VexThreadStates::NATIVE_WAITING;
	};


	inline bool isDead() {
		return currentState == VexThreadStates::VEX_ZOMBIE;
	};

	inline bool isRunning() {
		return currentState == VexThreadStates::RUNNING || currentState == VexThreadStates::IN_SYSTEM_CALL;
	};

	inline bool isWaitingRealCodeToCompleteAfterModelSimulation() {
		return currentState == VexThreadStates::AFTER_MODEL_SIM_WAITING_FOR_REAL_CODE;
	}
	inline bool isSimulatingModel() {
		return currentState == VexThreadStates::IN_MODEL || currentState == VexThreadStates::IN_MODEL_WAITING;
	};
	inline bool isActiveInModel() {
		return currentState == VexThreadStates::IN_MODEL;
	};
	inline bool isWaiting() {
		return currentState == VexThreadStates::WAITING;
	};

	inline void setRegistering() {
		timers->setLastTimePerStateERT(timers->getEstimatedRealTime());
//		setLastTimePerStateERT(getEstimatedRealTime());
		currentState = VexThreadStates::REGISTERING;
	};

	inline void setNewState(VexThreadStates::Code newState) {
		previousState = currentState;
		stats->logTransition(currentState, newState, (timers->getEstimatedRealTime() - timers->getLastTimePerStateERT()));
		timers->setLastTimePerStateERT(timers->getEstimatedRealTime());
//		setLastTimePerStateERT(getEstimatedRealTime());
		currentState = newState;
	}
	inline void setSuspended() {
		setNewState(VexThreadStates::SUSPENDED);
		scheduling->setAwaken(false);
	};
	inline void setWaiting() {
		timedOut = true;
		afterInterruptedTimedParking = false;
		setNewState(VexThreadStates::WAITING);
	};
	inline void setNativeWaiting() {
		setNewState(VexThreadStates::NATIVE_WAITING);
		scheduling->releaseThreadFromItsControllingManager();
	};
	inline void setTimedWaiting(const long & _timeout) {
		setNewState(VexThreadStates::WAITING);
		timedOut = true;
		afterInterruptedTimedParking = false;
		scheduling->setAwaken(false);
		// Update the thread's virtual time here, so that it is only resumed after that time the getEstimatedRealTime() is increased by timeout but
		// will eventually be overwritten by the value of current global virutal time at the time point when the thread is set to resume state->addElapsedTime(timeout * state->getTimeScalingFactor());
		timers->updateTimesBeforeWaitingWithTimeout(_timeout);

	};
	inline void setRunning() {
		setNewState(VexThreadStates::RUNNING);
	};
	inline void setLearningIo() {
		setNewState(VexThreadStates::LEARNING_IO);
	};
	inline void setInIo() {
		setNewState(VexThreadStates::IN_IO);
	};
	inline void setInSystemCall() {
		setNewState(VexThreadStates::IN_SYSTEM_CALL);
	}
	inline bool isInSystemCall() {
		return currentState == VexThreadStates::IN_SYSTEM_CALL;
	}
	inline void waitForRealCodeToComplete() {
		setNewState(VexThreadStates::AFTER_MODEL_SIM_WAITING_FOR_REAL_CODE);
		timers->setBlockingTimeToLastResumedTime();
	};
	inline void setInModelSimulation() {
		setNewState(VexThreadStates::IN_MODEL);
	};
	inline void setInModelWaiting() {
		setNewState(VexThreadStates::IN_MODEL_WAITING);
	};

	inline void setDead() {
		setNewState(VexThreadStates::VEX_ZOMBIE);
	};
	inline void setJoinWaitCompleted() {
		setNewState(VexThreadStates::AWAKING_FROM_WAITING);
	}
	inline void setInIoStale() {
		stats->logTransition(currentState, VexThreadStates::IN_IO_STALE);
	};
	inline void setInIoInvalidation() {
		stats->logTransition(currentState, VexThreadStates::IN_IO_INVALIDATION);
	};
	inline void setRecoveredAfterFailedIO() {
		stats->logTransition(currentState, VexThreadStates::RECOVERED_AFTER_FAILED_IO);
	};
	inline void setCustom1() {
		stats->logTransition(currentState, VexThreadStates::CUSTOM1);
	};
	inline void setCustom2() {
		stats->logTransition(currentState, VexThreadStates::CUSTOM2);
	};
	inline void setCustom3() {
		stats->logTransition(currentState, VexThreadStates::CUSTOM3);
	};

	inline void notifyParentWaitingForYouToJoin() {
		if (parentThreadWaitingYouToJoin != NULL && (*parentThreadWaitingYouToJoin) != NULL) {
			(*parentThreadWaitingYouToJoin) -> setAwakeningFromJoin(true);
		}
	};

	// Notify your parent when he's waiting for you to join - used to avoid leaps in time
	inline void setParentThreadWaitingYouToJoin(VexThreadState **waitingThread) {
		parentThreadWaitingYouToJoin = waitingThread;
	};





	bool getAndSetUnparkedState() {
		if (alreadyUnparked) {
			alreadyUnparked = false;
			parked = false;
			return true;
		} else {
			parked = true;
			timedWaitingObjectId = 0;
			return false;
		}
	}

	bool getAndSetParkedState() {
		if (parked) {
			alreadyUnparked = false;
			parked = false;
			return true;
		} else {
			alreadyUnparked = true;
			return false;
		}
	}

	const bool isNotAterInterruptedTimedParking() const {
		return !afterInterruptedTimedParking;
	}

	void setUnparked(const bool value) {
		alreadyUnparked = value;
	}



	//MODEL SIMULATION
	void setCurrentQueueingNode(CinqsNode *node);
	void setInModelSimulation(CinqsNode *source, Customer *customer);
	bool resumeSimulation();
	void initiateLocalResourceConsumption(const long long &totalTime);


	inline void startModelTimedWaiting(const long long &duration) {
		// Keep state IN_MODEL
		timers->updateTimesBeforeWaitingWithTimeout(duration);
	}

    inline void startSchedulerControlledModelSimulation() {
    	scheduling->startSchedulerControlledModelSimulation();
    	timers->logStartingVirtualTime();
    	ignoreThreadFromVEX();
    }
    inline void blockUntilModelSimulationEnd() {
    	scheduling->blockUntilModelSimulationEnd();
    }
    inline void notifyModelSimulationEnd() {
    	scheduling->notifyModelSimulationEnd();
    }

    inline void locklessNotifyModelSimulationEnd() {
    	scheduling->locklessNotifyModelSimulationEnd();
    }
    inline void endModelSimulation() {
    	timers->unsetBlockingTimeToLastResumedTime();
    	resetThreadIntoVEX();
    };
    bool resumeModelSimulation(long long &totalExecutionTime);
    // System state polling - viable for Linux

//    w S  --  Process Status
//        The status of the task which can be one of:
//           ’D’ = uninterruptible sleep
//           ’R’ = running
//           ’S’ = sleeping
//           ’T’ = traced or stopped
//           ’Z’ = zombie

    bool isThreadSystemStateBlocked() { // == blocked
    	char state = getSystemThreadState();
    	return (state == 'S') || (state == 'D') || (state == 'T');
    }

    bool isThreadSystemStateRunning() {	// == runnable
    	return getSystemThreadState() == 'R';
    }

	//TIME
	inline void addGlobalTime(const long long &timeAdded) {
		timers->addGlobalTime(timeAdded);
	}
	inline void clearTimeScalingFactor() {
		timers->setTimeScalingFactor(1.0);
	};
	inline void updateWaitingTimeFrom(const long long &startingWaiting) {
		timers->updateWaitingTimeFrom(startingWaiting);
	};

	inline long long const & getEstimatedRealTime() const {
		return timers->getEstimatedRealTime();
	};

	inline void setDontUpdateToGlobalTime(const bool &_dontUpdateToGlobalTime) {
		timers->setDontUpdateToGlobalTime(_dontUpdateToGlobalTime);
	};
	inline void updateCpuTimeAddingSimulatedVirtualTime() {
		timers->updateCpuTimeAddingSimulatedVirtualTime();
	};

	inline void setEstimatedRealTime(const long long & _estimatedRealTime) {
		timers->setEstimatedRealTime(_estimatedRealTime);
	};
	inline void setLastCPUTime(const long long & _lastCPUTime) {
		timers->setLastCPUTime(_lastCPUTime);
	};
	inline long long const getVirtualTime() const {
		return timers->getVirtualTime();
	};
	inline long long const & getLastCPUTime() const {
		return timers->getLastCPUTime();
	};

	inline void setVirtualTime(const long long & _virtualTime) {
		timers->setVirtualTime(_virtualTime);
	};

	inline float const & getTimeScalingFactor() const {
		return timers->getTimeScalingFactor();
	};
	inline long long const & getLastRealTime() const {
		return timers->getLastRealTime();
	};
	inline long long const & getResumedLastAt() const {
		return timers->getResumedLastAt();
	};

	inline long long const & getResumedLastAtReal() const {
		return timers->getResumedLastAtReal();
	};

	inline void setSynchronizationBarrierTime(const long long &schedulerTimeslot) {
		timers->setSynchronizationBarrierTime(schedulerTimeslot);
	};
	inline void clearSynchronizationBarrierTime(const long long &schedulerTimeslot) {
		timers->clearSynchronizationBarrierTime(schedulerTimeslot);
	};

	inline void setTimeScalingFactor(const float & _virtualTimeSpeedup) {
		timers->setTimeScalingFactor(_virtualTimeSpeedup);
	};

	inline void setResumedLastAt(const long long & _resumedLastAt) {
		timers->setResumedLastAt(_resumedLastAt);
	};

	inline long long const & getLocalTimeSinceLastResume() const {
		return timers->getLocalTimeSinceLastResume();
	};
	inline void setLastRealTime(const long long & _lastRealTime) {
		timers->setLastRealTime(_lastRealTime);
	};

	inline long long const & getLastTimeInHandler() const {
		return timers->getLastTimeInHandler();
	};

	inline long long getThreadTimeBeforeMethodInstrumentation() {
		return timers->getThreadTimeBeforeMethodInstrumentation();
	};
	inline long long getThreadTimeBeforeIoMethodInstrumentation() {
		return timers->getThreadTimeBeforeIoMethodInstrumentation();
	};

	inline void updateCpuTimeClock() {
		timers->updateCpuTimeClock();
	};

	inline bool isSuspendingAllowed() {
		return scheduling->isSuspendingAllowed();
	}
	inline void onVexExit() {
		inVex = false;
		scheduling->exitingVex();
		timers->updateCpuTimeClock();
	};

	void onVexExitWithoutTimeUpdate();
	void onVexExitWithCpuTimeUpdate();
	void onVexExitWithBothClocksUpdate();

	void updateRealTimeClock() {
			return timers->updateRealTimeClock();
	}
	long long updateClocks() {
		return timers->updateClocks();
	}

	long long addElapsedTime(long long time) {
		return timers->addElapsedTime(time);
	}


	inline void setCpuTimeClockAfterIo() {
		inVex = false;
		scheduling->exitingVex();
		timers->setCpuTimeClockAfterIo();
	};
	long long leapForwardBy(long long time) {
		return timers->leapForwardBy(time);
	}

	long long leapTo(long long time) {
		return timers->leapTo(time);
	}

	inline void leapForwardTo(const long long &currentTime) {
		timers->leapForwardTo(currentTime);
	}

	long long getTimeDifferenceUntil(const long long &startingTime) {
		return timers->getTimeDifferenceUntil(startingTime);
	}

	long long getAndResetLocalTime() {
		long long a = timers->getAndResetLocalTime();
//		if (a > 100000000) {
//			cout << "thread " << getName() << " added local time " << a << endl;
//		}
		return a;
	}

	long long getLocalTime() {
		return timers->getLocalTime();
	}

	void updateThreadLocalTimeSinceLastResumeTo(const long long &presetTime) {
		timers->updateThreadLocalTimeSinceLastResumeTo(presetTime);
	}
	void updateThreadLocalTimeSinceLastResumeToRealTime(const long long &presetTime) {
		timers->updateThreadLocalTimeSinceLastResumeToRealTime(presetTime);
	}
	inline void leapToGlobalTime(const long long &currentVirtualTime) {
		timers->leapToGlobalTime(currentVirtualTime);
	};

	inline bool beforeCurrentGlobalTime(const long long &currentVirtualTime) {
		return timers->beforeCurrentGlobalTime(currentVirtualTime);
	};

	inline void setLocalTime(const long long &_localtime) {
		timers->setLocalTime(_localtime);
	}

	inline Timers *getThreadTimers() {
		return timers;
	};

	inline void updateCurrentLocalTime() {
		return timers->updateCurrentLocalTime();
	};
	void addLocalTime(const long long &_timediff) {
		timers->addLocalTime(_timediff);
	}
	void addLocalIoTime(const long long &_timediff) {
		timers->addLocalIoTime(_timediff);
	}


//protected:
//	inline long long const & getMonitorWaitingTime() const {
//		return timers->getMonitorWaitingTime();
//	};
//	inline long long const & getIoWaitingTime() const {
//		return timers->getIoWaitingTime();
//	};
//
//	inline bool shouldNotUpdateToGlobalTime() {
//		return timers->shouldNotUpdateToGlobalTime();
//	};
//
//
//	inline void synchronizeWithLocalTimelineOf(const int &managerId) {
//		timers->synchronizeWithLocalTimelineOf(managerId);
//	}
//
//	inline void setLocalTimeSinceLastResume(const long long & _localTimeSinceLastResume) {
//		timers->setLocalTimeSinceLastResume(_localTimeSinceLastResume);
//	};
//
//	inline void setResumedLastAtReal(const long long & _resumedLastAtReal) {
//		timers->setResumedLastAtReal(_resumedLastAtReal);
//	};
//	inline void updateResumedLastRealAt() {
//		timers->updateResumedLastRealAt();
//	};
//
//	inline long long const & getLastTimePerStateERT() const {
//		return timers->getLastTimePerStateERT();
//	};
//	inline void setLastTimePerStateERT(const long long & _lastTimePerStateERT) {
//		timers->setLastTimePerStateERT(_lastTimePerStateERT);
//	};
//
//	inline void setLastTimeInHandler(const long long & _lastTimeInHandler) {
//		timers->setLastTimeInHandler(_lastTimeInHandler);
//	};
//	inline long long const & getLastRealTimeInHandler() const {
//		return timers->getLastRealTimeInHandler();
//	};
//	inline void setLastRealTimeInHandler(const long long & _lastRealTimeInHandler) {
//		timers->setLastRealTimeInHandler(_lastRealTimeInHandler);
//	};
//
//	void updateLastResumedTo(const long long &realHandlerTime) {
//		timers->updateLastResumedTo(realHandlerTime);
//	};
//
//	inline bool isInVex() {
//		return scheduling->isInVex();
//	}
//
//	inline void doNotSuspend() {
//		scheduling->doNotSuspend();
//	}
//
//
//	inline void allowSuspend() {
//		scheduling->allowSuspend();
//	}
//
//	inline bool aheadOfCurrentGlobalTime(const long long &currentVirtualTime) {
//		return timers->aheadOfCurrentGlobalTime(currentVirtualTime);
//	};



	//I/O
public:
	bool invalidIoInvocationPoint() {
		return ioHandler->invalidIoInvocationPoint();
	}

	void resetIoStateFlags() {
		ioHandler->resetIoStateFlags();
	}

	bool invalidIoInvocationPoint(const int &invocationPointHashValue) {
		return ioHandler->invalidIoInvocationPoint(invocationPointHashValue);
	}
	bool isIgnoringIo() {
		return ioHandler->isIgnoringIo();
	}

	void doNotRegardAsIo() {
		ioHandler->ignoreNextIo();
	}
	void recognizedCallAsCached() {
		ioHandler->recognizedCallAsCached();
	}


	bool callRecognizedAsCached() {
		return ioHandler->callRecognizedAsCached();
	}

	PredictionMethod *getLastIoPredictionMethod() {
		return ioHandler->getLastIoPredictionMethod();
	}

	unsigned int getStackDepth() {
		return ioHandler->getStackDepth();
	}

	void addPredictionToEstimatedRealTime() {
		timers->addElapsedTime(ioHandler->getLastIoPrediction());
		ioHandler->addPredictionToEstimatedRealTime();
	}
	bool isIoPredictionStillValid(const long long &currentRealTime);
	void setIoInvocationPointHashValue(const int &_ioInvocationPointHashValue) {
		ioHandler->setIoInvocationPointHashValue(_ioInvocationPointHashValue);
	}

	long long const & getLastIoPrediction() const {
		return ioHandler->getLastIoPrediction();
	}

	void setLastIoPrediction(const long long & _lastIoPrediction) {
		ioHandler->setLastIoPrediction(_lastIoPrediction);
	}

	int getIoUniqueInvocationPoint() {
		return ioHandler->getIoInvocationPointHashValue() * ioHandler->getStackTraceHash();
	}
	void setIoPredictionInfo(PredictionMethod *invocationTraceInfo, const long long &lastIoPrediction) {
		ioHandler->setIoPredictionInfo(invocationTraceInfo, lastIoPrediction);
	}


	void setStackTraceHash(const int &_stackTraceHash) {
		ioHandler->setStackTraceHash(_stackTraceHash);
	}
	int getIoInvocationPointHashValue() {
		return ioHandler->getIoInvocationPointHashValue();
	}

	int getStackTraceHash() {
		return ioHandler->getStackTraceHash();
	}
	bool const & getIoFinishedBeforeLogging() const {
		return ioHandler->getIoFinishedBeforeLogging();
	}

	long long getIoCPUTime() {
		return ioHandler->getIoCPUTime();
	}
	void updateIoCPUTimeTo(const long long &startingTime) {
		ioHandler->setIoCPUTime(startingTime - getLastCPUTime());
	}
	void setIoFinishedBeforeLogging(const bool & _ioFinishedBeforeLogging) {
		ioHandler->setIoFinishedBeforeLogging(_ioFinishedBeforeLogging);
	}
	void invalidateIoPrediction();
	//METHODS
	MethodCallInfo *getCurrentMethodInfo() {
		return methodLog->getCurrentMethodInfo();
	}

	void updateEstimatedRealTimeAfterIoPrediction(const long long &actualIoDuration);

	void compensateForAdditionalIoWaitingTime(const long long &estimatedRealTimeBeforeSuspend);

	void setCurrentMethodInfo(MethodCallInfo *_newMethod) {
		methodLog->setCurrentMethodInfo(_newMethod);
	}
	MethodCallInfo *getExitingMethodInfo() {
		return methodLog->getExitingMethodInfo();
	}

	void updateExitingMethodInfo(const int &methodId) {
		timers->updateExitingMethodInfo(methodId, methodLog->getExitingMethodInfo());
	}
	int getCurrentMethodId() {
		return methodLog->getCurrentMethodId();
	}

	void decreaseNextMethodInfoToUse() {
		methodLog->decreaseNextMethodInfoToUse();
	}

	void pushMethodEntryEventIntoThreadMethodStack(MethodCallInfo *callinfo) {
		methodLog->pushMethodEntryEventIntoThreadMethodStack(callinfo);
	}
	stack<MethodCallInfo*>* getThreadMethodStack() {
		return methodLog->getThreadMethodStack();
	}

	virtual MethodCallInfo *getNextMethodCallInfo(const int &methodId);
	virtual PerformanceMeasure *getExitingMethodPerformanceMeasure();
	virtual unordered_map<int, PerformanceMeasure*> *getMethodMeasures() {
		return measures;
	}
	virtual PerformanceMeasure *getCurrentStackTrace() {
		return NULL;
	}

	void setInvocationPoints(const unsigned long &points) {
		stats->setInvocationPoints(points);
	}
	void addInvocationPoints() {
		stats->addInvocationPoints();
		scheduling->increaseVtfInvocationsSinceLastResume();
	}

	static void setVisualizer(Visualizer *vis) {
		visualizer = vis;
	}


protected:
	ThreadManager *getPreviouslyControllingManager() const {
		ThreadManager *previouslyControllingManager = scheduling->getPreviouslyControllingManager();
		if (previouslyControllingManager == NULL) {
			return defaultThreadManager;
		} else {
			return previouslyControllingManager;
		}
	}

	long long getPredictionError(const long long &currentRealTime);
//	IoHandler *getIoHandler() {
//		return ioHandler;
//	}
//
//	void resetTimeCounters();
//
//	int const & getTotalThreadsInIO() const {
//		return ioHandler->getTotalThreadsInIO();
//	}
//	virtual int getIoHashValue();
	static Visualizer *visualizer;
//	void setExitingMethodInfo(MethodCallInfo *_newMethod) {
//		methodLog->setExitingMethodInfo(_newMethod);
//	}

public:
	//LOGGING
	inline void clearTransitionCounters() {
		stats->clearTransitionCounters();
	}

	unsigned long getInvocationPoints() {
		return stats->getInvocationPoints();
	}

	long long *getTotalTimePerState() {
		return stats->getTotalTimePerState();
	}
	StateTransitionMatrix *getStateTransitionsMatrix() {
		return stats->getStateTransitionsMatrix();
	}


	inline void setLastVexMethodInvoked(const char *_lastMethod) {
		stats->setLastVexMethodInvoked(_lastMethod);
	};

	//SCHEDUING
	inline void lockShareResourceAccessKey() {
//		stringstream str;
//		str << getName() << " " << getId() << " LOCKING" << endl;
//		vtfstacktrace(true, stderr, str.str().c_str());

//		assert(getState() != VexThreadStates::SUSPENDED || getId() != gettid());	// if a thread is suspended before acquiring a lock then
		scheduling->lockShareResourceAccessKey();
	}

	inline void unlockShareResourceAccessKey() {
		scheduling->unlockShareResourceAccessKey();
//		stringstream str;
//		str << getName() << " " << getId() << " UN-LOCKING" << endl;
//		vtfstacktrace(true, stderr, str.str().c_str());
	}

	void beforeThreadInterruptAt(ObjectRegistry *objectRegistry);
	void waitForThreadToBlock() {
		scheduling->waitForThreadToBlock();
	}
	void signalBlockedThreadToResume() {
		scheduling->signalBlockedThreadToResume();
	}
	void allowSignalledThreadToResume() {
		scheduling->allowSignalledThreadToResume();
	}
	void addVtfInvocationPoint() {
		scheduling->addVtfInvocationPoint();
	}
	inline void resetThreadIntoVEX() {
		currentThreadState = permanentThreadState;
	};
	bool const & wasAwaken() const {
		return scheduling->wasAwaken();
	}
	int const & getWaitingInNativeVTFcode() {
		return scheduling->getWaitingInNativeVTFcode();
	}

	inline void ignoreThreadFromVEX() {
		currentThreadState = NULL;
	};


	void setAwaken(const bool & _awaken) {
		scheduling->setAwaken(_awaken);
	}


	bool getAndResetForcedSuspendFlag() {
		return scheduling->getAndResetForcedSuspendFlag();
	}

	bool isThreadSetToBeForcefullySuspended() {
		return scheduling->isThreadSetToBeForcefullySuspended();
	}

	// Called when methods are resumed by a manager to synchronize on the global timeline
	inline void onThreadResumeByManager(unsigned int &controllingManagerId) {
		scheduling->onThreadResumeByManager(controllingManagerId);
	};

	bool isThreadBlockedInNativeWait(const long long &realTime);

    ThreadManager *getThreadCurrentlyControllingManager() const {
        return scheduling->getThreadCurrentlyControllingManager();
    }
    ThreadManager *getDefaultThreadManager() const {
    	return defaultThreadManager;
    }
    ThreadManager **getThreadCurrentlyControllingManagerPtr() const {
        return scheduling->getThreadCurrentlyControllingManagerPtr();
    }
    void setThreadCurrentlyControllingManager(ThreadManager *threadCurrentlyControllingManager) {
        scheduling->setThreadCurrentlyControllingManager(threadCurrentlyControllingManager);
    }

	void acquireThreadControllingLock() {
		scheduling->acquireThreadControllingLock();
	}
	short getSuspendingThreadIntention() {
		return scheduling->getSuspendingThreadIntention();
	}
	void blockHereUntilSignaled() {
		scheduling->blockHereUntilSignaled();
	}

	void notifySchedulerThatThreadResumed() {
		scheduling->notifySchedulerThatThreadResumed();
	}
	void waitForResumingThread() {
		scheduling->waitForResumingThread();
	}
	void notifySchedulerForIntention(const short& intention) {
		scheduling->notifySchedulerForIntention(intention);
	}

	inline void setAwakeningFromJoin(const bool &value) {
		scheduling->setAwakeningFromJoin(value);
	};
//	inline bool getIsAwakeningAfterJoin() {
//		return scheduling->getIsAwakeningAfterJoin();
//	}
	// Each time subtract one from the counter denoting that you are waiting on a thread to join
//	inline bool getAndConsumeIsAwakeningAfterJoinCounter() {
//		return scheduling->getAndConsumeIsAwakeningAfterJoinCounter();
//	};

	inline const bool & isAwakeningAfterJoin() const {
		return scheduling->isAwakeningAfterJoin();
	};

	inline void flagThreadBeingCurrentlyPolledForNativeWaiting() {
		scheduling->flagThreadBeingCurrentlyPolledForNativeWaiting();
	}

	inline void clearThreadBeingCurrentlyPolledForNativeWaiting() {
		scheduling->clearThreadBeingCurrentlyPolledForNativeWaiting();
	}

	inline const bool &isThreadBeingCurrentlyPolledForNativeWaiting() const {
		return scheduling->isThreadBeingCurrentlyPolledForNativeWaiting();
	}

	inline bool isWaitingInRealTime() {
		return waitingInRealTime;
	}

	void forceThreadSuspend() {
		scheduling->forceThreadSuspend();
	}
protected:
	void eraseWaitingObjectIdFromRegistry(ObjectRegistry *objectRegistry);
//	inline bool doicurrentlyOwnLock() {
//		return scheduling->getSharedResourceAccessKeyCurrentOwner() == (int)tid;
//	}
//	void beforeThreadInterrupt(ObjectRegistry *objectRegistry);
//
//	long long const & getVtfInvocationsSinceLastResume() const {
//		return scheduling->getVtfInvocationsSinceLastResume();
//	}
//	void increaseConsecutiveTimeslots() {
//		scheduling->increaseConsecutiveTimeslots();
//	}
//	void setVtfInvocationsSinceLastResume(const long long & _vtfInvocationsSinceLastResume) {
//		scheduling->setVtfInvocationsSinceLastResume(_vtfInvocationsSinceLastResume);
//	}
//	void setWaitingInNativeVTFcode(const int &_value) {
//		scheduling->setWaitingInNativeVTFcode(_value);
//	}

	bool isThreadBlockedInNativeWait();
	bool isThreadStillBlockedInNativeWait(const long long &cpuTimeAtBeginningOfSignalHandler);
//
//
////	inline void onThreadResumeByManagerOnLocalTimelineOf(unsigned int &controllingManagerId) {
////		scheduling->onThreadResumeByManager(controllingManagerId);
////		timers->updateTimesAfterResumingOnLocalTimelineOf(controllingManagerId);
////	};
//
//
//	// Called when methods are resumed by a manager to synchronize on the local timeline
//	inline void onThreadResumeByLocalManager(unsigned int &controllingManagerId) {
//		scheduling->onThreadResumeByManager(controllingManagerId);
//		timers->updateTimesAfterResumingOnLocalTimelineOf(controllingManagerId);
//	};
//	void setControllingManagerId(const int &_id) {
//		scheduling->setControllingManagerId(_id);
//	}
//	unsigned int getControllingManagerId() {
//		return scheduling->getControllingManagerId();
//	}
//
//	void releaseThreadFromItsControllingManager() {
//    	scheduling->releaseThreadFromItsControllingManager();
//    }



//	inline void setPerformingInstrumentation(const bool &value) {
////		performingInstrumentation = value;
//	};
//
//	inline bool isPerformingInstrumentation() {
//		return true;//performingInstrumentation;
//	};
//
//
//	inline void setExplicitWaiting() {
//		waitingInRealTime = true;
//	};
//	inline void unsetExplicitWaiting() {
//		waitingInRealTime = false;
//	};

public:

	//DISTRIBUTED-CLIENT

	inline VexThreadState * getVtfServerStatePtr() {
		return vtfServerStatePtr;
	};
	inline void setVtfServerStatePtr(VexThreadState * _vtfServerStatePtr) {
		vtfServerStatePtr = _vtfServerStatePtr;
	};

	inline VexThreadState * getVtfClientStatePtr() {
		return vtfClientStatePtr;
	};

	inline bool isManagedLocally() {
		return (managingSchedulerFd == 0);
	};

	inline void setVtfClientStatePtr(VexThreadState * _vtfClientStatePtr) {
		vtfClientStatePtr = _vtfClientStatePtr;
	};

	inline unsigned int const & getManagingSchedulerFd() const {
		return managingSchedulerFd;
	};
	inline void setManagingSchedulerFd(const unsigned int & _managingSchedulerFd) {
		managingSchedulerFd = _managingSchedulerFd;
	};

	inline unsigned int const & getThreadResponseFd() const {
		return threadResponseFd;
	};
	inline void setThreadResponseFd(const unsigned int & _threadResponseFd) {
		threadResponseFd = _threadResponseFd;
	};

protected:




	//OPERATORS
//	VexThreadState &operator=(const VexThreadState &rhs);
	bool operator==(const VexThreadState &rhs) const;
	bool operator<(const VexThreadState &rhs) const;
	friend std::ostream & operator <<(std::ostream &outs, const VexThreadState &state) {
		outs << state.name << "(" << state.tid << ") <<-," << (&state) << ", VT=" << state.getVirtualTime()/1000000 << ", ERT=" << state.getEstimatedRealTime()/1000000 << ", CPUt=" << state.getLastCPUTime()/1000000 << " state: " << state.currentState;

		if (state.currentState == VexThreadStates::WAITING || state.currentState == VexThreadStates::IN_MODEL) {
			if (state.getTimeout() <= 0) {
				outs << " <<" << state.getTimeout() << ">>";
			} else {
				outs << " <<";
				Time::concisePrinting(outs, state.getTimeout(), false);
				outs << ">>";
			}

		} else if (state.currentState == VexThreadStates::RUNNING) {
			outs << ", LRA=" << state.getResumedLastAt()/1000000 << " lt=" << state.getLocalTimeSinceLastResume()/1000000 << " [@"<< state.getThreadCurrentlyControllingManager() << "]";
		}
		return outs;
	};


public:
	//EVENTS
	inline void onEntry(const int &methodId) {
		ioHandler->enteringMethod(methodId);
		scheduling->increaseVtfInvocationsSinceLastResume();

	};
	inline void onExit(const int &methodId) {
		ioHandler->exitingMethod(methodId);
		scheduling->increaseVtfInvocationsSinceLastResume();
	};

	inline void onIoEntry(const bool &possiblyBlocking) {
		// TODO maybe should be cleaned up by timeout-ing I/O themselves only when they actually happen
		setTimeout(-1);
//		timedOut = false;
		ioHandler->notifyAboutPossiblyBlockingIo(possiblyBlocking);
		scheduling->increaseVtfInvocationsSinceLastResume();
	};

	ThreadManager *getCurrentSchedulerOnVexEntry(const long long &startTime);
	ThreadManager *getCurrentSchedulerOnVexEntryWithGuaranteedSuspend(const long long &startTime);


	void toggleShowMethodEntries() {
		showMethodEntries ^= 1;
	}


	const bool & shouldShowMethodEntries() const {
		return showMethodEntries;
	}

	char getSystemThreadState();
	ThreadManager *getCurrentlyControllingManagerOf();

protected:
	unsigned long tid;
	bool showMethodEntries;

	ThreadManager *getCurrentSchedulerOnVexEntry();

	//STATE
	static __thread VexThreadState *currentThreadState;	// local thread state of each thread
	static __thread VexThreadState *permanentThreadState;

	char *name;
	long jthreadId;
	VexThreadState **parentThreadWaitingYouToJoin;

	VexThreadStates::Code currentState;
	VexThreadStates::Code previousState;	// used for debugging reasons

	long timedWaitingObjectId;
	bool timedOut;

	bool waitingInRealTime;
	bool alreadyUnparked;
	bool parked;
	bool afterInterruptedTimedParking;

	//TIME
	Timers *timers;

	//I/O
	IoHandler *ioHandler;

	//MODELS
	ModelHandler *modelHandler;

	//METHODS
	MethodLog *methodLog;

	//SCHEDULING
	Scheduling *scheduling;

	//LOGGING
	Statistics *stats;

	//Behaviour: native waiting
	NativeWaitingCriteria *nativeWaitingCriteria;
	StackTraceBasedCriteria stackTraceBasedCriteria;	// used always to check if a thread is still native waiting

	// DISTRIBUTED
	unsigned int managingSchedulerFd;// used for the distributed version
	unsigned int threadResponseFd;
	VexThreadState *vtfServerStatePtr;
	VexThreadState *vtfClientStatePtr;

	virtual void clearStackTraceInfo();
	unsigned int localManagerId;		// the id of the manager that controls this thread in the local id array
	ThreadManager *threadCurrentlyControllingManager;

//	bool forceSuspend;
	bool inVex;
	bool awakeningFromJoin;				// used for coordination with a joining thread, because a thread exits VEX before really dying (and allowing blocked threads waiting to join on it to resume)
//	bool suspendingAllowed;
//	bool awaken;
//	bool simulatingModel;
//	bool currentlyPolledForWhetherItIsNativeWaiting; 	// flag so that only one scheduler polls you at each time
//
//	unsigned int consecutiveTimeslots;
//	int consecutiveTimesFoundNativeWaiting;


	short suspendFlag;
	pthread_mutex_t suspendFlagLock;
	pthread_cond_t condSuspendFlagLock;

	long long vtfInvocationsSinceLastResume;	//NAT_WAIT
	int waitingInNativeVTFcode;				// used to avoid taking time while in VEX code

	pthread_mutex_t suspendLock;
	pthread_mutex_t sharedAccessKey;
	pthread_cond_t condSuspendLock;

	// STATIC VARS
	static ThreadManager *defaultThreadManager;
	static ThreadManagerRegistry *threadManagerRegistry;	// used for operation that should affect all managers

private:
	unordered_map<int, PerformanceMeasure*>* getThreadPerformanceMeasures();
	unordered_map<int, PerformanceMeasure*> *measures;
};



/***
 * Subclass of ThreadState used for storage of stack traces
 **/
class StackTraceThreadState : public VexThreadState {

public:
	StackTraceThreadState() : VexThreadState() {
		rootStackTraceInfos = NULL;
		currentStackTraceInfo = NULL;
	}
	StackTraceThreadState(const long &_threadId, char *_name) : VexThreadState(_threadId, _name) {
		rootStackTraceInfos = NULL;
		currentStackTraceInfo = NULL;
	}


	int getIoHashValue();
	~StackTraceThreadState();

	PerformanceMeasure *getExitingMethodPerformanceMeasure();
	StackTraceInfo *getNextMethodCallInfo(const int &methodId);

	vector<PerformanceMeasure *> *getRootStackTraceInfos() {
		return rootStackTraceInfos;
	}

	PerformanceMeasure *getCurrentStackTrace() {
		return currentStackTraceInfo;
	}

protected:
	void clearStackTraceInfo();

private:
	PerformanceMeasure *getCurrentStackTraceInfo(const int &methodId);
	vector<PerformanceMeasure *> *rootStackTraceInfos;	// the root PerformanceMeasures
	PerformanceMeasure *currentStackTraceInfo;			// using the PerformanceMeasure->submethods
														// we create the thread execution call tree

	bool methodAlreadyCalledInStackTrace(PerformanceMeasure *traceInfo, const int &methodId);

};




struct threadStatePtr_compare : binary_function<VexThreadState *, VexThreadState *, bool> {
	bool operator() (VexThreadState *lhs, VexThreadState *rhs) const {
		if (lhs->getEstimatedRealTime() > rhs->getEstimatedRealTime()) {
			return true;
		} else {
			return false;
		}
	}
};

struct threadStatePtr_compare_ascending : binary_function<VexThreadState *, VexThreadState *, bool> {
	bool operator() (VexThreadState *lhs, VexThreadState *rhs) const {
		if (lhs->getEstimatedRealTime() < rhs->getEstimatedRealTime()) {
			return true;
		} else {
			return false;
		}
	}
};

#endif /*THREADSTATE_H_*/
