/*
 * Timer.h: timeline file
 *
 *  Created on: 28 Jun 2010
 *      Author: nb605
 */

#ifndef VIRTUALTIMELINE_H_
#define VIRTUALTIMELINE_H_

#include <iostream>
#include <syscall.h>
#include <iomanip>
#include <fstream>
#include <climits>

#include "Time.h"

// Get thread id through system call
//inline unsigned long gettid() {
//	return (unsigned long) syscall(SYS_gettid);
//}

class Timer {

public:
	Timer();
	void lock();
	void unlock();

	// Any non-manager lock should only be acquired by a thread, after the thread has acquired its controlling manager lock, so that
	// it will not get signalled, while holding the lock essentially blocking its controller => deadlock. The getGlobalTime() is
	// therefore used to avoid acquiring two locks in cases where the exact value of getGlobalTime does not matter.
	inline long long getGlobalTime() {
		return globalVirtualTime;
	};

	void reset();

	inline void setGlobalTimeTo(const long long & time) {
		lock();
		globalVirtualTime = time;
		unlock();
	};

	inline void progressGlobalTimeBy(const long long & time) {
		lock();
		globalVirtualTime += time;
		unlock();
	};

	inline bool tryForwardTimeLeap(const long long &time) {
		lock();
		if (time > globalVirtualTime) {
			globalVirtualTime = time;
			unlock();
			return true;
		} else {
			unlock();
			return false;
		}
	}

	virtual ~Timer();



	// GC START END....
	void onGarbageCollectionStart();
	long long getGarbageCollectionRealDuration();


	// TIME ADDITION STATISTICS - THIS IS THE MOST IMPORTANT BIT FOR ONCE AND FORALL FIXES
	void addStatsSample(const int &position, const long long &time) {
//		timestats.addSample(position, time);
	};

	// OUTPUT METHODS
	void printTotalTime();
	void printStats(char *file_name);



protected:
	pthread_spinlock_t spinlock;
    int spinint;

	/**The current global virtual time.*/
	long long globalVirtualTime;
	long long lastGCtime;
	long long accGCtime;

	/**Time progress monitoring module: flag and data structure */
	long totalInstrumentationTime;
	int totalRemovals;

};



class MulticoreTimer : public Timer {
public:
	MulticoreTimer() : Timer() {};
	MulticoreTimer(const int &schedulers) : Timer() {
		localCoreVirtualTimes = new long long[schedulers];
		localCoreActive = new bool[schedulers];
		for (int i =0 ; i<schedulers;i++) {
			localCoreVirtualTimes[i] = 0;//LLONG_MAX;
			localCoreActive[i]=false;
		}
		processors = schedulers;
	}

	inline long long activateAndGetLocalTimeOfScheduler(const int &managerId) {
			lock();
			localCoreActive[managerId] = true;
			long long currentValue = localCoreVirtualTimes[managerId];
	cout << "activating " << managerId << " with local time " << localCoreVirtualTimes[managerId] << endl;
			unlock();
			return currentValue;
		}

		inline void activateManager(const int &managerId) {
			lock();
	cout << "activating " << managerId << " with local time " << localCoreVirtualTimes[managerId] << endl;
			localCoreActive[managerId] = true;
			unlock();
		}
		inline long long getLocalTimeOfScheduler(const int &managerId) {
			lock();
			long long currentValue;
			if (localCoreActive[managerId]) {
				currentValue = localCoreVirtualTimes[managerId];
	cout << "getLocalTimeOfScheduler = local["<<managerId<<"] = " << currentValue << endl;
			} else {
				currentValue = globalVirtualTime;
	cout << "getLocalTimeOfScheduler = global = " << currentValue << endl;
			}
			unlock();
			return currentValue;
		};
		inline void setLocalTimeOfScheduler(const int &managerId, const long long &time) {
			lock();
			localCoreVirtualTimes[managerId] = time;
			updateGlobalVirtualTime(managerId);
	cout << "setting time of " << managerId << " to " << time << " and updating GVT to " << globalVirtualTime << endl;
			unlock();
		};

		inline void updateGlobalVirtualTime(const long long &managerId) {
			long long min = LLONG_MAX;
			for (int i = 0; i < processors; i++) {
				if (localCoreActive[i] && localCoreVirtualTimes[i] < min) {
					min = localCoreVirtualTimes[i];
				}
			}
			if (min != LLONG_MAX) {
				globalVirtualTime = min;
			}
		}

		inline bool tryForwardTimeLeap(const long long &time, const int &managerId) {
			if (time > localCoreVirtualTimes[managerId]) {
				setLocalTimeOfScheduler(managerId, time);
				return true;
			} else {
				return false;
			}
		}


		inline long long getGlobalTime(const int &managerId) {
			lock();
			updateGlobalVirtualTime(managerId);
			unlock();
			return globalVirtualTime;
		};

		// No locking required for getting the local time from the context of the scheduler itself
		inline long long getMySchedulerLocalTime(const int &managerId) {
			return localCoreVirtualTimes[managerId];
		};
	//	inline long long getLastTimeCompletedTimestamp(const int &managerId) {
	//		return lastTimeCompletedTimestamp[managerId];
	//	};






		// Returns true if all schedulers are disabled
		inline bool disableLocalTimeOfScheduler(const int &managerId) {
			bool allSchedulersAreDisabled = false;
			lock();
			localCoreActive[managerId] = false;
			updateGlobalVirtualTime(managerId);
			allSchedulersAreDisabled = !atLeastOneSchedulerEnabled();
	cout << "disabling scheduler " << managerId << " and getting new GVT " << globalVirtualTime << ". Are all off? " << allSchedulersAreDisabled << endl;
			unlock();
			return allSchedulersAreDisabled;
		}

		inline bool atLeastOneSchedulerEnabled() {
			for (int i = 0; i < processors; i++) {
				if (localCoreActive[i]) {
					return true;
				}
			}
			return false;
		}

		inline long long differenceFromGVT(const int &managerId) {
			lock();
			long long difference = localCoreVirtualTimes[managerId] - globalVirtualTime;
			unlock();
			return difference;

		}

		inline void progressGlobalTimeBy(const long long & time, const int &managerId) {
			lock();
			localCoreVirtualTimes[managerId] += time;
			if (localCoreActive[managerId]) {
				updateGlobalVirtualTime(managerId);
			} else {
				// Can come here when updating time from native waiting thread
				localCoreActive[managerId] = true;
				updateGlobalVirtualTime(managerId);
				localCoreActive[managerId] = false;
			}
			unlock();
		};








		/*
		inline long long getGlobalTime(const int &managerId) {
			lock();
			updateGlobalVirtualTime(managerId);
			unlock();
			return globalVirtualTime;
		};

		// No locking required for getting the local time from the context of the scheduler itself
		inline long long getMySchedulerLocalTime(const int &managerId) {
			return localCoreVirtualTimes[managerId];
		};

		inline long long getLastTimeCompletedTimestamp(const int &managerId) {
			return lastTimeCompletedTimestamp[managerId];
		};

		inline long long getLocalTimeOfScheduler(const int &managerId) {
			lock();
			long long currentValue = localCoreVirtualTimes[managerId];
			if (currentValue == 0) {
				currentValue = lastTimeCompletedTimestamp[managerId];
				if (currentValue == 0) {
					currentValue = globalVirtualTime;
				}
			}
			unlock();
			return currentValue;
		};

		inline void setLocalTimeOfScheduler(const int &managerId, const long long &time) {
			lock();
			localCoreVirtualTimes[managerId] = time;
			unlock();
		};

		inline void setGlobalTimeTo(const long long & time) {
			lock();
			globalVirtualTime = time;
			unlock();
		};

		inline void updateGlobalVirtualTime(const long long &managerId) {
			long long min = LLONG_MAX;
			for (int i = 0; i < processors; i++) {
				if (localCoreVirtualTimes[i] < min && localCoreVirtualTimes[i] != 0) {
					min = localCoreVirtualTimes[i];
				}
			}
			if (min != LLONG_MAX) {
	//			if (min >= globalVirtualTime) {
				globalVirtualTime = min;
	//			} else {
	//				cout << "mistake caused in update because globalVirtualTime " << globalVirtualTime << " and min " << min << " and:" << endl;
	//				for (int i = 0; i < processors; i++) {
	//					cout << i << " " << localCoreVirtualTimes[i] << endl;
	//				}
	//			}
			}
		}



		// Returns true if all schedulers are disabled
		inline bool disableLocalTimeOfScheduler(const int &managerId) {

			lock();
			updateGlobalVirtualTime(managerId);
			if (localCoreVirtualTimes[managerId] > 0) {
				lastTimeCompletedTimestamp[managerId] = localCoreVirtualTimes[managerId];
			} else {
				// The idea is to move forward in case all schedulers are disabled and no progress can be made
				if (!atLeastOneSchedulerEnabled()) {
					unlock();
					return true;
				}
			}
			localCoreVirtualTimes[managerId] = 0;
			unlock();
			return false;
		}

		inline bool atLeastOneSchedulerEnabled() {
			for (int i = 0; i < processors; i++) {
				if (localCoreVirtualTimes[i] != 0) {
					return true;
				}
			}
			return false;
		}

		inline void updateGlobalTimeToMaximumCommittedTime() {
			long long max = 0;
			for (int i = 0; i < processors; i++) {
				if (lastTimeCompletedTimestamp[i] > max) {
					max = lastTimeCompletedTimestamp[i];
				}
			}
			if (max > globalVirtualTime) {
				globalVirtualTime = max;
			}
		}

		inline long long differenceFromGVT(const int &managerId) {
			long long difference;
			lock();
			if (localCoreVirtualTimes[managerId] != 0) {
				difference = localCoreVirtualTimes[managerId] - globalVirtualTime;
			} else {
				difference = lastTimeCompletedTimestamp[managerId] - globalVirtualTime;
			}
			unlock();
			return difference;

		}

		inline void progressGlobalTimeBy(const long long & time, const int &managerId) {
			lock();
			if (localCoreVirtualTimes[managerId] != 0) {
				localCoreVirtualTimes[managerId] += time;
				updateGlobalVirtualTime(managerId);
			} else {
				lastTimeCompletedTimestamp[managerId] += time;
				localCoreVirtualTimes[managerId] = lastTimeCompletedTimestamp[managerId];
				updateGlobalVirtualTime(managerId);
				localCoreVirtualTimes[managerId] = 0;
			}
			unlock();
		};

		inline bool tryForwardTimeLeap(const long long &time, const int &managerId) {
			lock();
			if (time > localCoreVirtualTimes[managerId]) {
				localCoreVirtualTimes[managerId] = time;
				updateGlobalVirtualTime(managerId);
				unlock();
				return true;
			} else {
				unlock();
				return false;
			}
		}
	*/

protected:
	long long *localCoreVirtualTimes;			// Local time per scheduler
	bool *localCoreActive;
	int processors;
};

#endif /* VIRTUALTIMELINE_H_ */
