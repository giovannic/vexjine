/*
 * ForwardLeapTest.cpp: Thic class is used to test, if forward leaps in virtual time
 * occur if and only if they should according to the Rules and Estimations of VEX.
 *
 *  Created on: 21 Nov 2011
 *      Author: root
 */

#include "ForwardLeapTest.h"
#include <cassert>
#include "ThreadState.h"
ForwardLeapTest::ForwardLeapTest() {

}

bool ForwardLeapTest::allow(VexThreadState *state) {
	cout << " ---> ALLOW LEAP" << endl;
	return manager->testIsValidToLeapInVirtualTimeTo(state);
}

bool ForwardLeapTest::forbid(VexThreadState *state) {
	cout << " ---> FORBID LEAP" << endl;
	return !manager->testIsValidToLeapInVirtualTimeTo(state);
}

ForwardLeapTest::~ForwardLeapTest() {

}


void StateCheckingForwardLeapTest::run() {

	VexThreadState *state = VexThreadState::getCurrentThreadState();
	// TEST 1: new thread - unregistered
	cout << "Test: new thread - unregistered" << endl;
	cout << "===============================" << endl;
	state->setTimedWaiting(50000000);
	cout << "(GVT " << threadEventsBehaviour->getTime()/1e6 << "): " << *state << endl;
	cout << "\t...before increasing new thread counter" ;
	assert(allow(state));

//	cout << "\t...after increasing new thread counter" ;
//	registry->newThreadBeingSpawned();
//	assert(forbid(state));

//	cout << "\t...after decreasing new thread counter" ;
//	registry->newThreadStarted();
//	assert(allow(state));
//	cout << endl;

	// TEST 2: new thread - registered before
	cout << "Test: new thread - registered in lower ERT" << endl;
	cout << "==========================================" << endl;
	VexThreadState *stateNewBefore = new StackTraceThreadState(12522, (char *)"testingThread");
	stateNewBefore -> setEstimatedRealTime(20000000);
	stateNewBefore -> setSuspended();
	cout << "(GVT " << threadEventsBehaviour->getTime()/1e6 << "): " << *state << " BUT " << *stateNewBefore << endl;

//	cout << "\t...after increasing new thread counter and in registry before" ;
//	registry->newThreadBeingSpawned();
//	registry->add(stateNewBefore);
//	assert(forbid(state));
//	cout << endl;


	// TEST 3: new thread - registered after
	cout << "Test: new thread - registered in higher ERT" << endl;
	cout << "==========================================" << endl;
	stateNewBefore->setEstimatedRealTime(80000000);
	cout << "(GVT " <<threadEventsBehaviour->getTime()/1e6 << "): " << *state << " BUT " << *stateNewBefore << endl;
	cout << "\t...after increasing new thread counter and in registry after" ;
	assert(allow(state));

	cout << endl;


	// TEST 4: difference to GVT
//	cout << "Test: comparison to GVT" << endl;
//	cout << "==========================================" << endl;
//	virtualTimeline->leapForwardTo(state->getEstimatedRealTime());
//	state->setTimedWaiting(5000000);
//	cout << "(GVT " <<threadEventsBehaviour->getTime()/1e6 << "): " << *state << endl;
//	cout << "\t...when leap time is less than a timeslice different from GVT" ;
//	assert(allow(state));
//	virtualTimeline->reset();
//	cout << endl;


	// TEST 5: Non-blocking I/O threads
	cout << "Test: Learning non-blocking I/O thread" << endl;
	cout << "======================================" << endl;
	VexThreadState *stateLearningIo = stateNewBefore;

	stateLearningIo -> setEstimatedRealTime(20000000);
	stateLearningIo -> onIoEntry(false);	// blocking = false
	stateLearningIo -> setLearningIo();

	cout << "(GVT " <<threadEventsBehaviour->getTime()/1e6 << "): " << *state << " BUT " << *stateLearningIo << endl;
	cout << "\t...when a thread is learning non-blocking I/O in lower ERT" ;
	assert(forbid(state));

	stateLearningIo -> setEstimatedRealTime(80000000);
	cout << "\t...when a thread is learning non-blocking I/O in higher ERT" ;
	assert(allow(state));
	cout << endl;

	// TEST 6: Blocking I/O threads
	cout << "Test: Learning blocking I/O thread" << endl;
	cout << "===================================" << endl;
	stateLearningIo -> setEstimatedRealTime(20000000);
	stateLearningIo -> onIoEntry(true);	// blocking = false
	stateLearningIo -> setLearningIo();

	cout << "(GVT " <<threadEventsBehaviour->getTime()/1e6 << "): " << *state << " BUT " << *stateLearningIo << endl;
	stateLearningIo -> setLastRealTime(Time::getRealTime() - 600000000);
	cout << "\t...when a thread is blocking I/O in lower ERT and has been so for more than 5sec" ;
	assert(allow(state));

	stateLearningIo -> setLastRealTime(Time::getRealTime() - 10000);
	cout << "\t...when a thread is blocking I/O in lower ERT and has been so for less than 5sec" ;
	assert(forbid(state));

	stateLearningIo -> setEstimatedRealTime(80000000);
	stateLearningIo -> setLastRealTime(Time::getRealTime() - 600000000);
	cout << "\t...when a thread is blocking I/O in higher ERT and has been so for more than 5sec" ;
	assert(allow(state));
	stateLearningIo -> setLastRealTime(Time::getRealTime() - 10000);
	cout << "\t...whethreadEventsBehaviour->getTime()n a thread is blocking I/O in higher ERT and has been so for less than 5sec" ;
	assert(allow(state));
	stateLearningIo -> onIoEntry(false);	// blocking = false
	cout << endl;


	// TEST 7: Native waiting threads
	cout << "Test: Native waiting thread" << endl;
	cout << "===================================" << endl;
	VexThreadState *nativeWaitingThread = stateLearningIo;
	nativeWaitingThread -> setEstimatedRealTime(80000000);
	nativeWaitingThread -> setNativeWaiting();
	nativeWaitingThread -> setLastRealTime(Time::getRealTime() - 600000000);

	cout << "(GVT " <<threadEventsBehaviour->getTime()/1e6 << "): " << *state << " BUT " << *nativeWaitingThread << endl;
	cout << "\t...when a thread is native waiting in higher ERT" ;
	assert(allow(state));

	long long remainingTime = state->getEstimatedRealTime() -threadEventsBehaviour->getTime();
	nativeWaitingThread -> setEstimatedRealTime(state->getEstimatedRealTime() - 10000000);
	state -> setLastRealTime(Time::getRealTime() - 10000000);
	cout << "\t...when a thread is native waiting in 10ms lower ERT and RT diff is 10ms" ;
	assert(forbid(state));

	state -> setLastRealTime(Time::getRealTime() - remainingTime - 1);
	cout << "\t...when a thread is native waiting in 10ms lower ERT and RT diff is " << (remainingTime + 1)/1e6 << "ms" ;
	assert(allow(state));

//	virtualTimeline->leapForwardTo(state->getEstimatedRealTime());
	nativeWaitingThread -> setEstimatedRealTime(state->getEstimatedRealTime());

	state -> setTimedWaiting(24000000000);
	remainingTime = state->getEstimatedRealTime() -threadEventsBehaviour->getTime();
	state -> setLastRealTime(Time::getRealTime() - remainingTime - 1);
	cout << "\t...when a thread is native waiting in 24sec lower ERT and RT diff is " << (remainingTime + 1)/1e9 << "sec" ;
	assert(forbid(state));

	state -> setLastRealTime(Time::getRealTime() - ((double)log10(state->getEstimatedRealTime()-nativeWaitingThread->getEstimatedRealTime()) - 7) * remainingTime - 1);
	cout << "\t...when a thread is native waiting in 24sec lower ERT and RT diff is " << ((double)(log10(state->getEstimatedRealTime()-nativeWaitingThread->getEstimatedRealTime()) - 7) * remainingTime + 1)/1e9 << "sec" ;
	assert(allow(state));


//	virtualTimeline->leapForwardTo(state->getEstimatedRealTime());
	nativeWaitingThread -> setEstimatedRealTime(state->getEstimatedRealTime());

	state -> setTimedWaiting(92000000000);
	remainingTime = state->getEstimatedRealTime() -threadEventsBehaviour->getTime();
	state -> setLastRealTime(Time::getRealTime() - remainingTime - 1);
	cout << "\t...when a thread is native waiting in 92sec lower ERT and RT diff is " << (remainingTime + 1)/1e9 << "sec" ;
	assert(forbid(state));

	state -> setLastRealTime(Time::getRealTime() - (log10(state->getEstimatedRealTime()-nativeWaitingThread->getEstimatedRealTime()) - 7) * remainingTime - 1);
	cout << "\t...when a thread is native waiting in 92sec lower ERT and RT diff is " << ((double)(log10(state->getEstimatedRealTime()-nativeWaitingThread->getEstimatedRealTime()) - 7) * remainingTime + 1)/1e9 << "sec" ;
	assert(allow(state));


	cout << endl;






//	registry->remove(nativeWaitingThread);
//	registry->cleanup(nativeWaitingThread);
	state->setRunning();
}

int main(int argc, char **argv) {

	char options[128];
	strcpy(options, "file=options_lftest");
	VEX::initializeSimulator(options);

	eventLogger->registerMethod("main", 100);

	ForwardLeapTest *fltest = new StateCheckingForwardLeapTest();

	threadEventsBehaviour->onThreadMainStart(12521);
	methodEventsBehaviour->afterMethodEntry(100);

	fltest->run();

	methodEventsBehaviour->beforeMethodExit(100);

	return 0;
}
