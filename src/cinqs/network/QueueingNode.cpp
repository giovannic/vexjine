#include "network/QueueingNode.h"

//
// The only difference between this and a resource pool is that 
// it releases a resource as soon as the customer leaves...
//
void QueueingNode::forward( Customer *c ) {
	ResourcePool::forward( c ) ;
	releaseResource();
}


std::string QueueingNode::getNodeType() {
	return "QueueingNode";
}
