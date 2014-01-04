#include "network/PreemptiveResumeNode.h"

#include "Debug.h"
#include "Sim.h"

using namespace std;
//
// In the resume policy the remaining service time can be found
// straightforwardly from the descheduled end service event...
//
double PreemptiveResumeNode::remainingServiceTime( Customer *c ) {
	return lastEndServiceEvent->invokeTime() - Sim::now() ;
}

string PreemptiveResumeNode::getNodeType() {
	return "PreemptiveResumeNode";
}
void PreemptiveResumeNode::accept( Customer *c ) {
	if ( resources->resourceIsAvailable() ) {
		Debug::trace( "Resource claimed" ) ;
		resources->claim() ;
	} else {
		Sim::deschedule( lastEndServiceEvent ) ;
		Customer *preemptedCustomer = lastEndServiceEvent->getCustomer() ;
		stringstream str;
		str << "Preempting customer in service (Id " << preemptedCustomer->getId() << ")";
		Debug::trace( str.str() ) ;
		double nextServiceDemand = remainingServiceTime( preemptedCustomer ) ;
		preemptedCustomer->setServiceDemand( nextServiceDemand ) ;
		queue->enqueueAtHead( preemptedCustomer ) ;
	}
	invokeService( c ) ;
}

