/*
 * ThreadRegistryTester.h
 *
 *  Created on: 12 Oct 2010
 *      Author: root
 */

#ifndef THREADREGISTRYTESTER_H_
#define THREADREGISTRYTESTER_H_

#include "Tester.h"

class ThreadRegistryTester: public Tester {
public:
	ThreadRegistryTester();
	virtual ~ThreadRegistryTester();
	bool test();
	string testName();

private:
	bool registryAddAndRetrieve();

};

#endif /* THREADREGISTRYTESTER_H_ */
