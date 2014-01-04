/*
 * VexLocalQueueingNode.cpp
 *
 *  Created on: 28 Jun 2011
 *      Author: root
 */

#include "VexLocalQueueingNode.h"
#include "Debug.h"
#include "PostActionNode.h"

#include <string.h>

using namespace std;

string VexLocalQueueingNode::getNodeType() {
	stringstream str;
	str << "Local VexLocalQueueingNode with " << serviceTime->getDelayDistributionSampleType();
	return str.str();
}


void VexLocalQueueingNode::invokeService( Customer *c ) {
	long long serviceTime = (long long)(c->getServiceDemand() * 1000000000);

//	cout << c->getThreadName() << " entered " << name << " ("<< getNodeType() <<") and will be serviced locally for " << (serviceTime/1e6) << " ms" << endl;

	if (c->service(serviceTime)) {	//locking inside service() to update GVT
		onCompletingService(c);
	} else {
		c->setLocation(postServiceAction);
	}

//	if (c->setLocation(this)) {
//		//	c->lock();
//		releaseResourceAt(c->getEstimatedRealTime());
//		c->unlock();
//		ResourcePool::forward(c);
//	} else {
//		c->unlock();
//	}


}
