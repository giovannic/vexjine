/*
 * SequenceVerification.cpp
 *
 *  Created on: 12 Oct 2010
 *      Author: root
 */

#include "SequenceVerification.h"

#include "ThreadState.h"
#include <cstdlib>


SequenceVerification::SequenceVerification(int threadsPerCategory) {
	threads = 3*threadsPerCategory;
}

SequenceVerification::~SequenceVerification() {

}

/*
 * Each thread start the random program from main
 */
void *sequenceVerificationThreadRoutine(void *_threadId) {
	int *threadIdPtr = (int *)_threadId;
	int threadId = *threadIdPtr;
	// Default constructor using thread-id as uniqueId and name LWP-<thread-id>

	VexThreadState *state = new VexThreadState();
	char threadName[64];
	switch(threadId % 3) {
		case 1: sprintf(threadName, "acceleratedThread%d", threadId/3); break;
		case 2: sprintf(threadName, "deceleratedThread%d", threadId/3); break;
		default: sprintf(threadName, "normalThread%d", threadId/3);
	}
	state->setName(threadName);

	AgentTester::onThreadSpawn(state);
//	if (hpcProfiler != NULL) {
//		hpcProfiler->onThreadStart();
//	}

	double e = 0;
//	long long s = Time::getRealTime();//VirtualTime();
	switch(threadId % 3) {
		case 1: e = (new AcceleratedLoopTest())->run(); break;
		case 2: e = (new DeceleratedLoopTest())->run(); break;
		default: e = (new LoopTest())->run();
	}
	cout << state->getName() << ": e = " << e << endl;

//	long long e = Time::getRealTime();//VirtualTime();
//	cout << state->getName() << " vt is " << (e-s) << endl;

//	if (hpcProfiler != NULL) {
//		hpcProfiler->onThreadEnd(state->getName());
//	}
	AgentTester::onThreadEnd(state);

	return NULL;
}


bool SequenceVerification::test() {

	long long start = Time::getRealTime();
	spawnThreadsToExecute(threads, sequenceVerificationThreadRoutine);
	long long finish = Time::getRealTime();
	cout << "Real time: " << (double)(finish - start)/1e9  << endl;
	if (AgentTester::virtualExecution) {
		cout << "VTF time: " << (double)threadEventsBehaviour->getTime()/1e9 << endl;
	}
	return true;
}

std::string SequenceVerification::testName() {
	return "Verify correct virtual scheduling";
}

LoopTest::LoopTest() {

}

LoopTest::~LoopTest() {


}

double LoopTest::calculateE() {
	methodEventsBehaviour->afterMethodEntry(LOOPS_METHOD_ID);
	double temp = 1.0;
	for (int i = 1; i<10000000; i++) {
		temp += pow(1 + 1.0/(double)i, i);
	}
	methodEventsBehaviour->beforeMethodExit(LOOPS_METHOD_ID);
	return temp;
}

double LoopTest::run() {
	methodEventsBehaviour->afterMethodEntry(NORMAL_METHOD_ID);
	double e = calculateE();
	methodEventsBehaviour->beforeMethodExit(NORMAL_METHOD_ID);
	return e;
}

double AcceleratedLoopTest::run() {
	methodEventsBehaviour->afterMethodEntry(ACCELERATED_METHOD_ID);
	double e = calculateE();
	methodEventsBehaviour->beforeMethodExit(ACCELERATED_METHOD_ID);
	return e;
}

double DeceleratedLoopTest::run() {
	methodEventsBehaviour->afterMethodEntry(DECELERATED_METHOD_ID);
	double e = calculateE();
	methodEventsBehaviour->beforeMethodExit(DECELERATED_METHOD_ID);
	return e;
}




int main(int argc, char **argv) {
	if (argc < 3) {
		cout << "Usage ./verifySequence <\"R\" for real execution | VEX options> <threads per category>" << endl;
		return -1;
	}
	if (!Time::onSystemInit()) {
		exit(1);
	}

	if (strcmp(argv[1], "R") != 0) {
		if (!VEX::initializeSimulator(argv[1])) {
			exit(1);
		}
	} else {
		AgentTester::virtualExecution = false;
		methodEventsBehaviour = 0;
	}

	srand(1200);
	int threads = atoi(argv[2]);

	if (AgentTester::virtualExecution) {
		eventLogger->registerMethod("LoopTest::calculateE", LOOPS_METHOD_ID);
		eventLogger->registerMethod("LoopTest::run", NORMAL_METHOD_ID);
		eventLogger->registerMethod("AcceleratedLoopTest::run", ACCELERATED_METHOD_ID);
		eventLogger->registerMethod("DeceleratedLoopTest::run", DECELERATED_METHOD_ID);

		methodEventsBehaviour->registerMethodTimeScalingFactor(ACCELERATED_METHOD_ID, 0.5);
		methodEventsBehaviour->registerMethodTimeScalingFactor(DECELERATED_METHOD_ID, 2.0);
	}
	SequenceVerification *tester = new SequenceVerification(threads);
	tester->test();

	if (AgentTester::virtualExecution) {
		VEX::endSimulator();
		//VEX::printResults();
	}
}

