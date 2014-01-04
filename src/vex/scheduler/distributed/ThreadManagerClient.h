/*
 * ThreadManagerClient.h
 *
 *  Created on: 4 Oct 2010
 *      Author: nb605
 */

#ifndef DISTRIBUTEDMANAGERCLIENT_H_
#define DISTRIBUTEDMANAGERCLIENT_H_

#include "Logger.h"
#include "ThreadManager.h"
#include "ThreadManagerCommunicationProtocol.h"

class ThreadManagerClient : public ThreadManager {
public:

	ThreadManagerClient(unsigned int _id, VirtualTimelineController *_globalTimer, ThreadQueue *runnableThreads, IoSimulator *ioSim, ThreadRegistry *tReg, ObjectRegistry *oReg) : ThreadManager(_id, _globalTimer, runnableThreads, ioSim, tReg, oReg) {
		//clientInit(_globalTimer);//TODO: I BROKE IT
	};
	~ThreadManagerClient();

	bool initialize(char *, int);

	void acceptServerRequests(int fd);

	void leapToGlobalTime(VexThreadState *state);

	int wakeup();
	int conditionalWakeup(VexThreadState *state);
	int unconditionalWakeup();
	void unFreeze();

//	/*** Methods for changing the state of a thread (from the VTF-scheduler perspective) ******/
	void setRunningThread(VexThreadState *state);
	void setRunnableThread(VexThreadState *state);
	void setWaitingThread(VexThreadState *state);
	void setNativeWaiting(VexThreadState *state);
	void setIoThread(VexThreadState *state, bool learning);
	void setVirtualTimeBlockingThread(VexThreadState *state);
//
//	/*** Method to be called throughout the thread lifecycle ******/
	void onThreadSpawn(VexThreadState *state);

	// Called to indicate that the given thread is waiting
	void onThreadWaitingStart(const long long &startingTime, VexThreadState *state);
	void onThreadWaitingStart(VexThreadState *state);

	// Called when a thread restarts after waiting
	void onThreadWaitingEnd(VexThreadState *state);

    void onThreadContendedEnter(VexThreadState * state, const long long & presetTime);
    void onThreadContendedEntered(VexThreadState *state);

	void onThreadIoStart(VexThreadState *state);
	void onThreadIoEnd(const long long &realTimeValueOnExit, VexThreadState *state, const int &methodId);
//
	// Called to notify the thread manager that the thread is about to execute an action in which it will be suspended
	// like waiting or sleeping) that expires after a timeout. This should be correctly simulated in virtual time
	void onThreadTimedWaitingStart(VexThreadState *state, long &timeout);
	// Either when the timed action finishes or when it gets interrupted
    void onThreadTimedWaitingEnd(VexThreadState * state, const long  & interruptTime);

	//void resumeWaitingThread(VexThreadState *state, const long &interruptTime);
//
	void onThreadYield(VexThreadState *state, const long long &startingTime);
	void onThreadEnd(VexThreadState *state);
//
//	// Called just before an interaction point
//	void onThreadInteractionPointEncounter(VexThreadState *state, const long long& startingTime);
//
//
//
//	/*** Methods to update the running thread's clocks ****/
	void updateCurrentThreadVT(VexThreadState *state, bool useApproximation);
	void setCurrentThreadVT(const long long &presetTime, VexThreadState *state);	// used to avoid lost times

//
//	/**Suspends the current thread - if the shouldCurrentThreadSuspend conditions are fulfilled.*/
	void suspendCurrentThread(VexThreadState *state, const long long & startingTime, const char & options);
//	/**Check if the current thread that was asked to be suspended should actually suspend**/
	bool shouldCurrentThreadSuspend(VexThreadState *state);
	bool changeVexThreadStateToRunning(VexThreadState *state);


//	void blockCurrentThread(VexThreadState *state);	/* Immediately block thread - suspend after awaking*/

	void addVexThreadState(VexThreadState *state) ;
	void ps() ;
	void printVexThreadStates() ;
	void printStats(const char *file_name) ;

	void notifySchedulerForVirtualizedTime(VexThreadState *state, const float &scalingFactor);
//	void resumeThread(VexThreadState *state);

	void sendSchedulerRequest(int code, VexThreadState *state); 		// Accessed from the handler
	void sendSchedulerRequest(int code, VexThreadState *state, long long timestamp);

	long long getCurrentGlobalTime();

	void end();

	void setRunningThreadToState(VexThreadState *state);
	Log *getLogger() {
		return logger;
	}
	int listenerSockFd;

	short askServerWhetherThreadShouldSuspend(VexThreadState *state); 	// called from the signal handler
protected:
	// Remake the heap of the threads
	void updateThreadList();
	bool suspendThread(VexThreadState *state);

	void registerSignalHandler();


private:
	int sockfd;
	void clientInit(VirtualTimeline *vt);


	unsigned int listenerFdOfClientAtServer;
	bool notified;

	int createListenerThread(int port);
	int initListenerThread(int port);
	int connectToNextFreePort(int port);
	bool serviceServerRequest(SchedulerRequest serverRequest);
	void registerListenerThreadPort(int port);
	bool connectToServer(char *, int);

	void sendSchedulerRequest(int code);
	void sendSchedulerRequest(int code, int fd);
	void sendSchedulerRequest(int code, float speedup, VexThreadState *state);
	void sendSchedulerRequest(int code, VexThreadState *state, long long timestamp, const char &c);
	void sendSchedulerRequest(int code, VexThreadState *state, long long timestamp, const int &i, const int &s);
	void notifyServerThatThreadCannotBeSuspended();

	int getSchedulerResponseInt();
	long long getSchedulerResponseLongLong();
	bool getSchedulerResponseBool();
	void getSchedulerResponseTimes(VexThreadState *state);

};

#endif /* DISTRIBUTEDMANAGERCLIENT_H_ */
