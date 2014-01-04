/*
 * Tester.h
 *
 *  Created on: 6 Jul 2010
 *      Author: nb605
 */

#ifndef TESTER_H_
#define TESTER_H_

#include "../VTF.h"

#include <iostream>
#include <time.h>
#include <signal.h>
#include <string>
#include "ThreadManager.h"
using namespace std;
using namespace VEX;

namespace VEX {
	extern ThreadManager *manager;
}

class Tester {
public:
	Tester();
	virtual bool test() = 0;
	virtual string testName() = 0;

	virtual ~Tester();

	static void enableVTF();
	static bool isVTFenabled();
	static void disableVTF();

	static bool virtualExecution;

protected:
	void startVTFManager();
	void endVTFManager();
	void spawnThreadsToExecute(int threadsCount, void *(*threadRoutine)(void *));

private:
	pthread_t manager_thread;

};

#endif /* TESTER_H_ */
