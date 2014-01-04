/*
 * PostActionNode.cpp: This class is used with VexSchedSimQueueingNetworks.
 * In this case, invokeService or waitRemote or blockAtQueue are invoked by the scheduler
 * and then the scheduler needs to exit the node and continue its normal execution.
 * When the current action (invokeService or waitRemote or blockAtQueue) finishes then
 * the enterNode() method is called on a Post*ActionNode class, which now redirects the
 * execution flow to the methods of VexQueueingNode that need to be executed.
 *
 *  Created on: 24 Sep 2011
 *      Author: root
 */

#include "PostActionNode.h"

PostActionNode::~PostActionNode() {

}

void PostActionNode::forward( Customer *c ) {

}

void PostActionNode::accept( Customer *c )  {

}


void PostServiceActionNode::enter( Customer *c ) {
	attachedToVexQueueingNode->onCompletingService(c);
}


void PostQueueingActionNode::enter( Customer *c ) {
	attachedToVexQueueingNode->onCompletingQueueing(c);
}



PostActionInfiniteServer::~PostActionInfiniteServer() {

}

void PostActionInfiniteServer::enter( Customer *c ) {
	attachedToVexInfiniteServerNode->onCompletingService(c);
}


void PostActionInfiniteServer::accept( Customer *c )  {

}

void PostActionInfiniteServer::forward( Customer *c ) {

}

