/*
 * VexQueueingNode.h
 *
 *  Created on: 6 Jun 2011
 *      Author: root
 */

#ifndef VEXQUEUEINGNODE_H_
#define VEXQUEUEINGNODE_H_

#include "QueueingNode.h"
#include <string.h>

class PostQueueingActionNode;
class PostServiceActionNode;

class VexQueueingNode: public QueueingNode {
public:
	VexQueueingNode( Network *_network, const std::string &s, Delay *d, int n ) : QueueingNode(_network, s, d, n ) {
		init(_network, s);
	};
	VexQueueingNode( Network *_network, const std::string &s, Delay *d, int n, Queue *q ) : QueueingNode(_network, s, d, n, q) {
		init(_network, s);
	};

	virtual std::string getNodeType();

	void releaseResourceAt(const long long &releaseTime) ;

	void onCompletingQueueing(Customer *c);
	void onCompletingService( Customer *c);

	unsigned int uncontendedServiceTimes;
	unsigned int queueingTimes;

protected:
	virtual void accept( Customer *c ) ;
	virtual void invokeService( Customer *c );


	void init( Network *_network, const std::string &s);

	void lock();
	void unlock();
	pthread_mutex_t mutex;
	pthread_cond_t cond;

	PostQueueingActionNode *postQueueingAction;
	PostServiceActionNode *postServiceAction;

};


#endif /* VEXQUEUEINGNODE_H_ */

