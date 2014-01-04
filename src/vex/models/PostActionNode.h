/*
 * PostActionNode.h
 *
 *  Created on: 24 Sep 2011
 *      Author: root
 */

#ifndef POSTACTIONNODE_H_
#define POSTACTIONNODE_H_

#include "VexQueueingNode.h"
#include "VexInfiniteServerNode.h"

class PostActionNode : public CinqsNode {

public:
	PostActionNode(Network *_network, const std::string &s, VexQueueingNode *vexQueueingNode) : CinqsNode(_network, s) {
		attachedToVexQueueingNode = vexQueueingNode;
	}
	virtual ~PostActionNode();
	virtual void enter( Customer *c ) = 0;
	void forward( Customer *c ) ;

protected:
	void accept( Customer *c ) ;
	VexQueueingNode *attachedToVexQueueingNode;
};


class PostServiceActionNode : public PostActionNode {
public:
	PostServiceActionNode(Network *_network, const std::string &s, VexQueueingNode *v) : PostActionNode(_network, s, v) {};
	void enter( Customer *c ) ;
};


class PostQueueingActionNode : public PostActionNode {
public:
	PostQueueingActionNode(Network *_network, const std::string &s, VexQueueingNode *v) : PostActionNode(_network, s, v) {};
	void enter( Customer *c ) ;
};




class PostActionInfiniteServer : public CinqsNode {

public:
	PostActionInfiniteServer(Network *_network, const std::string &s, VexInfiniteServerNode *node) : CinqsNode(_network, s) {
		attachedToVexInfiniteServerNode = node;
	}
	virtual ~PostActionInfiniteServer();
	void enter( Customer *c );
	void forward( Customer *c ) ;

protected:
	void accept( Customer *c ) ;
	VexInfiniteServerNode *attachedToVexInfiniteServerNode;
};

#endif /* POSTACTIONNODE_H_ */
