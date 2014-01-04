#include "network/ResourcePool.h"

#include "Debug.h"
#include "Logger.h"

#include <sstream>

using namespace std;

ResourcePool::~ResourcePool() {
	delete queue;
	delete resources;
}

void ResourcePool::setLossNode( CinqsNode *n ) {
	lossNode = n ;
}

string ResourcePool::getNodeType() {
	return "ResourcePool";
}

string ResourcePool::toString() {
	stringstream str;
	str << name << ", resources = " << resources->numberOfAvailableResources() << ", queue length = " << queue->queueLength();
	return  str.str();
}

//
// Customers queue for resources, if there are none available.
// If the queue is full customers are routed to the loss node.
//
void ResourcePool::accept( Customer *c ) {
	if ( resources->resourceIsAvailable() ) {
		Debug::trace( "Resource claimed" ) ;
		resources->claim() ;
		invokeService( c ) ;
	} else {
		if ( queue->canAccept( c ) ) {
			Debug::trace( "No resources-> Enqueueing customer..." ) ;
			queue->enqueue( c ) ;
		} else {
			losses++ ;
			Debug::trace( "No resources-> Queue full - customer sent to " + lossNode->getId() ) ;
			lossNode->enter( c ) ;
		}
	}
}

//
// A released resource is allocated to the next queued customer, if
// there is one.
//
void ResourcePool::releaseResource() {
	Debug::trace( toString() + " releasing resource" ) ;
	if ( !queue->isEmpty() ) {
		Customer *c = queue->dequeue() ;
		invokeService( c ) ;
	} else {
		resources->release();
	}
}

//
// Useful additional methods
//

int ResourcePool::queueLength() {
	//between this and a resource pool is that it releases
	return queue->queueLength() ;
}

//
// Measurement stuff...
//

int ResourcePool::getLosses() {
	return losses ;
}

double ResourcePool::getLossProbability() {
	return (double)losses / (double)arrivals ;
}

double ResourcePool::serverUtilisation() {
	return resources->utilisation() ;
}

double ResourcePool::meanNoOfQueuedCustomers() {
	return queue->meanQueueLength() ;
}

double ResourcePool::varianceOfNoOfQueuedCustomers() {
	return queue->varQueueLength() ;
}

double ResourcePool::meanTimeInQueue() {
	return queue->meanTimeInQueue() ;
}

double ResourcePool::varianceOfTimeInQueue() {
	return queue->varTimeInQueue() ;
}

void ResourcePool::resetMeasures() {
	queue->resetMeasures() ;
	resources->resetMeasures() ;
}

void ResourcePool::myLogResult(const string &name, const string &message, double value) {
	stringstream stream;
	stream << name << message;
	Logger::logResult( stream.str(), value ) ;

}

void ResourcePool::logResults() {
	myLogResult( name , ", Server utilisation", serverUtilisation() ) ;
	myLogResult( name , ", Mean number of customers in queue", meanNoOfQueuedCustomers() ) ;
	myLogResult( name , ", Variance of number of customers in queue", varianceOfNoOfQueuedCustomers() ) ;
	myLogResult( name , ", Conditional mean queueing time", meanTimeInQueue() ) ;
	myLogResult( name , ", Conditional variance of queueing time", varianceOfTimeInQueue() ) ;
	myLogResult( name , ", Losses", getLosses() ) ;
	myLogResult( name , ", Proportion of customers lost", getLossProbability() ) ;
}

