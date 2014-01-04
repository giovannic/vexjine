#include "network/InfiniteServerNode.h"

#include "Debug.h"
#include "Sim.h"

using namespace std;


InfiniteServerNode::~InfiniteServerNode() {
//	if (lastEndServiceEvent != NULL) {
//		delete lastEndServiceEvent;
//	}
}
string InfiniteServerNode::getNodeType() {
	return "InfiniteServerNode";
}

//
// Invokes a service delay when called. After the delay, the method
// forward is called, which can be overridden to effect
// special behaviours.
//
void InfiniteServerNode::invokeService( Customer *c ) {
	double serveTime = c->getServiceDemand() ;
	stringstream str;
	str << "Customer " << c->getId() << " entering service, " << "service time = " << serveTime << endl;
	Debug::trace( str.str() ) ;

	lastEndServiceEvent = new EndServiceEvent( c, Sim::now() + serveTime , this) ;
	Sim::schedule( lastEndServiceEvent ) ;
}

/*
InfiniteServerNode::EndServiceEvent( Customer *c, double t ) {
	super( t ) ;
	customer = c ;
}
*/

void InfiniteServerNode::invoke() {
	forward( lastEndServiceEvent->getCustomer() ) ;
}
Customer *InfiniteServerNode::getCustomer() {
	return lastEndServiceEvent->getCustomer();
}



//
// The service demand is set on entry as the customer may be preempted
// by a subclass.  In preemptive-resume strategies, this case the
// demand needs to be reduced to reflect the remaining service time.
// This method is finalised - subclasses should modify behaviour via 
// accept().
//
void InfiniteServerNode::enter( Customer *c ) {
	c->setServiceDemand( serviceTime->sample( c ) ) ;
	CinqsNode::enter( c ) ;
}

//
// delay.  
//
void InfiniteServerNode::accept( Customer *c ) {
	invokeService( c ) ;
}

