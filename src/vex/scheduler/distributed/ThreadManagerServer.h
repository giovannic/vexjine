/*
 * ThreadManagerServer.h
 *
 *  Created on: 4 Oct 2010
 *      Author: root
 */

#ifndef THREADMANAGERSERVER_H_
#define THREADMANAGERSERVER_H_

#include "ThreadManager.h"
#include "ThreadManagerCommunicationProtocol.h"


void *startMainThread(void *);
class ThreadManagerServer : public ThreadManager {
public:

	ThreadManagerServer(unsigned int _id, VirtualTimelineController *_globalTimer, ThreadQueue *runnableThreads, IoSimulator *ioSim, ThreadRegistry *tReg, ObjectRegistry *oReg) : ThreadManager(_id, _globalTimer, runnableThreads, ioSim, tReg, oReg) {}
	~ThreadManagerServer();

	void startManagerServerThread();
	bool init(int port);

	virtual void suspendRunningResumeNext();
	virtual void interruptTimedOutIoThread(VexThreadState *state);

	/*** Methods for changing the state of a thread (from the VTF-scheduler perspective) ******/
	void setIoThread(VexThreadState *state, const bool &learning);

	/*** Method to be called throughout the thread lifecycle ******/
	void onThreadIoEnd(const long long &realTimeValueOnExit, VexThreadState *state, const int &methodId);

	// Called to notify the thread manager that the thread is about to execute an action in which it will be suspended
	// like waiting or sleeping) that expires after a timeout. This should be correctly simulated in virtual time
	// Either when the timed action finishes or when it gets interrupted
	void resumeWaitingThread(VexThreadState *state, const long &interruptTime);

	/**Suspends the current thread - if the shouldCurrentThreadSuspend conditions are fulfilled.*/
	void suspendCurrentThread(VexThreadState *state, const long long & startingTime, const char & options);
	void resumeThread(VexThreadState *state);

	void acceptConnections();
	void acceptClientRequests(int clientSockFd) ;

private:
	int sockfd;

	bool notifyClientScheduler(VexThreadState *state);

	bool serverSuspendThread(VexThreadState *state);
//	void serverResumeSuspendedThread(VexThreadState *state);

	void sendClientRequest(int code);
	void sendClientRequest(int code, VexThreadState *state);
	void sendRequestToClientListener(int code, int fd);
	void sendRequestToClientListener(int code, VexThreadState *state);
	void sendRequestToClientListener(int code, VexThreadState *state, long long timestamp);


	void getClientListenerTimeResponse(const int & fd, long long *buf);

	int createClientListenerConnection(struct sockaddr_in cli_addr) ;

	char getClientResponse(const int & fd);

	bool threadDidNotChangeStateOrDiedWhileWaitingForClientResponse(VexThreadState *state) {
		return state != NULL && state == runningThread && state->isRunning();
	};

	bool signalCouldNotBeDeliveredToThread(const long long & buffer) {
		return buffer == TIME_CODE_DENOTING_THAT_THREAD_WAS_NOT_SUSPENDED;
	};

	bool serviceClientRequest(const int &clientSockFd, SchedulerRequest clientRequest);

	inline size_t serverMessageWrite(int __fd, const void * _buf, size_t n) {
		size_t bwritten = write(__fd, _buf, n);
		LOG(logger, logINFO) << "clientServer->clientListener: " << n << " (" << bwritten << ") at" << getCallingMethod() << endl;
		return bwritten;
	};


	inline int serverMessageRead(int __fd, void * _buf, size_t n) {
		size_t bread = read(__fd, _buf, n);
		LOG(logger, logINFO) << "clientListener->clientServer: " << n << " (" << bread << ") at"  << getCallingMethod() << endl;
		return bread;
	};

	inline void serverLockMutex() {
		lockMutex();
		LOG(logger, logINFO) << "note right of clientServer: lockMutex() at " << getCallingMethod() << endl;
		LOG(logger, logINFO) << "activate clientServer" << endl;

	};

	inline void serverUnlockMutex() {
		LOG(logger, logINFO) << "note right of clientServer: unlockMutex() at " << getCallingMethod()  << endl;
		LOG(logger, logINFO) << "deactivate clientServer" << endl;
		unlockMutex();

	};


	void notifyRemoteThreadForSchedulerDecisionOnIfItShouldSuspend(VexThreadState *state, bool shouldSuspend, const char & options);
};

#endif /* THREADMANAGERSERVER_H_ */
