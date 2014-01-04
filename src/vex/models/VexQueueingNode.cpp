/*
 * VexQueueingNode.cpp
 *
 *  Created on: 6 Jun 2011
 *      Author: root
 */

#include "VexQueueingNode.h"
#include "Debug.h"
#include "PostActionNode.h"

#include <string.h>
#include <sstream>

using namespace std;

string VexQueueingNode::getNodeType() {
	stringstream str;
	str << "VexQueueingNode with " << serviceTime->getDelayDistributionSampleType();
	return str.str();
}

//#include <fstream>
//static ofstream tempOutfile("/data/service_times_demo2");
void VexQueueingNode::invokeService( Customer *c ) {
	long long serviceTime = (long long)(c->getServiceDemand() * 1000000000);
	
//	tempOutfile << serviceTime / 1e9 << endl;
//	tempOutfile << c->getThreadName() << " entered " << name << " ("<< getNodeType() <<") at " << c->getEstimatedRealTime() << " and will be waiting remotely for " << (serviceTime/1e9) << " s" << endl;


	//HACK
//	if (name == "DB" || name == "MySQL Server") {
//	cout << c->getThreadName() << " entered " << name << " ("<< getNodeType() <<") and will be waiting remotely for " << (serviceTime/1e6) << " ms" << endl;
//	if (serviceTime < 8000000) {
//cout << "increased " << serviceTime << " To " << 7000000 << endl;
//		serviceTime = 8000000;
//	}
//	cout << c->getThreadName() << " entered " << name << " ("<< getNodeType() <<") and will be waiting remotely for " << (serviceTime/1e6) << " ms" << endl;
//	}
	

	c->lock();
	if (c->waitRemote(serviceTime)) {
		onCompletingService(c);
	} else {
//		cout << c->getThreadName() << " entered " << name << " ("<< getNodeType() <<") and deferred returning immediately for service time: " << (serviceTime/1e6) << " ms" << endl;
		c->setLocation(postServiceAction);
	}
}


void VexQueueingNode::onCompletingQueueing(Customer *c) {
	invokeService(c);
}

void VexQueueingNode::onCompletingService( Customer *c) {
	// Release resource
	releaseResourceAt(c->getEstimatedRealTime());
	c->unlock();
	ResourcePool::forward(c);
}

//
// Customers queue for resources, if there are none available.
// If the queue is full customers are routed to the loss node.
//
void VexQueueingNode::accept( Customer *c ) {
//	cout << name << ": accepting Server" << endl;
	c->lock();
	lock();

//	if (name == "MySQL Server") {
//		cout << name << " " << queue->queueLength() << endl;
//	}
	if ( resources->resourceIsAvailable() ) {
//		cout <<  "Resource claimed by "  << c->getThreadName()  << endl;
		resources->claim() ;
		++uncontendedServiceTimes;
		unlock();
		c->unlock();
		invokeService( c ) ;
	} else {
		if ( queue->canAccept( c ) ) {
//			cout <<  "No resources-> Enqueueing customer..."  << c->getThreadName()  << endl;
			++queueingTimes;
			queue->enqueue( c ) ;
			if (c->blockAtQueue(&mutex)) {	// the service controller
				invokeService( c ) ;
			} else {
				c->setLocation(postQueueingAction);
			}
		} else {
			losses++ ;
//			////cout << "No resources-> Queue full - customer sent to " + lossNode->getId()  << endl;
			lossNode->enter( c ) ;
		}
	}
}

//
// A released resource is allocated to the next queued customer, if
// there is one.
//
void VexQueueingNode::releaseResourceAt(const long long &releaseTime) {

	lock();
	if ( !queue->isEmpty() ) {
		Customer *c = queue->dequeue() ;
//cout << ">>>>>>>>>>>>>>>>>> Resuming to enter contended resource <<<<<<<<<<<< " << endl;
		c->resume(releaseTime);
		unlock();

	} else {
//		cout << "Releasing resource after " << uncontendedServiceTimes << " uncontended customer services and " << queueingTimes << " customer queueings" << endl;
		resources->release();
		unlock();
	}

}


void VexQueueingNode::init( Network *_network, const std::string &s) {
	pthread_mutex_init(&mutex, NULL);
	postQueueingAction = new PostQueueingActionNode(_network, s, this);
	postServiceAction = new PostServiceActionNode(_network, s, this);
	uncontendedServiceTimes = 0;
	queueingTimes = 0;
}

void VexQueueingNode::lock() {
	pthread_mutex_lock(&mutex);
}

void VexQueueingNode::unlock() {
	pthread_mutex_unlock(&mutex);
}
