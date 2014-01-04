/*
 * ModelHandler.h
 *
 *  Created on: 28 Jun 2011
 *      Author: root
 */

#ifndef MODELHANDLER_H_
#define MODELHANDLER_H_

class CinqsNode;
class Customer;

class ModelHandler {
public:
	ModelHandler();
	virtual ~ModelHandler();

	CinqsNode *getCurrentNode() {
		return currentNode;
	}

	void setCurrentNode(CinqsNode *node) {
		currentNode = node;
	}

	bool resumeModelSimulation();	// returns true when the model is exited
	long long simulateLocalServiceTime(const long long &remainingTime);	// returns how much time was simulated

	void initiateLocalResourceConsumption(const long long &totalTime);

	void setCustomer(Customer *customer) {
		modelCustomer = customer;
	}
protected:
	CinqsNode *currentNode;
	Customer *modelCustomer;

	long long remainingLocalResourceConsumption;

};

#endif /* MODELHANDLER_H_ */
