/*
 * MethodSimulation.h
 *
 *  Created on: 12 Oct 2010
 *      Author: root
 */

#ifndef METHODSIMULATION_H_
#define METHODSIMULATION_H_

#include "AgentTester.h"
#include "MethodOperations.h"
#include <vector>
#include <map>
#include <string>

using namespace std;

class MethodSimulation {
public:
	MethodSimulation(int _methodId, char *_methodName);
	virtual ~MethodSimulation();

	void printMethodTree(string &result, int level);
	void runMethod();

	static void createMethods(int methodIdFrom, int methodIdTo);

	static void printAllSubMethods();
	static void printMethodsTree();
	static void startMain();
	static void printStats();

	static void setMethodOperationPercentages(int calcPerc);
private:
	static MethodSimulation *getMain();
	static void clearCounters();

	int getSubmethodsCount();
	void printMethod(string &result, int level);
	void generateMethodBodyFunctionalities();
	void createSequenceOfEvents();
	void printSubMethods();


	void createSubmethods();
	void createOperations();
	int getSubmethodsCount(int random);
	int getOperationsCount(int random);

	string printSpaces(int level);

	vector<MethodSimulation *> subMethods;
	vector<MethodOperations *> methodOperations;

	bool *nextActionIsSubMethod;
	int totalSize;

	int methodId;
	char *methodName;

	static int maxMethodId;
	static int minMethodId;
	static int invocationPointsCounter;
	static vector<MethodSimulation *> methodsRegistry;

	static int ioMethodsCounter;
	static int calculationsCounter;
	static int waitingCounter;
	static float ioMethodsDuration;
	static float calculationsDuration;
	static float waitingDuration;
	static int calculationsPercentage;

};


#endif /* METHODSIMULATION_H_ */
