/*
 * ModelHandler.cpp
 *
 *  Created on: 28 Jun 2011
 *      Author: root
 */
#include "ModelHandler.h"
#include "cinqs.h"
//#include "Node.h"


ModelHandler::ModelHandler() {
	remainingLocalResourceConsumption = 0;
}

ModelHandler::~ModelHandler() {

}

bool ModelHandler::resumeModelSimulation() {
	currentNode->enter(modelCustomer);
	if (modelCustomer->isFinished()) {
//		delete modelCustomer;
//		modelCustomer = NULL;
		modelCustomer->clearFinishedFlag();
		return true;
	}

	return false;
}

long long ModelHandler::simulateLocalServiceTime(const long long &remainingTimeslotTime) {
	// No local service time defined
	if (remainingLocalResourceConsumption == 0) {
		return 0;

	} else {
		// The remaining service time can fit in the remainingTimeslot
		if (remainingLocalResourceConsumption <= remainingTimeslotTime) {
			long long lastBitRemaining = remainingLocalResourceConsumption;
			remainingLocalResourceConsumption = 0;
			return lastBitRemaining;
		} else {
			// Next time you will simulate the rest
			remainingLocalResourceConsumption -= remainingTimeslotTime;
			return remainingTimeslotTime;
		}
	}
}

void ModelHandler::initiateLocalResourceConsumption(const long long &totalTime) {
	remainingLocalResourceConsumption = totalTime;
}
