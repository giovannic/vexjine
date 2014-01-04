/*
 * VexQueueingNetwork.h
 *
 *  Created on: 26 May 2011
 *      Author: root
 */

#ifndef VEXQUEUEINGNETWORK_H_
#define VEXQUEUEINGNETWORK_H_

#include "Network.h"
#
class VexThreadState;
class MethodPerformanceModelMetadata;

class VexQueueingNetwork : public Network {
public:
	VexQueueingNetwork() : Network(), networkSource(NULL) {};
	VexQueueingNetwork(const int &totalNodes) : Network(totalNodes), networkSource(NULL) {};

	int add( CinqsNode *n );

	CinqsNode *getNetworkSource();
	CinqsNode *getNetworkSourceWithLabel(const char *sourceCodeLabel);
	virtual void simulate(VexThreadState *state, MethodPerformanceModelMetadata *methodMetadata);

	virtual ~VexQueueingNetwork();

protected:
	CinqsNode *networkSource;

	static __thread Customer *threadCustomer;
};

#endif /* VEXQUEUEINGNETWORK_H_ */
