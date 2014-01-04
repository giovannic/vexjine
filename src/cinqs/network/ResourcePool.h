#ifndef NETWORK_RESOURCEPOOL_H
#define NETWORK_RESOURCEPOOL_H

#include "InfiniteServerNode.h"
#include "Resource.h"
#include "FIFOQueue.h"


#include "Network.h"

class ResourcePool : public InfiniteServerNode {

public:
	ResourcePool( Network *_network, const std::string &s, Delay *d, int n ) : InfiniteServerNode(_network, s, d) {
		noOfResources = n ;
		queue = new FIFOQueue() ;
		resources = new Resource( noOfResources );
		lossNode = Network::nullNode;
		losses = 0;
	}

	ResourcePool( Network *_network, const std::string &s, Delay *d, int n, Queue *q ) : InfiniteServerNode(_network, s, d) {
		noOfResources = n ;
		queue = q ;
		resources = new Resource( noOfResources ) ;
		lossNode = Network::nullNode;
		losses = 0;
	}

	virtual std::string getNodeType();
	virtual ~ResourcePool();

	void setLossNode( CinqsNode *n ) ;
	std::string toString() ;

	int queueLength() ;
	int getLosses() ;

	double getLossProbability() ;
	double serverUtilisation() ;
	double meanNoOfQueuedCustomers() ;
	double varianceOfNoOfQueuedCustomers() ;
	double meanTimeInQueue() ;
	double varianceOfTimeInQueue() ;


	virtual void releaseResource() ;
	virtual void resetMeasures() ;
	virtual void logResults() ;

protected:
	Queue *queue ;
	CinqsNode *lossNode ;
	Resource *resources ;

	int noOfResources ;
	int losses;

	virtual void accept( Customer *c ) ;
	void myLogResult(const std::string &name, const std::string &message, double value);
};

#endif
