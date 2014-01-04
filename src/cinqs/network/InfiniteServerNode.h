#ifndef NETWORK_INFINITESERVERNODE_H
#define NETWORK_INFINITESERVERNODE_H

#include "Node.h"
#include "Event.h"
#include "Delay.h"

class EndServiceEvent;

class InfiniteServerNode : public CinqsNode {

public:
	InfiniteServerNode( Network *_network, Delay *d ) : CinqsNode(_network, "Delay Node") {serviceTime = d;lastEndServiceEvent = NULL;};
	InfiniteServerNode( Network *_network, const std::string &s, Delay *d ) : CinqsNode (_network, s) {serviceTime = d;lastEndServiceEvent = NULL;};

	~InfiniteServerNode();
	//EndServiceEvent( Customer *c, double t ) ;
	void invoke() ;
	Customer *getCustomer() ;
	virtual void enter( Customer *c ) ;
	virtual std::string getNodeType();
protected:
	void invokeService( Customer *c ) ;
	virtual void accept( Customer *c ) ;

	Delay *serviceTime ;
	EndServiceEvent *lastEndServiceEvent ;
};

class EndServiceEvent : public Event {
public:
	EndServiceEvent( Customer *c, double t , InfiniteServerNode *node) : Event(t) {
		customer = c ;
		this->node = node;
	}
	void invoke() {
		node->forward( customer ) ;
	}
	Customer *getCustomer() {
		return customer ;
	}

private:
	Customer *customer ;
	InfiniteServerNode *node;
};

#endif
