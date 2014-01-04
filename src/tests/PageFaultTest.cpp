/*
 * PerformanceTest.cpp
 *
 *  Created on: 12 Nov 2011
 *      Author: root
 */

#include "PageFaultTest.h"
#include "AgentTester.h"

#include <cstdlib>
#include "ThreadState.h"

PageFaultTest::~PageFaultTest() {

}

static int pagesToLoad = 0;
void *pageFaultingThreadRoutine(void *_threadId) {
	int *threadIdPtr = (int *)_threadId;
	int threadId = *threadIdPtr;
	// Default constructor using thread-id as uniqueId and name LWP-<thread-id>

	VexThreadState *state = new VexThreadState();
	char threadName[64];
	sprintf(threadName, "pageFaultingThread%d", threadId);
	state->setName(threadName);

	AgentTester::onThreadSpawn(state);

	int methodId = 500;
	int N = 100;
	methodEventsBehaviour->afterMethodEntry(methodId);
	// Allocate
	char **charBlock = new char *[pagesToLoad*PAGESIZE];
	for (int i = 0; i < pagesToLoad*PAGESIZE; i++) {
		charBlock[i] = new char[PAGESIZE];
	}

	// Touch
	for (int i = 0; i < N; i++) {
		int indexi = rand() % (pagesToLoad*PAGESIZE);
		int indexj = rand() % PAGESIZE;
		charBlock[indexi][indexj] = rand();
	}

	// Free
	for (int i = 0; i < PAGESIZE; i++) {
		delete[] charBlock[i];
	}
	delete[] charBlock;
	methodEventsBehaviour->beforeMethodExit(methodId);

	AgentTester::onThreadEnd(state);

	return NULL;
}


bool PageFaultTest::test() {

	long long start = Time::getRealTime();
	spawnThreadsToExecute(threads, pageFaultingThreadRoutine);
	long long finish = Time::getRealTime();
	cout << "Real time: " << (double)(finish - start)/1e9  << endl;
	if (AgentTester::virtualExecution) {
		cout << "VTF time: " << (double)threadEventsBehaviour->getTime()/1e9 << endl;
	}
	return true;
}

std::string PageFaultTest::testName() {
	return "Identify limitations on page faults";
}


int main(int argc, char **argv) {

	if (argc != 3) {
		cout << "Syntax error: ./pagefault <threads> <pages to load>" << endl;
		return -1;
	}

	int threads = atoi(argv[1]);
	pagesToLoad = atoi(argv[2]);
	char options[128];
	strcpy(options, "file=options");
	VEX::initializeSimulator(options);

	eventLogger->registerMethod("main", 100);
	threadEventsBehaviour->onThreadMainStart(12521);
	methodEventsBehaviour->afterMethodEntry(100);

	PageFaultTest *ptest = new PageFaultTest(threads);
	ptest->test();

	methodEventsBehaviour->beforeMethodExit(100);
	delete ptest;

	threadEventsBehaviour->onEnd();
	VEX::endSimulator();
	//VEX::printResults();
	return 0;
}
