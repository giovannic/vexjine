/*
 * VexQueueingNetwork.cpp
 *
 *  Created on: 26 May 2011
 *      Author: root
 */

#include "VexQueueingNetwork.h"
#include "VexThreadCustomer.h"
#include "ThreadState.h"
#include "cinqs.h"
#include "MethodPerformanceModel.h"

#include <iostream>
using namespace std;

__thread Customer *VexQueueingNetwork::threadCustomer = NULL;

VexQueueingNetwork::~VexQueueingNetwork() {

}

int VexQueueingNetwork::add( CinqsNode *n ) {
	nodes[ nodeCount++ ] = n ;
	return nodeCount-1 ;
}

// Version where all methods share always the same source node
CinqsNode *VexQueueingNetwork::getNetworkSource() {
	if (networkSource == NULL) {
		for (int i = 0; i<nodeCount ; i++) {
			if (nodes[i]->getNodeType().compare("Source") == 0) {
				networkSource = nodes[i];
				break;
			}
		}
	}
	return networkSource;
}

// Version where each method may have its own source code
CinqsNode *VexQueueingNetwork::getNetworkSourceWithLabel(const char *sourceCodeLabel) {
	if (sourceCodeLabel != 0) {
		string sourceCodeLabelString(sourceCodeLabel);
		for (int i = 0; i<nodeCount ; i++) {
			if (nodes[i]->getNodeType().compare("Source") == 0 && nodes[i]->getName() == sourceCodeLabelString) {
				return nodes[i];
			}
		}

	}
	return (Source *)getNetworkSource();
}

/*
 * This method simulates the model, by making the thread that invoked
 * the model simulation execute the model itself.
 */
void VexQueueingNetwork::simulate(VexThreadState *state, MethodPerformanceModelMetadata *methodMetadata) {
	VexThreadCustomer *customer = (VexThreadCustomer *)VexQueueingNetwork::threadCustomer;	// static __thread for each thread
	if (customer == NULL) {
		// The only safe way to allocate memory at a place where the scheduler might interrupt
		// Deadlocks can occur if a thread is holding a memory related kernel lock
		state->lockShareResourceAccessKey();
		VexQueueingNetwork::threadCustomer = new VexThreadCustomer(state);
		state->unlockShareResourceAccessKey();
		customer = (VexThreadCustomer *)VexQueueingNetwork::threadCustomer;
	}

	customer->setclass(methodMetadata->getclass());
	CinqsNode *sourceNode = methodMetadata->getSourceNode();
	state->setInModelSimulation(sourceNode, customer);

	// Enter and parse the entire network (enter->accept->forward->move) simulating your own behaviour in virtual time
	sourceNode->enter(customer);

	// Set the thread state in model, after completing the simulation
	state->lockShareResourceAccessKey();
	state->getThreadCurrentlyControllingManager()->setInModelSimulation(state, true);
	state->unlockShareResourceAccessKey();
}

