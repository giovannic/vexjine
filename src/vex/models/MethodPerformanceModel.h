/*
 * MethodPerformanceModel.h
 *
 *  Created on: 25 May 2011
 *      Author: root
 */

#ifndef METHODPERFORMANCEMODEL_H_
#define METHODPERFORMANCEMODEL_H_


class CinqsNode;
class VexThreadState;
class MethodPerformanceModelMetadata;
class VexQueueingNetwork;

/***
 * Class denoting the customer class and the source for this method invocation
 */
class MethodPerformanceModelMetadata {
public:
	MethodPerformanceModelMetadata(CinqsNode *_sourceNode, const int &_customerClass) {
		sourceNode = _sourceNode;
		customerClass = _customerClass;
	}

	CinqsNode *getSourceNode() {
		return sourceNode;
	}

	int const &getclass() {
		return customerClass;
	}
private:
	CinqsNode *sourceNode;
	int customerClass;
};


class MethodPerformanceModel {
public:
	MethodPerformanceModel(const char *modelFilename, const bool &useSchedSimNetwork);
	virtual ~MethodPerformanceModel();

	void simulate(VexThreadState *state, MethodPerformanceModelMetadata *methodMetadata);

	void enableSimulation();
	void disableSimulation();
	bool isActive();

	CinqsNode *getSourceNodeWithLabel(const char *sourceNodeLabel);	//find the source node that corresponds to this method
private:
	int methodId;
	bool active;
	VexQueueingNetwork *queueingNetworkModel;
};


#endif /* METHODPERFORMANCEMODEL_H_ */
