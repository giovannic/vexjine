/*
 * IoProtocol.h: class used to remove the I/O module management from the ThreadManager
 * class. This cannot be done by subclasses of ThreadManager, because we have other
 * subclasses that might be used with any of the I/O modules.
 *
 *  Created on: 12 Oct 2010
 *      Author: root
 */

#ifndef IOPROTOCOL_H_
#define IOPROTOCOL_H_

#include "ThreadState.h"
class ThreadManagerRegistry;

// Sort-of abstract superclass
class IoProtocol {

public:
	IoProtocol();

	virtual void onStart(VexThreadState *state);
	virtual void onEnd(VexThreadState *state, const long long &actualIoDuration);

	virtual const char *getTitle();
	virtual bool areInIoThreadsInRunnablesQueue();
	virtual ~IoProtocol();

};

class IoProtocolSerial : public IoProtocol {
public:
	IoProtocolSerial();
	virtual const char *getTitle();
	void onStart(VexThreadState *state);
	void onEnd(VexThreadState *state, const long long &actualIoDuration);

};

class IoProtocolStrict : public IoProtocol {
public:
	IoProtocolStrict();
	virtual const char *getTitle();
	void onStart(VexThreadState *state);
	void onEnd(VexThreadState *state, const long long &actualIoDuration);
};

class IoProtocolLax : public IoProtocol {
public:
	IoProtocolLax();
	virtual const char *getTitle();
	void onStart(VexThreadState *state);
	void onEnd(VexThreadState *state, const long long &actualIoDuration);
};

class IoProtocolNormal : public IoProtocol {
public:
	IoProtocolNormal();
	virtual const char *getTitle();
	void onStart(VexThreadState *state);
	void onEnd(VexThreadState *state, const long long &actualIoDuration);
	bool areInIoThreadsInRunnablesQueue();

protected:
	void minimizeSchedulerTimeslot(ThreadManager *stateManager);
	void resetSchedulerTimeslot(ThreadManagerRegistry *allThreadManagers);

	pthread_spinlock_t spinlock;
	int pendingIoCalls;
};


class IoProtocolEnforcer : public IoProtocol {
public:
	IoProtocolEnforcer(IoProtocol *serialSystemCallRealTimeProtocol, IoProtocol *parallelIoRealTimeProtocol);

	virtual void onStart(VexThreadState *state);
	virtual void onEnd(VexThreadState *state, const long long &actualIoDuration);

protected:
	IoProtocol *serialSystemCallRealTimeProtocol;
	IoProtocol *parallelIoRealTimeProtocol;
};
#endif /* IOPROTOCOL_H_ */
