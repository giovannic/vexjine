/*
 * VexInfiniteServerNode.h: An infinite server node of a queueing network
 * that corresponds to an remote (external) resource. This means that
 * the service time will not affect local execution/scheduling on the host.
 *
 *  Created on: 26 May 2011
 *      Author: root
 */

#ifndef VEXINFINITESERVERNODE_H_
#define VEXINFINITESERVERNODE_H_

#include "Network.h"
#include "InfiniteServerNode.h"

class PostActionInfiniteServer;

class VexInfiniteServerNode : public InfiniteServerNode {

public:
	VexInfiniteServerNode(Network *_network, const std::string &s, Delay *d) : InfiniteServerNode(_network, s, d) {
		init(_network, s);
	};

	std::string getNodeType();
	~VexInfiniteServerNode();

	void onCompletingService( Customer *c);

protected:
	void init( Network *_network, const std::string &s);

	void accept( Customer *c ) ;
	PostActionInfiniteServer *postServiceAction;

};


#endif /* VEXINFINITESERVERNODE_H_ */
