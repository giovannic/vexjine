#ifndef NETWORK_QUEUEINGNODE_H
#define NETWORK_QUEUEINGNODE_H

#include "ResourcePool.h"

class QueueingNode : public ResourcePool {

public:
	QueueingNode( Network *_network, const std::string &s, Delay *d, int n ) : ResourcePool(_network, s, d, n ) {};
	QueueingNode( Network *_network, const std::string &s, Delay *d, int n, Queue *q ) : ResourcePool(_network, s, d, n, q) {};

protected:
	void forward( Customer *c ) ;
	std::string getNodeType();

};

#endif
