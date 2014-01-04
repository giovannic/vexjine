/*
 * VexLocalInfiniteServerNode.cpp
 *
 *  Created on: 28 Jun 2011
 *      Author: root
 */

#include "VexLocalInfiniteServerNode.h"
#include "PostActionNode.h"

#include <iostream>
#include <sstream>
using namespace std;

VexLocalInfiniteServerNode::~VexLocalInfiniteServerNode() {

}


string VexLocalInfiniteServerNode::getNodeType() {
	stringstream str;
	str << "Local VexInfiniteServerNode with " << serviceTime->getDelayDistributionSampleType();
	return str.str();
}

/*
 * Local InfiniteServerNode resource consumption needs to be serviced,
 * in a way to disallow other thread to occupy the executing CPU at the same time.
 * In the model_scheduler_sim version the forward will occur immediately
 * despite the fact that the actual leap will occur only after the "duration"
 * expires after one or more timeslots
 */
void VexLocalInfiniteServerNode::accept( Customer *c ) {
	long duration = (long long)(c->getServiceDemand() * 1000000000);

//	cout << c->getThreadName() << " entered " << name << " ("<< getNodeType() <<") and will be serviced locally for " << (duration/1e6) << " ms" << endl;

	if (c->service(duration)) {
		onCompletingService(c);
	} else {
		c->setLocation(postServiceAction);
	}
}

