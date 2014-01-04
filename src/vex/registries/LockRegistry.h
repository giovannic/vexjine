/*
 * LockRegistry.h
 *
 *  Created on: 21 Nov 2011
 *      Author: root
 */

#ifndef LOCKREGISTRY_H_
#define LOCKREGISTRY_H_

#include "ThreadState.h"

class LockRegistryInfo {
public:
	LockRegistryInfo() {
		threadsContendedOnLock 	= new vector<VexThreadState *>;
		monitorLocked 	= true;
	}

	void enterMonitor() {
		monitorLocked = true;
	}

	virtual bool releaseMonitor();
	virtual void print();

	bool isMonitorFree() {
		return !monitorLocked;
	}

	void blockOnMonitor(VexThreadState *state) {
		threadsContendedOnLock->push_back(state);
	}

	vector<VexThreadState *> *getThreadsBlockedOnMonitor() {
		return threadsContendedOnLock;
	}

	virtual ~LockRegistryInfo() {
		delete threadsContendedOnLock;
	}

protected:
	vector<VexThreadState *> *threadsContendedOnLock;
	bool monitorLocked;
};


class ReentrantLockRegistryInfo : public LockRegistryInfo {
public:

	ReentrantLockRegistryInfo(VexThreadState *state) : LockRegistryInfo() {
		owner = state;
		counter = 1;
	}

	void enterMonitor(VexThreadState *state) {
		monitorLocked = true;
		owner = state;
		++counter;
//		cout << "+++++" << state->getName() << " counter " << counter << endl;
	}

	virtual bool releaseMonitor();
	virtual void print();

	void setNextOwner(VexThreadState *state) {
		owner = state;
		counter = 1;
	}

	bool isCurrentOwnerThread(VexThreadState *state) {
		return state == owner;
	}

	bool isMonitorFree() {
		return !monitorLocked;
	}

protected:
	VexThreadState *owner;
	int counter;	// used to count times of entry in reentrant locks
};



class LockRegistry {
public:
	LockRegistry();

	void lockRegistryMutex();
	void unlockRegistryMutex();

	void print();

	virtual bool tryAcquiringLock(VexThreadState *state, const long &lockId);
	virtual VexThreadState *releaseAndGetNext(const long &lockId);

	virtual ~LockRegistry();

protected:
	// Registry: object ID <-> array of thread IDs (timed) waiting on that object monitor
	pthread_mutex_t mutex;
	//unordered_map<int, LockRegistryInfo *> *registry;
	map<long, LockRegistryInfo *> *registry;


};


class ReentrantLockRegistry : public LockRegistry {
public:
	ReentrantLockRegistry() : LockRegistry() {};
	bool tryAcquiringLock(VexThreadState *state, const long &lockId);
	VexThreadState *releaseAndGetNext(const long &lockId);

};

#endif /* LOCKREGISTRY_H_ */
