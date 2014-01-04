/*
 * ThreadManagerTester.cpp
 *
 *  Created on: 13 Oct 2010
 *      Author: root
 */

#include "ThreadManagerTester.h"
#include "ThreadExecutionPattern.h"
#include "ThreadState.h"

#include <cstdlib>
#include <iomanip>
ThreadManagerTester::ThreadManagerTester() {
	

}

ThreadManagerTester::~ThreadManagerTester() {
	 
}


// pass as Args
static int ioRequestsToBeMadePerThread;

static bool isRunner;
static char *patternType;

void *threadRoutine(void *Args) {

	// Default constructor using thread-id as uniqueId and name LWP-<thread-id>
	VexThreadState *state = new VexThreadState();

	ThreadExecutionPattern *pattern;

	if (strcmp(patternType, "startsuspendexit")==0) {
		pattern = new StartSuspendExit(state, 1000000000);

	} else if (strcmp(patternType, "barrier")==0) {
		pattern = new Barrier(state, ioRequestsToBeMadePerThread, false);

		struct sched_param param;
		param.__sched_priority = sched_get_priority_max(SCHED_RR)-1;
		sched_setscheduler(0, SCHED_RR, &param);
	} else if (strcmp(patternType, "timedbarrier")==0) {
		pattern = new Barrier(state, ioRequestsToBeMadePerThread, true);

		struct sched_param param;
		param.__sched_priority = sched_get_priority_max(SCHED_RR)-1;
		sched_setscheduler(0, SCHED_RR, &param);

	} else if (strcmp(patternType, "ioconstant")==0) {
		pattern = new IssuingConstantIo(state, ioRequestsToBeMadePerThread);

	} else if (strcmp(patternType, "iovariable")==0) {
		pattern = new IssuingVariableIo(state, ioRequestsToBeMadePerThread);

	} else if (strcmp(patternType, "iowhileloops")==0) {
		pattern = new IssuingIoWhileRunningLoops(state, ioRequestsToBeMadePerThread, isRunner);
		if (isRunner) {
			isRunner = false;
		} else {
			isRunner = true;
		}
	} else if (strcmp(patternType, "modelexecuting")==0){
		pattern = new ModelRunning(state, false);

	} else if  (strcmp(patternType, "ioandloops")==0) {
		pattern = new IssuingIoAndLoops(state, ioRequestsToBeMadePerThread);

	} else if  (strcmp(patternType, "timedconstwaiting")==0) {
		pattern = new TimedWaiting(state, ioRequestsToBeMadePerThread, false);

	} else if  (strcmp(patternType, "timedwaiting")==0) {
		pattern = new TimedWaiting(state, ioRequestsToBeMadePerThread, true);

	} else {
		cout << "Pattern type " << patternType << " not found" << endl;
		return NULL;

	}

	if (!AgentTester::isVTFenabled() && pattern->isFunctional()) {
		cout << "Warning: real time execution with functional test - no meaningful results expected" << endl;
	}

	AgentTester::onThreadSpawn(state);
	AgentTester::methodEntry(101);

	pattern -> apply();

	AgentTester::methodExit(101);
	AgentTester::onThreadEnd(state);


	return NULL;
}

bool ThreadManagerTester::test() {

	long long start = Time::getRealTime();
	long long startInVT = 0, endInVT = 0;
	if (AgentTester::virtualExecution) {
		startInVT = threadEventsBehaviour->getTime();
		startVTFManager();
	}

	spawnThreadsToExecute(threads, threadRoutine);

	if (AgentTester::virtualExecution) {
		endInVT = threadEventsBehaviour->getTime();
		endVTFManager();
	}
	long long finish = Time::getRealTime();
	cout << "Real time: " << ((finish - start)) << endl;
	if (AgentTester::virtualExecution) {
		cout << "VTF time: " << (endInVT - startInVT) << endl;
	}
	return true;

}

string ThreadManagerTester::testName() {
	return "ThreadManagerTester";
}


void ThreadManagerTester::printTests() {
	cout << "Available tests (<type>)" << endl;

	cout << left << setw(19) << "barrier" << "(performance) threads-1 threads wait forever, while one wakes them up. This is repeated <requests> times" << endl;
	cout << left << setw(19) << "timedbarrier" << "(performance) threads-1 threads wait for at most 1sec while one wakes them up. This is repeated <requests> times" << endl;
	cout << left << setw(19) << "startsuspendexit" << "(functional) start, suspend a random number of times until 1sec of VT is reached per thread and then exit" << endl;
	cout << left << setw(19) << "ioconstant" << "(performance) each thread performs 20ms of calcs and then simulates <requests> 20ms I/O requests" << endl;
	cout << left << setw(19) << "iovariable" << "(performance) each thread performs the same <requests> random I/O requests from 2-35ms or the ones read from <file with io durations>" <<endl;
	cout << left << setw(19) << "" << "If <fill I/O buffer>=Y, then the buffer will be filled with requests up to the maximum random I/O duration" << endl;
	cout << left << setw(19) << "ioandloops" << "(performance) same as iovariable, but every I/O is prepended by a random busy-waiting CPU calc" << endl;
	cout << left << setw(19) << "iowhileloops" << "(performance) half the threads perform busy-waiting, while the other half perform random I/O operations" << endl;
	cout << left << setw(19) << "modelexecuting" << "(functional) start, register a performance model, then start a method being described by the model" << endl;
	cout << left << setw(19) << "timedconstwaiting" << "(functional) start, wait for '<requests>' nsec and then exit" << endl;
	cout << left << setw(19) << "timedwaiting" << "(functional) start, wait for random durations until '<requests>' nsec of VT are reached per thread and then exit" << endl;
}



/*
 * Direct testing of the ThreadManager class methods
 */
int main(int argc, char **argv) {

	if (argc < 4) {
		cout << "Usage ./tmt <options> <type> <threads> <requests> <fill io buffer(Y|N)> [<file with io durations>]]" << endl;
		ThreadManagerTester::printTests();
		return -1;
	}
	if (!Time::onSystemInit()) {
		exit(1);
	}

	if (strcmp(argv[1], "R") != 0) {
		Tester::enableVTF();
		if (!VEX::initializeSimulator(argv[1])) {
			return -1;
		}
	} else {
		Tester::disableVTF();
	}

	patternType = argv[2];

	int threads = atoi(argv[3]);
	ThreadExecutionPattern::setThreads(threads);
	ioRequestsToBeMadePerThread = atoi(argv[4]);
	Barrier::init();

	if (argc == 6 && ioRequestsToBeMadePerThread < 10000) {
		IssuingVariableIo::createRandomRequests(ioRequestsToBeMadePerThread);
	}
	if (argc == 7) {
		IssuingVariableIo::readRequestsFromFile(argv[6]);
	}

//	if (strcmp(argv[5],"Y")==0) {
//		ioSimulator->prefillBuffers(IssuingVariableIo::getMaxIoDuration(), threads, 20);
//	}
	isRunner = false;


	if (eventLogger) {
		eventLogger -> registerMethod("main", 101);
		eventLogger -> registerMethod("ioMethod", 21);
	}
	ThreadManagerTester *tester = new ThreadManagerTester(threads);
	tester->test();

}
