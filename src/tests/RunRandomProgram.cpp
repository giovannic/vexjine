/*
 * RunRandomProgram.cpp
 *
 *  Created on: 12 Oct 2010
 *      Author: root
 */

#include "RunRandomProgram.h"

#include <iostream>
#include <cstdlib>

#include "ThreadState.h"

using namespace std;

RunRandomProgram::~RunRandomProgram() {

}

string RunRandomProgram::testName() {
	return "Random program testing";
}


/*
 * Each thread start the random program from main
 */
void *randomProgramThreadRoutine(void *threadId) {

	// Default constructor using thread-id as uniqueId and name LWP-<thread-id>
	VexThreadState *state = new VexThreadState();
	AgentTester::onThreadSpawn(state);
	MethodSimulation::startMain();
	AgentTester::onThreadEnd(state);

	return NULL;
}


bool RunRandomProgram::test() {

	threadEventsBehaviour->onThreadMainStart(12321);
	threadEventsBehaviour->onWrappedWaitingStart();
	long long start = Time::getRealTime();
	spawnThreadsToExecute(threads, randomProgramThreadRoutine);
	long long finish = Time::getRealTime();
	cout << (double)(finish - start)/1e9  << " - Real time" << endl;
	threadEventsBehaviour->onWrappedWaitingEnd();
	threadEventsBehaviour->onEnd();

	cout << (double)threadEventsBehaviour->getTime()/1e9 << " - VTF time: " << endl;


	return true;
}


struct IllegalArgumentException : public std::exception {
	const char *what() {return "Wrong arguments";}
};

/*
 * Testing program for random program
 */
int main(int argc, char **argv) {
	if (argc < 6) {
		cout << "Usage ./rr <VTF options> <random methods #> <threads #> <CPU calc % [0-100]> <Profile only main: [Y|N]> [<set to enable REAL time execution>]" << endl;
		for (int i =0 ; i<argc; i++) {
			cout << i << " " << argv[i] << endl;
		}
		return -1;
	}

	if (!Time::onSystemInit()) {
		exit(1);
	}

	if (!VEX::initializeSimulator(argv[1])) {
		exit(1);
	}

	if (argc < 7) {
		Tester::enableVTF();

	} else {
		Tester::disableVTF();
	}

	srand(1200);
	int methods = atoi(argv[2]);
	int threads = atoi(argv[3]);
	int calculationsPercent = atoi(argv[4]);
	if (strcmp(argv[5], "Y") == 0) {
		AgentTester::profileOnlySelected();
	}

	if (methods <= 0 || threads <= 0 || calculationsPercent < 0 || calculationsPercent > 100 || ((strcmp(argv[5], "Y") != 0) && strcmp(argv[5], "N") != 0)) {
		throw IllegalArgumentException();
	}
	MethodSimulation::setMethodOperationPercentages(calculationsPercent);
	MethodSimulation::createMethods(100, 100+methods);
//	MethodSimulation::printMethodsTree();
//	MethodSimulation::printStats();

	RunRandomProgram *tester = new RunRandomProgram(threads);
	tester->test();

	VEX::endSimulator();
}
