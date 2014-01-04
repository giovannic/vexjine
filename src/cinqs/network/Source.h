#ifndef NETWORK_SOURCE_H
#define NETWORK_SOURCE_H

#include "Node.h"
#include "Event.h"
#include "Sim.h"
#include "Network.h"
#include "Deterministic.h"

class Arrival;

class Source : public CinqsNode {

public:
	Source( Network *_network, const std::string &name, DistributionSampler *d ) : CinqsNode(_network, name) {
		delay = d ;
		batchsize = new Deterministic( 1 ) ;
		schedule();

	}
	Source( Network *_network, const std::string &name, DistributionSampler *d, DistributionSampler *b ) : CinqsNode(_network, name) {
		delay = d ;
		batchsize = b ;
		schedule();
	}

	void invoke();
	~Source();
//	Arrival( double t ) ;

	virtual std::string getNodeType();
protected:
	DistributionSampler *delay ;
	DistributionSampler *batchsize ;
	Customer *buildCustomer() ;

private:
	void injectCustomer();
	void injectCustomers();
	void schedule();

};

class Arrival : public Event {
public:
	Arrival( double t , Source *source) : Event(t) {this->source = source;}

	void invoke() {
		source->invoke();
	}

private:
	Source *source;
};

#endif
