/*
 * MethodSimulation.cpp
 *
 *  Created on: 12 Oct 2010
 *      Author: root
 */

#include "MethodSimulation.h"
#include <cstdlib>
vector<MethodSimulation *> MethodSimulation::methodsRegistry;
int MethodSimulation::minMethodId;
int MethodSimulation::maxMethodId;
int MethodSimulation::invocationPointsCounter;

int MethodSimulation::ioMethodsCounter;
int MethodSimulation::calculationsCounter;
int MethodSimulation::waitingCounter;
float MethodSimulation::ioMethodsDuration;
float MethodSimulation::calculationsDuration;
float MethodSimulation::waitingDuration;
int MethodSimulation::calculationsPercentage = 25;

MethodSimulation::MethodSimulation(int _methodId, char *_methodName) {
	
	methodId = _methodId;
	methodName = _methodName;

}

MethodSimulation::~MethodSimulation() {
	 
}

void MethodSimulation::setMethodOperationPercentages(int _calcPercentage) {
	calculationsPercentage = _calcPercentage;
}

void MethodSimulation::generateMethodBodyFunctionalities() {
	createSubmethods();
	createOperations();
	createSequenceOfEvents();
}

void MethodSimulation::createSequenceOfEvents() {
	int submethodsSize = subMethods.size();
	int methodOperationsSize = methodOperations.size();
	totalSize = submethodsSize + methodOperationsSize ;
	nextActionIsSubMethod = new bool[totalSize];

	int submethodsCount = 0;
	int methodOperationsCount = 0;

	int totalCount = 0;
	while (totalCount < totalSize) {
		if ((rand() % 10 < 7 && methodOperationsCount < methodOperationsSize) || (submethodsCount==submethodsSize)) {
			nextActionIsSubMethod[totalCount] = false;
			++methodOperationsCount;
		} else {
			nextActionIsSubMethod[totalCount] = true;
			++submethodsCount;
		}
		++totalCount;
	}
}

void MethodSimulation::runMethod() {

	AgentTester::methodEntry(methodId);

	int submethodsCount = 0;
	int operationsCount = 0;

//	cout << "running method " << methodName << endl;
	for (int i = 0; i < totalSize; ++i) {
		if (nextActionIsSubMethod[i]) {
//			cout << "running submethod " << subMethods[submethodsCount]->methodName << endl;
//			for (int l = 0; l<5000000; l++) {	// USED FOR PROFILING
				(subMethods[submethodsCount])->runMethod();
//			}
			++submethodsCount;
		} else {
//			cout << "- op of " << methodName << endl;
			for (int l = 0; l<5000; l++) {	// USED FOR PROFILING
				(methodOperations[operationsCount])->run();
			}
			++operationsCount;
		}
	}

	AgentTester::methodExit(methodId);
}

void MethodSimulation::printMethodsTree() {
	MethodSimulation *mainMethod = MethodSimulation::getMain();
	string result;
	mainMethod->printMethodTree(result, 0);
	cout << result << endl;
}


void MethodSimulation::createMethods(int methodIdFrom, int methodIdTo) {
	clearCounters();

	for (int i = methodIdFrom; i < methodIdTo; i++) {
		char *methodName = new char[64];
		sprintf(methodName, "method%d", i);

		eventLogger->registerMethod(methodName, i);
		methodsRegistry.push_back(new MethodSimulation(i, methodName));
	}

	minMethodId = methodIdFrom;
	maxMethodId = methodIdTo;

	for (int i = methodIdTo; i<(methodIdTo+10); i++) {
		char *methodName = new char[64];
		sprintf(methodName, "ioOperation%d", (i-methodIdTo));
		eventLogger->registerMethod(methodName, i);
	}

	for (int i = methodIdFrom; i < methodIdTo; i++) {
		methodsRegistry[i-methodIdFrom]->generateMethodBodyFunctionalities();
	}
}

MethodSimulation *MethodSimulation::getMain() {
	if (methodsRegistry.size() != 0) {
		return methodsRegistry[0];
	} else {
		return NULL;
	}
}

void MethodSimulation::clearCounters() {
	ioMethodsCounter = 0;
	waitingCounter = 0;
	calculationsCounter = 0;
	ioMethodsDuration=0;
	calculationsDuration =0;
	waitingDuration =0;
}

void MethodSimulation::startMain() {
	MethodSimulation* main = getMain();
	main -> runMethod();
}

void MethodSimulation::printMethod(string &result, int level) {

	string temp = "";
	temp.append(printSpaces(level));
	temp.append(" - ");
	char buf[24];

	sprintf(buf, "%s [%u]\t<", methodName, (unsigned int)methodOperations.size());
	temp.append(buf);
	for (int i = 0 ; i<totalSize; i++) {
		if (nextActionIsSubMethod[i]) {
			temp.append("M");
		} else {
			temp.append("O");
		}
	}

	temp.append(">\n");

	result.insert(0, temp);
}


void MethodSimulation::printMethodTree(string &result, int level) {
	int size = subMethods.size();
	if (size == 0) {
		printMethod(result, level);
	} else {
		for (int i = 0; i<size; i++) {
			subMethods[i]->printMethodTree(result, level+1);
		}
		printMethod(result, level);
	}
}


void MethodSimulation::printStats() {
	cout << "Total io calls: " << ioMethodsCounter <<  " of total duration " << ioMethodsDuration << endl;
	cout << "Total calculations: " << calculationsCounter << " of total duration " << calculationsDuration << endl;
	cout << "Total waits: " << waitingCounter << " of total duration " << waitingDuration<< endl;
	cout << "Expected total serialized time: " << (ioMethodsDuration+calculationsDuration+waitingDuration) << endl;

}
void MethodSimulation::printSubMethods() {

	vector<MethodSimulation *>::iterator it = subMethods.begin();

	cout << "Submethods of " << methodName << endl;
	while (it != subMethods.end()) {
		MethodSimulation *m = (*it);
		cout << " - " << m->methodName << endl;
		++it;
	}
}

void MethodSimulation::printAllSubMethods() {
	vector<MethodSimulation *>::iterator it = methodsRegistry.begin();

	while (it != methodsRegistry.end()) {
		MethodSimulation *m = (*it);
		m->printSubMethods();
		cout << endl;
		++it;
	}
}

string MethodSimulation::printSpaces(int level) {
	string s = "";
	for (int i =0 ; i<level; i++) {
		s.append("\t");
	}
	return s;
}


void MethodSimulation::createSubmethods() {

	int methodsThatCanBeSubmethods = maxMethodId - methodId; 		// the idea is that submethods have >methodIds

	int random = rand() % 100;
	int submethodsProposed = getSubmethodsCount(random);

	if (submethodsProposed > methodsThatCanBeSubmethods) {
		submethodsProposed = methodsThatCanBeSubmethods;
	}

	int currentId = methodId;
	int offset;
	while (currentId < maxMethodId && (submethodsProposed--)>0) {
		if (methodsThatCanBeSubmethods>1) {
			offset = (rand() % ((int)((methodsThatCanBeSubmethods+2)/2)-1)) + 1;
			currentId += offset;
			if (currentId >= minMethodId) {
				subMethods.push_back(methodsRegistry[currentId - minMethodId]);
			}
			methodsThatCanBeSubmethods -= offset;
		}
	}
}

int MethodSimulation::getSubmethodsCount(int random) {
	if (random < 40) {
		return 1;
	} else if (random < 75) {
		return 2;
	} else if (random < 85) {
		return 3;
	} else if (random < 89) {
		return 4;
	} else if (random < 93) {
		return 5;
	} else if (random < 95) {
		return 6;
	} else if (random < 98) {
		return 7 + (rand() % 10);
	} else {
		return 10 + (rand() % 25);
	}
}

//oooooooooooooooo
void MethodSimulation::createOperations() {
	int random = rand() % 100;
	int operationsProposed = getOperationsCount(random);
	float durationInMicroseconds;
	while ((operationsProposed--)>0) {
		random = rand() % 100;

		durationInMicroseconds = (float)(rand() % 1000) / 1000000.0;
		if (random < calculationsPercentage) {
			methodOperations.push_back(new Calculations(durationInMicroseconds));
			++calculationsCounter;
			calculationsDuration += durationInMicroseconds;
//		} else if (random < 94){
//
//			methodOperations.push_back(new WaitOperation(durationInMicroseconds));
//			++waitingCounter;
//			waitingCounter += duration;
		} else {

			methodOperations.push_back(new IoOperation(100000 * durationInMicroseconds, (rand() % 10) + maxMethodId, invocationPointsCounter++));
			++ioMethodsCounter;
			ioMethodsDuration += durationInMicroseconds;
		}
	}
}

int MethodSimulation::getOperationsCount(int random) {
	if (random < 40) {
		return 1;
	} else if (random < 75) {
		return 2;
	} else if (random < 85) {
		return 3;
	} else if (random < 89) {
		return 4;
	} else if (random < 93) {
		return 5;
	} else if (random < 95) {
		return 6;
	} else if (random < 98) {
		return 7 + (rand() % 10);
	} else {
		return 10 + (rand() % 25);
	}
}
