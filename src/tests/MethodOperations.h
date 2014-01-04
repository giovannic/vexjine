/*
 * MethodOperations.h
 *
 *  Created on: 12 Oct 2010
 *      Author: root
 */

#ifndef METHODOPERATIONS_H_
#define METHODOPERATIONS_H_

#include "AgentTester.h"

#include <time.h>

class MethodOperations {
public:
	MethodOperations();

	virtual void run() = 0;
	virtual ~MethodOperations();

protected:
	float duration;

};

class Calculations : public MethodOperations {
public:
	Calculations(float _duration);
	~Calculations();

	void run() {
//	cout << "calculations of " << duration <<endl;
//		long iterations = 7680000 * duration;
//		for (long i  =0 ; i<iterations; ++i) {
//			temp += 1;
//		}
		temp /= duration;
	};

private:
	double temp;
};

class IoOperation : public MethodOperations {
public:
	IoOperation(float _duration, int _methodId, int _invocationPointHashValue);
	~IoOperation();
	void run() {
//		cout << "io of " << duration <<endl;
		struct timespec ioTime;
		ioTime.tv_nsec =  (long)(1000 * duration);
		ioTime.tv_sec = 0;

		AgentTester::afterMethodIoEntry(methodId, invocationPointHashValue);
		nanosleep(&ioTime, NULL);
		AgentTester::beforeMethodIoExit(methodId);

	};

private:
	int methodId;
	int invocationPointHashValue;

};


class WaitOperation : public MethodOperations {
public:
	WaitOperation(float _duration);
	~WaitOperation();

	void run() {

//		cout << "wait of " << duration <<endl;

		struct timespec waitingTime;
		waitingTime.tv_nsec =  (long)(1000000000 * duration);
		waitingTime.tv_sec = 0;

		AgentTester::onThreadWaitingStart(waitingTime.tv_nsec);
		nanosleep(&waitingTime, NULL);
		AgentTester::onThreadWaitingEnd();
	};

};
#endif /* METHODOPERATIONS_H_ */
