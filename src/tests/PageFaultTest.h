/*
 * SequenceVerification.h
 *
 *  Created on: 12 Oct 2010
 *      Author: root
 */

#ifndef PAGEFAULT_H_
#define PAGEFAULT_H_

#include "AgentTester.h"

#define PAGESIZE 4096
class PageFaultTest : public Tester {
public:
	PageFaultTest(int _threads) : Tester() {
		threads = _threads;
	}
	virtual ~PageFaultTest();
	bool test();
	std::string testName();
private:
	int threads;

};


#endif /* PAGEFAULT_H_ */
