/*
 * LockRegistry.cpp
 *
 *  Created on: 21 Nov 2011
 *      Author: root
 */

#include "LockRegistry.h"
#include "ThreadState.h"
#include <cassert>
/*
 * Class used to store which locks have their monitors being waited on.
 * This is used for Java threads simulation
 */

#include <iostream>
using namespace std;

bool LockRegistryInfo::releaseMonitor() {
	monitorLocked = false;
	return true;
}

void LockRegistryInfo::print() {

	vector<VexThreadState *>::iterator i = threadsContendedOnLock->begin();
	cout << "Mutex locked? " << monitorLocked << endl;
	while (i != threadsContendedOnLock->end()) {
		cout << "\t" << (*i)->getName() << " (" << (*i)->getId() << ")" << endl;
		++i;
	}
}


bool ReentrantLockRegistryInfo::releaseMonitor() {
	if (--counter == 0) {
		monitorLocked = false;
		owner = NULL;
//		cout << "-----" << " counter " << counter << endl;
		return true;
	} else {
//		cout << "-----" << " counter " << counter << endl;
		return false;
	}
}

void ReentrantLockRegistryInfo::print() {

	vector<VexThreadState *>::iterator i = threadsContendedOnLock->begin();
	cout << "Mutex locked? " << monitorLocked << " " << counter << " times";
	if (owner != NULL) {
		cout << " by " << owner->getName() << " " << owner->getId();
	}
	cout << endl;
	while (i != threadsContendedOnLock->end()) {
		cout << "\t" << (*i)->getName() << " (" << (*i)->getId() << ")" << endl;
		++i;
	}
}


LockRegistry::LockRegistry() {
	registry = new map<long, LockRegistryInfo *>;
	pthread_mutex_init(&mutex, NULL);
}

void LockRegistry::lockRegistryMutex() {
	pthread_mutex_lock(&mutex);
}

void LockRegistry::unlockRegistryMutex() {
	pthread_mutex_unlock(&mutex);
}

void LockRegistry::print() {
	map<long, LockRegistryInfo *>::iterator registry_iter = registry->begin();
	while (registry_iter != registry->end()) {
		cout << "Lock registry for lock with id: " << registry_iter->first;
		cout << "================================" << endl;
		registry_iter->second->print();
		cout << endl;
		++registry_iter;
	}
}

/**
 * Invoked when a thread tries to acquire a lock
 * If the lock is already acquired by another thread
 * then the method adds the thread into a list of blocked threads
 * and returns false. Otherwise it registers the owner of the monitor (lock)
 * as the invoking thread and returns true.
 */
bool LockRegistry::tryAcquiringLock(VexThreadState *state, const long &lockId) {

	map<long, LockRegistryInfo *>::iterator registry_iter;

	lockRegistryMutex();
	registry_iter = registry -> find(lockId);

	if (registry_iter != registry -> end()) {
		LockRegistryInfo *lockRegistryInfo = (LockRegistryInfo *)registry_iter -> second;
		if (lockRegistryInfo->isMonitorFree()) {
			lockRegistryInfo->enterMonitor();
			////cout << state->getName() << " acquired monitor " << lockId << endl;
			unlockRegistryMutex();
			return true;
		} else {
			lockRegistryInfo->blockOnMonitor(state);
			////cout << state->getName() << " blocked on contended monitor " << lockId << endl;
			unlockRegistryMutex();
			return false;
		}
	} else {
		(*registry)[lockId] = new LockRegistryInfo();
		////cout << state->getName() << " acquired monitor " << lockId << endl;
		unlockRegistryMutex();
		return true;
	}
}




/*
 * This method releases an already acquired lock
 */
VexThreadState *LockRegistry::releaseAndGetNext(const long &lockId) {
	map<long, LockRegistryInfo *>::iterator registry_iter;
	lockRegistryMutex();

	registry_iter = registry -> find(lockId);

	VexThreadState *state = NULL;
	if (registry_iter != registry -> end()) {
		LockRegistryInfo *lockRegistryInfo = (LockRegistryInfo *)registry_iter -> second;

		vector<VexThreadState *> *lockThreadList = lockRegistryInfo -> getThreadsBlockedOnMonitor();

		if (lockThreadList -> empty()) {
			////cout << "monitor " << lockId << " released and there is no-one to claim it" << endl;
			lockRegistryInfo->releaseMonitor();
		} else {
			vector<VexThreadState *>::iterator vect_iter = lockThreadList -> end();
			--vect_iter;
			state = *vect_iter;	// the state is now the owner of the monitor
			lockThreadList -> pop_back();

			////cout << state->getName() << " acquired monitor " << lockId << " that was just released" << endl;
		}
	}

	unlockRegistryMutex();

	return state;


}


bool ReentrantLockRegistry::tryAcquiringLock(VexThreadState *state, const long &lockId) {
	map<long, LockRegistryInfo *>::iterator registry_iter;

	lockRegistryMutex();
	registry_iter = registry -> find(lockId);

	if (registry_iter != registry -> end()) {
		ReentrantLockRegistryInfo *lockRegistryInfo = (ReentrantLockRegistryInfo *)registry_iter -> second;
		if (lockRegistryInfo->isMonitorFree() || lockRegistryInfo->isCurrentOwnerThread(state)) {
			lockRegistryInfo->enterMonitor(state);
			cout << state->getName() << " acquired monitor " << lockId << endl;
			unlockRegistryMutex();
			return true;
		} else {
			lockRegistryInfo->blockOnMonitor(state);
			cout << state->getName() << " blocked on contended monitor " << lockId << endl;
			unlockRegistryMutex();
			return false;
		}
	} else {
		(*registry)[lockId] = new ReentrantLockRegistryInfo(state);
cout << state->getName() << " CREATED AND acquired monitor " << lockId << endl;
		unlockRegistryMutex();
		return true;
	}
}

/*
 * This method releases an already acquired lock
 */
VexThreadState *ReentrantLockRegistry::releaseAndGetNext(const long &lockId) {
	map<long, LockRegistryInfo *>::iterator registry_iter;
	lockRegistryMutex();

	registry_iter = registry -> find(lockId);

	VexThreadState *state = NULL;
	if (registry_iter != registry -> end()) {
		ReentrantLockRegistryInfo *lockRegistryInfo = (ReentrantLockRegistryInfo *)registry_iter -> second;

		vector<VexThreadState *> *lockThreadList = lockRegistryInfo -> getThreadsBlockedOnMonitor();

		if (lockRegistryInfo->releaseMonitor() && !lockThreadList -> empty()) {
			vector<VexThreadState *>::iterator vect_iter = lockThreadList -> end();
			--vect_iter;
			state = *vect_iter;	// the state is now the owner of the monitor
			lockThreadList -> pop_back();
			lockRegistryInfo->setNextOwner(state);
//			cout << state->getName() << " acquired monitor " << lockId << " that was just released" << endl;
		}
	} else {
		cout << " COULD NOT FIND monitor " << lockId << endl;
		print();
		assert(false);
	}

	unlockRegistryMutex();
	return state;
}

//
///*
// * Erase a thread waiting record from this lock monitor
// */
////void LockRegistry::erase(const int &lockId, vector<long>::iterator threadRecord) {
//void LockRegistry::erase(const int &lockId, const long &threadId) {
//	map<int, LockRegistryInfo *>::iterator registry_iter;
//	registry_iter = registry -> find(lockId);
//
//	if (registry_iter != registry -> end()) {
//		LockRegistryInfo *lockRegistryInfo = (LockRegistryInfo *)registry_iter -> second;
//		vector<long> *lockThreadList = lockRegistryInfo -> getThreadsBlockedOnMonitor();
//
//		if (lockThreadList -> empty()) {
//			return;
//		} else {
//
//			vector<long>::iterator threadRecord = lockThreadList -> begin();
//			while (threadRecord != lockThreadList->end()) {
//				if (threadId == *threadRecord) {
//					lockThreadList -> erase(threadRecord);
//					return;
//				}
//				++threadRecord;
//			}
//
//		}
//	}
//
//}
//
///*
// * Add a new thread waiting on this lock monitor
// */
//void LockRegistry::insert(const int &lockId, const long &threadId) {
//	map<int, LockRegistryInfo *>::iterator registry_iter;
//	registry_iter = registry -> find(lockId);
//
//	LockRegistryInfo *lockRegistryInfo;
//	if (registry_iter != registry -> end()) {
//		lockRegistryInfo = (LockRegistryInfo *)registry_iter -> second;
//		vector<long> *lockThreadList = lockRegistryInfo -> getThreadsBlockedOnMonitor();
//
//		vector<long>::iterator vector_iter = lockThreadList -> end();
//		lockThreadList -> insert(vector_iter, threadId);
//		//lockThreadList -> push_back(threadId);
//
//	} else {
//		lockRegistryInfo = new LockRegistryInfo();
//		vector<long> *lockThreadList = lockRegistryInfo -> getThreadsBlockedOnMonitor();
//
//		vector<long>::iterator vector_iter = lockThreadList -> end();
//		vector_iter = lockThreadList -> insert(vector_iter, threadId);
//		(*registry)[lockId] = lockRegistryInfo;
//	//	return vector_iter;
//	}
//
//	lockRegistryInfo -> releaseMonitor();
//
//}
//
//
///*
// * Print lock registry contents
// */
//void LockRegistry::print() {
//	map<int, LockRegistryInfo *>::iterator registry_iter;
//	vector<long>::iterator threadRecord;
//
//
//	registry_iter = registry -> begin();
//	////cout << "Lock Registry Printing" << endl;
//	////cout << "========================" << endl;
//	while (registry_iter != registry-> end()) {
//
//		////cout << "Lock: " << registry_iter->first << endl;
//
//		LockRegistryInfo *lockRegistryInfo = (LockRegistryInfo *)registry_iter -> second;
//		vector<long> *lockThreadList = lockRegistryInfo -> getThreadsBlockedOnMonitor();
//
//		threadRecord = lockThreadList -> begin();
//		if (!lockThreadList ->empty()) {
//			////cout << "Threads waiting: ";
//			while (threadRecord != lockThreadList->end()) {
//				////cout << *threadRecord << " ";
//				++threadRecord;
//			}
//		} else {
//			////cout << "No threads waiting";
//		}
//		////cout << endl << endl;;
//		++registry_iter;
//
//	}
//}
//
///*
// * Check whether anyone is waiting on this monitor
// */
//bool LockRegistry::empty(const int &lockId) {
//	map<int, LockRegistryInfo *>::iterator registry_iter;
//	registry_iter = registry -> find(lockId);
//
//	if (registry_iter != registry -> end()) {
//		// this always exists as it created once and left there
//		LockRegistryInfo *lockRegistryInfo = (LockRegistryInfo *)registry_iter -> second;
//		vector<long> *lockThreadList = lockRegistryInfo -> getThreadsBlockedOnMonitor();
//
//		if (lockThreadList -> empty()) {
//			return true;
//		} else {
//			return false;
//		}
//	} else {
//		return true;
//	}
//}
//
//
///*
// * Check whether anyone is waiting on this monitor
// */
//bool LockRegistry::isLockFree(const int &lockId) {
//	map<int, LockRegistryInfo *>::iterator registry_iter;
//	registry_iter = registry -> find(lockId);
//
//	LockRegistryInfo *lockRegistryInfo;
//	if (registry_iter != registry -> end()) {
//		// this always exists as it created once and left there
//		lockRegistryInfo = (LockRegistryInfo *)registry_iter -> second;
//
//	} else {
//		lockRegistryInfo = new LockRegistryInfo();
//		(*registry)[lockId] = lockRegistryInfo;
//
//	}
//	return lockRegistryInfo -> isMonitorFree();
//}
//
//LockRegistryInfo *LockRegistry::getLockRegistryInfo(const int &lockId) {
//	map<int, LockRegistryInfo *>::iterator registry_iter;
//	registry_iter = registry -> find(lockId);
//
//	LockRegistryInfo *lockRegistryInfo;
//	if (registry_iter != registry -> end()) {
//		// this always exists as it created once and left there
//		lockRegistryInfo = (LockRegistryInfo *)registry_iter -> second;
//
//	} else {
//		lockRegistryInfo = new LockRegistryInfo();
//		(*registry)[lockId] = lockRegistryInfo;
//
//	}
//	return lockRegistryInfo;
//}

LockRegistry::~LockRegistry() {
	delete registry;
	pthread_mutex_destroy(&mutex);
}
