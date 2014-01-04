/*
 * VexLocalQueueingNode.h
 *
 *  Created on: 28 Jun 2011
 *      Author: root
 */

#ifndef VEXLOCALQUEUEINGNODE_H_
#define VEXLOCALQUEUEINGNODE_H_

#include "VexQueueingNode.h"
#include <string.h>

class VexLocalQueueingNode: public VexQueueingNode {
public:
	VexLocalQueueingNode( Network *_network, const std::string &s, Delay *d, int n ) : VexQueueingNode(_network, s, d, n ) {};
	VexLocalQueueingNode( Network *_network, const std::string &s, Delay *d, int n, Queue *q ) : VexQueueingNode(_network, s, d, n, q) {};

	virtual std::string getNodeType();

protected:
	virtual void invokeService( Customer *c );
};


#endif /* VEXLOCALQUEUEINGNODE_H_ */
