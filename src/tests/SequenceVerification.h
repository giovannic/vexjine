/*
 * SequenceVerification.h
 *
 *  Created on: 12 Oct 2010
 *      Author: root
 */

#ifndef SEQUENCEVERIFICATION_H_
#define SEQUENCEVERIFICATION_H_

#include "AgentTester.h"

#define LOOPS_METHOD_ID 110
#define NORMAL_METHOD_ID 111
#define ACCELERATED_METHOD_ID 112
#define DECELERATED_METHOD_ID 113

class SequenceVerification : public Tester {
public:
	SequenceVerification(int threadsPerCategory);
	virtual ~SequenceVerification();
	bool test();
	std::string testName();

private:
	int threads;
};

class LoopTest {
public:
	LoopTest();
	~LoopTest();

	double calculateE();
	virtual double run();
};

class AcceleratedLoopTest : public LoopTest {
public:
	virtual double run();
};

class DeceleratedLoopTest : public LoopTest {
public:
	virtual double run();
};

#endif /* SEQUENCEVERIFICATION_H_ */
