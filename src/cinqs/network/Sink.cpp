#include "network/Sink.h"

#include "Network.h"
#include "Sim.h"

/**
 * A Sink node absrobs customers from a queueing network.
 * Departing customers are registered with the {@link Network}
 * class, which records the customer's sojourn time.
 * @param name The name of the source node
 * @param d The {@link DistributionSampler} used to generate the
 *          inter-arrival times
 * @param b The {@link DistributionSampler} used to generate the
            batch sizes
 */

std::string Sink::getNodeType() {
	return "Sink";
}

//
// Do nothing here - customer is absorbed...
//
void Sink::accept( Customer *c ) {
	network->increaseCustomerCompletions();
	network->registerCompletion( Sim::now() - c->getArrivalTime() ) ;
	c->setFinished();
	if (!c->isFinished()) {	// hack: only CINQS should clean up - CINQS customers do nothing on setFinished
		delete c;
		c=NULL;
	}
}

