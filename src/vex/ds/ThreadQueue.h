/*
 * ThreadQueue.h: The priority queue that is used to store all runnable threads
 * and determine the next thread to be resumed by the VEX scheduler
 *
 *  Created on: 9 Apr 2010
 *      Author: nb605
 */
#ifndef THREADQUEUE_H_
#define THREADQUEUE_H_

#include <queue>
#include <pthread.h>
#include "ThreadState.h"
#include "AdaptiblePQueue.h"
#include "Logger.h"

#include <sstream>

class ThreadManager;

class ThreadQueue {
public:
	ThreadQueue();

	void lockMutex();
	void unlockMutex();

	void push(VexThreadState *state);
	VexThreadState *top();
	VexThreadState *getNext();
	VexThreadState *getNextIfEqualsElseReturnTop(VexThreadState *state, ThreadManager *requestingManager);
	VexThreadState *getNext(ThreadManager *requestingManager);

	bool empty();
	unsigned int size();
	void setThreadToNextBiggestErt(VexThreadState *state);
	bool isNextRunnableThreadBefore(const long long &timestamp);

	void update();
	void erase(VexThreadState *state);
	void print();

	bool find(VexThreadState *state);
	long long getHighestRunnableTime(VexThreadState *askingThread);	// used for yield
	~ThreadQueue();

	void setLog(Log *logger) {
		queueLogger = logger;
	}

	std::string getAllCallingMethodsUntil(short maxStack);
	void invalidateExpiredIoPredictions(const long long &currentRealTime);

private:
	void recursivelyUpdatePendingIoRequests(VexThreadState *state, const long long &currentRealTime);
	AdaptiblePQueue<VexThreadState *, std::deque<VexThreadState *>, threadStatePtr_compare> *threadsQueue;
	pthread_mutex_t mutex;
	long long highestRunnableTime;
	Log *queueLogger;
};

#endif /*THREADQUEUE_H_*/
