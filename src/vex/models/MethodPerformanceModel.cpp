/*
 * MethodPerformanceModel.cpp
 *
 *  Created on: 25 May 2011
 *      Author: root
 */

#include "MethodPerformanceModel.h"
#include "cinqs.h"
#include "JmtNetworkParser.h"
#include "VexQueueingNetwork.h"
#include "ThreadState.h"
#include <cassert>

MethodPerformanceModel::MethodPerformanceModel(const char *modelFilename, const bool &useSchedSimNetwork) {
	Sim::initialize();
	queueingNetworkModel = JmtNetworkParser::parseNetwork(modelFilename, false, useSchedSimNetwork);
	active = true;
}

MethodPerformanceModel::~MethodPerformanceModel() {
	if (queueingNetworkModel != NULL) {
		delete queueingNetworkModel;
	}
}

void MethodPerformanceModel::enableSimulation() {
	active = true;
}
void MethodPerformanceModel::disableSimulation() {
	active = false;
}
bool MethodPerformanceModel::isActive() {
	return active;
}

CinqsNode *MethodPerformanceModel::getSourceNodeWithLabel(const char *sourceNodeLabel) {
	//find the source node that corresponds to this method
	if (queueingNetworkModel != NULL) {
		return queueingNetworkModel->getNetworkSourceWithLabel(sourceNodeLabel);
	} else {
		assert(false);
	}
	return NULL;
}


void MethodPerformanceModel::simulate(VexThreadState *state, MethodPerformanceModelMetadata *methodMetadata) {
	if (active && queueingNetworkModel != NULL) {
		queueingNetworkModel->simulate(state, methodMetadata);
	}
}
