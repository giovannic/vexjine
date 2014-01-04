/*
 * VexSchedSimQueueingNetwork.h
 *
 *  Created on: 28 Jun 2011
 *      Author: root
 */

#ifndef VEXSCHEDSIMQUEUEINGNETWORK_H_
#define VEXSCHEDSIMQUEUEINGNETWORK_H_

#include "VexQueueingNetwork.h"

class VexSchedSimQueueingNetwork : public VexQueueingNetwork {
public:
	VexSchedSimQueueingNetwork() : VexQueueingNetwork() {};
	VexSchedSimQueueingNetwork(const int &totalNodes) : VexQueueingNetwork(totalNodes) {};

	virtual void simulate(VexThreadState *state, MethodPerformanceModelMetadata *methodMetadata);

	virtual void enterNode( Customer *c, CinqsNode *n );

	virtual ~VexSchedSimQueueingNetwork();

};

#endif /* VEXSCHEDSIMQUEUEINGNETWORK_H_ */
