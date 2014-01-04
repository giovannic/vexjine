/*
 * VexInfiniteServerNode.cpp
 *
 *  Created on: 26 May 2011
 *      Author: root
 */

#include "VexInfiniteServerNode.h"
#include "PostActionNode.h"

#include <iostream>
#include <sstream>
using namespace std;
VexInfiniteServerNode::~VexInfiniteServerNode() {

}

std::string VexInfiniteServerNode::getNodeType() {
	stringstream str;
	str << "VexInfiniteServerNode with " << serviceTime->getDelayDistributionSampleType();
	return str.str();
}

/*
 * Remote InfiniteServerNode resource consumption is merely a timed-wait
 */
void VexInfiniteServerNode::accept( Customer *c ) {
	long duration = (long long)(c->getServiceDemand() * 1000000000);

//	cout << c->getThreadName() << " for " << (duration/1e6) << " ms" << endl;
//	cout << c->getThreadName() << " entered " << name << " ("<< getNodeType() <<") and will be waiting remotely for " << (duration/1e6) << " ms" << endl;

	c->lock();
	if (c->waitRemote(duration)) {
		onCompletingService(c);
	} else {
		c->setLocation(postServiceAction);
	}

}


void VexInfiniteServerNode::init( Network *_network, const std::string &s) {
	postServiceAction = new PostActionInfiniteServer(_network, s, this);

}

void VexInfiniteServerNode::onCompletingService(Customer *c) {
	c->unlock();
	forward(c);
}
