/*
 * ThreadExecutionPattern.h
 *
 *  Created on: 14 Oct 2010
 *      Author: root
 */

#ifndef THREADEXECUTIONPATTERN_H_
#define THREADEXECUTIONPATTERN_H_
#include "AgentTester.h"

class ThreadExecutionPattern {
public:
	ThreadExecutionPattern();
	ThreadExecutionPattern(VexThreadState * state) {};
	virtual void apply() = 0;
	virtual ~ThreadExecutionPattern();

	virtual bool isFunctional() = 0;		// distinguish real-vs-virtual performance from functional-
	static void setThreads(int _threads) {
		totalThreads = _threads;
	}

protected:
	VexThreadState *state;

	double busyCpu(float seconds) {
		long iterations = 795840000 * seconds;
		double temp = 0.0;
		for (long i  =0 ; i<iterations; i++) {
			temp += 1;
		}
		return temp;
	};

	static int totalThreads;

};


class StartSuspendExit : public ThreadExecutionPattern {
public:
	StartSuspendExit(VexThreadState * state);
	StartSuspendExit(VexThreadState * state, long long _totalThreadTime);

	void apply();
	bool isFunctional() { return true; }

	~StartSuspendExit();

private:
	long long totalThreadTime;
};


class IssuingConstantIo : public ThreadExecutionPattern {
public:
	IssuingConstantIo(VexThreadState * state, int _requests);
	void apply();
	bool isFunctional() { return false; }
	~IssuingConstantIo();

private:
	int ioRequests;
};


class IssuingVariableIo : public ThreadExecutionPattern {
public:
	IssuingVariableIo();
	IssuingVariableIo(VexThreadState * state, int _requests);
	virtual void apply();
	bool isFunctional() { return false; }

	~IssuingVariableIo();

	void static createRandomRequests(int);
	void static readRequestsFromFile(const char *filename);

	long long static getMaxIoDuration();

protected:
	int ioRequests;

	long getRandomIoDuration();

	static long *requestsDuration;
	static long long maxIoDuration;
	static int predefined;
};


class IssuingIoWhileRunningLoops : public ThreadExecutionPattern {
public:
	IssuingIoWhileRunningLoops(VexThreadState * state, int _requests, bool _isRunner);
	void apply();
	bool isFunctional() { return false; }
	~IssuingIoWhileRunningLoops();

	long static createRandomRequests(int);

private:
	int ioRequests;
	long getRandomIoDuration();
	bool isRunner;

	static long *requestsDuration;
};


class IssuingIoAndLoops : public IssuingVariableIo {
public:
	IssuingIoAndLoops(VexThreadState * state, int _requests);
	void apply();
	bool isFunctional() { return false; }
	~IssuingIoAndLoops();


};


class Barrier : public ThreadExecutionPattern {
public:
	Barrier(VexThreadState * state, int _loops, bool _isTimedBarrier);
	void apply();
	bool isFunctional() { return false; }
	~Barrier();
	static void init();

private:
	int loops;
	static int counter;
	double temp2;
	bool timedBarrier;
	static pthread_mutex_t lock;
	static pthread_cond_t cond;
};



class TimedWaiting : public ThreadExecutionPattern {
public:
	TimedWaiting(VexThreadState * state, const long long &totalThreadTime, bool _randomDurations);
	void apply();
	bool isFunctional() { return true; }
	~TimedWaiting();

private:
	long long totalThreadTime;
	bool randomDurations;
};

class ModelRunning : public ThreadExecutionPattern {
public:
	ModelRunning(VexThreadState *_state, const bool &_modelSchedulerSim);
	void apply();
	bool isFunctional() { return true; }
	~ModelRunning();

private:
	bool modelSchedulerSim;
};


#endif /* THREADEXECUTIONPATTERN_H_ */
