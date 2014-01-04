/*
 * VexSchedSimQueueingNetwork.cpp
 *
 *  Created on: 28 Jun 2011
 *      Author: root
 */

#include "VexSchedSimQueueingNetwork.h"
#include "VexSchedSimCustomer.h"
#include "MethodPerformanceModel.h"

#include <iostream>
using namespace std;

VexSchedSimQueueingNetwork::~VexSchedSimQueueingNetwork() {

}

/*
 * This method simulates the model, by updating the current node information of the thread
 * state and scheduling new future events in virtual time. The actual thread that has
 * invoked the model simulation is not involved (so may run real code).
 */
void VexSchedSimQueueingNetwork::simulate(VexThreadState *state, MethodPerformanceModelMetadata *methodMetadata) {
	Customer *customer = VexQueueingNetwork::threadCustomer;
	if (customer == NULL) {
		// The only safe way to allocate memory at a place where the scheduler might interrupt. WHY?
		state->lockShareResourceAccessKey();
		VexQueueingNetwork::threadCustomer = new VexSchedSimCustomer(state);
		state->unlockShareResourceAccessKey();
		customer = VexQueueingNetwork::threadCustomer;
	}

	// TODO: FIX: add delay here to check problem
	customer->setclass(methodMetadata->getclass());

	state->lockShareResourceAccessKey();
	state->setInModelSimulation(methodMetadata->getSourceNode(), customer);
	state->setInModelSimulation();	// might have been suspended since the time when it was set to IN_MODEL state

	// Model will be simulated by the thread scheduler
	state->startSchedulerControlledModelSimulation();
	state->getThreadCurrentlyControllingManager()->setInModelSimulation(state, false);
	state->unlockShareResourceAccessKey();
}


void VexSchedSimQueueingNetwork::enterNode( Customer *c, CinqsNode *n ) {
	c->setLocation(n);
}
