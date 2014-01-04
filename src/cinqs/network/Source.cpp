#include "network/Source.h"

void Source::invoke() {
	injectCustomers() ;
	schedule();
}

void Source::schedule() {
	if (delay != NULL) {
		Sim::schedule( new Arrival( Sim::now() + delay->next() , this )) ;
	}
}
/**
 * A Source node injects customers into a queueing network.
 * The inter-arrival time (a {@link DistributionSampler})
 * must be specified.  Arrivals may
 * optionally be batched, with the batch size specified by
 * a second {@link DistributionSampler}.
 */

Source::~Source() {
	delete batchsize;
}



/**
 * Builds a new customer. This can be overridden to support 
 * specialised {@link Customer} subclasses.
 * @return a customer
 *
 */
Customer *Source::buildCustomer() {
	return new Customer() ;
}

/** 
 * Injects customers into the network using forward.
 * The initial location of the customer is the source node.
 */
void Source::injectCustomer() {
	Customer *c =  buildCustomer() ;
	c->setLocation( this ) ;
	forward( c ) ;
}

void Source::injectCustomers() {
	int nArrivals = (int) batchsize->next() ;
	for ( int i = 0 ; i < nArrivals ; i++ ) {
		injectCustomer() ;
	}
}


std::string Source::getNodeType() {
	return "Source";

}
