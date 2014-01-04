/*
 * ForwardLeapTest.h
 *
 *  Created on: 21 Nov 2011
 *      Author: root
 */

#ifndef FORWARDLEAPTEST_H_
#define FORWARDLEAPTEST_H_

#include "AgentTester.h"

class ForwardLeapTest {
public:
	ForwardLeapTest();
	virtual void run() = 0;
	virtual ~ForwardLeapTest();
protected:
	bool allow(VexThreadState *state);
	bool forbid(VexThreadState *state);
};

class StateCheckingForwardLeapTest : public ForwardLeapTest {
public:
	void run();
};

#endif /* FORWARDLEAPTEST_H_ */
