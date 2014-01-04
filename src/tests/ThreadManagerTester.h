/*
 * ThreadManagerTester.h
 *
 *  Created on: 13 Oct 2010
 *      Author: root
 */

#ifndef THREADMANAGERTESTER_H_
#define THREADMANAGERTESTER_H_

#include "Tester.h"

class ThreadManagerTester: public Tester {
public:
	ThreadManagerTester();
	ThreadManagerTester(int _threads) : threads(_threads) {};

	bool test();
	string testName();

	virtual ~ThreadManagerTester();

	void setThreads(int threads);

	static void printTests();
private:
	int threads;


};

#endif /* THREADMANAGERTESTER_H_ */
