/*
 * Scheduling.h
 *
 *  Created on: 21 Apr 2011
 *      Author: root
 */

#ifndef SCHEDULING_H_
#define SCHEDULING_H_

#include <pthread.h>
#include "DebugUtil.h"
// Flags for synchronization withe the scheduler
enum {
	CLEAR_SUSPENDFLAG 	= 0,
	TO_BE_SUSPENDED		= 1,
	TO_KEEP_ON_RUNNING	= 2,
	TO_BE_DISREGARDED	= 3,
	RESUMING_SUSPENDFLAG = 4
};

#define MAXIMUM_CONSECUTIVE_TIMESLICES_AS_NATIVE_WAITING 5

class ThreadManager;

class Scheduling {
public:
	Scheduling();
	virtual ~Scheduling();


	inline void lockShareResourceAccessKey() {
		++waitingInNativeVTFcode;
		pthread_mutex_lock(&sharedAccessKey);
	}

	inline void unlockShareResourceAccessKey() {
		pthread_mutex_unlock(&sharedAccessKey);
	}

	bool isSuspendingAllowed();
//	inline bool isSuspendingAllowed() {
//		return doNotSuspendUntil < Time::getRealTime();
//	}

	inline void flagThreadBeingCurrentlyPolledForNativeWaiting() {
		currentlyPolledForWhetherItIsNativeWaiting = true;
	}

	inline void clearThreadBeingCurrentlyPolledForNativeWaiting() {
		currentlyPolledForWhetherItIsNativeWaiting = false;
	}

	inline const bool &isThreadBeingCurrentlyPolledForNativeWaiting() const {
		return currentlyPolledForWhetherItIsNativeWaiting;
	}

	inline void exitingVex() {
		inVex = false;
	};

	inline void waitForThreadToBlock() {
		if (!awaken) {
			pthread_mutex_lock(&suspendLock);
		}
	};
	inline void signalBlockedThreadToResume() {
		pthread_cond_signal(&condSuspendLock);
	};
	inline void allowSignalledThreadToResume() {
		pthread_mutex_unlock(&suspendLock);
	};

	inline void addVtfInvocationPoint() {
		++vtfInvocationsSinceLastResume;
	};
	inline bool const & wasAwaken() const {
		return awaken;
	};
	inline void setAwaken(const bool & _awaken) {
		awaken = _awaken;
	};

	inline void onThreadResumeByManager(unsigned int &controllingManagerId) {
		awaken 			= true;
		vtfInvocationsSinceLastResume = 0;
		localManagerId = controllingManagerId;
		consecutiveTimeslots = 0;
	};

	void notifySchedulerForIntention(const short& intention);
	short getSuspendingThreadIntention();
	void blockHereUntilSignaled();
	void notifySchedulerThatThreadResumed();

	void haltSuspendForAwhile();

	void acquireThreadControllingLock();
	void releaseThreadControllingLock();

	void waitForResumingThread();
	inline void setAwakeningFromJoin(const bool &value) {
		awakeningFromJoin = value;
//		if (value) {
//			++awakeningFromJoin;// = value;
//		} else {
//			// Can get below zero as all "waiting-end" calls set it to false, while it might not have been set
//			if (awakeningFromJoin > 0) {
//				--awakeningFromJoin;
//			}
//		}
	};

	inline void increaseVtfInvocationsSinceLastResume() {
		++vtfInvocationsSinceLastResume;
	}

	inline const bool &isAwakeningAfterJoin() const {
		return awakeningFromJoin;
	}

    ThreadManager *getThreadCurrentlyControllingManager() const {
        return threadCurrentlyControllingManager;
    }

    ThreadManager *getPreviouslyControllingManager() const {
        return previouslyControllingManager;
    }

    ThreadManager **getThreadCurrentlyControllingManagerPtr() {
			if (threadCurrentlyControllingManager != NULL) {
				return &threadCurrentlyControllingManager;
			} else {
				return NULL;
			}
    }

    void setThreadCurrentlyControllingManager(ThreadManager *threadCurrentlyControllingManager) {
        this->threadCurrentlyControllingManager = threadCurrentlyControllingManager;
    }

    void releaseThreadFromItsControllingManager() {
    	waitingInNativeVTFcode = 0;
    	previouslyControllingManager = threadCurrentlyControllingManager;
    	threadCurrentlyControllingManager = NULL;
    	consecutiveTimesFoundNativeWaiting = MAXIMUM_CONSECUTIVE_TIMESLICES_AS_NATIVE_WAITING;
    }

    void startSchedulerControlledModelSimulation();
    void blockUntilModelSimulationEnd();
    void notifyModelSimulationEnd();
    void locklessNotifyModelSimulationEnd();

	static void setDefaultThreadManager(ThreadManager *_defaultThreadManager) {
		defaultThreadManager = _defaultThreadManager;
	}

	bool getAndResetForcedSuspendFlag() {
		bool temp = forceSuspend;
		forceSuspend = false;
		return temp;
	};

	bool const &isThreadSetToBeForcefullySuspended() const {
		return forceSuspend;
	}

	int const & getWaitingInNativeVTFcode() {
		return waitingInNativeVTFcode;
	}

	inline bool isInVex() {
		return inVex;
	}
	inline void setVtfInvocationsSinceLastResume(const long long & _vtfInvocationsSinceLastResume) {
		vtfInvocationsSinceLastResume = _vtfInvocationsSinceLastResume;
	};

	void setWaitingInNativeVTFcode(const int &_value) {
		waitingInNativeVTFcode = _value;
	}
	inline long long const & getVtfInvocationsSinceLastResume() const {
		return vtfInvocationsSinceLastResume;
	};

	void forceThreadSuspend() {
		forceSuspend = true;
	};
protected:

//	inline bool maximumPossibleNativeWaitingExpired() {
//		return (consecutiveTimesFoundNativeWaiting-- > 0);
//	}
//

//
//	bool isThreadBlockedInNativeWait();
//	bool isThreadBlockedInNativeWait(const long long &realTime);
//	inline unsigned int getConsecutiveTimeslots() const {
//		return consecutiveTimeslots;
//	}
//	inline void setControllingManagerId(const int &_id) {
//		localManagerId = _id;
//	};
//	inline unsigned int getControllingManagerId() {
//		return localManagerId;
//	};
//
//
//	//	inline bool getIsAwakeningAfterJoin() {
//	//		return awakeningFromJoin;
//	//	}
//	//
//	//	// Each time subtract one from the counter denoting that you are waiting on a thread to join
//	//	inline bool getAndConsumeIsAwakeningAfterJoinCounter() {
//	//		return awakeningFromJoin;
//	////
//	////		if (awakeningFromJoin>0) {
//	////			--awakeningFromJoin;
//	////			return true;
//	////		}
//	////		return false;
//	//	};
//
//	void onEnteringVex();
//
//	inline int getSharedResourceAccessKeyCurrentOwner() {
//		return sharedAccessKey.__data.__owner;
//	}
//
//	inline void enteringVex() {
//		inVex = true;
//	};
//
//	inline void doNotSuspend() {
//		suspendingAllowed = false;
//	}
//
//	inline void allowSuspend() {
//		suspendingAllowed = true;
//	}
//
//	void increaseConsecutiveTimeslots() {
//		++consecutiveTimeslots;
//	};
















	unsigned int localManagerId;		// the id of the manager that controls this thread in the local id array
	ThreadManager *threadCurrentlyControllingManager;
	ThreadManager *previouslyControllingManager;
	bool forceSuspend;
	bool inVex;
	bool awakeningFromJoin;				// used for coordination with a joining thread, because a thread exits VEX before really dying (and allowing blocked threads waiting to join on it to resume)

	short suspendFlag;
	pthread_mutex_t suspendFlagLock;
	pthread_cond_t condSuspendFlagLock;

	long long vtfInvocationsSinceLastResume;	//NAT_WAIT
	int waitingInNativeVTFcode;				// used to avoid taking time while in VEX code

	bool awaken;
	bool simulatingModel;

	bool currentlyPolledForWhetherItIsNativeWaiting; 	// flag so that only one scheduler polls you at each time
	pthread_mutex_t suspendLock;
	pthread_mutex_t sharedAccessKey;

	pthread_cond_t condSuspendLock;
	unsigned int consecutiveTimeslots;
	int consecutiveTimesFoundNativeWaiting;

	long long doNotSuspendUntil;
	static ThreadManager *defaultThreadManager;
};

#endif /* SCHEDULING_H_ */
