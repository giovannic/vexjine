#ifndef NETWORK_DELAY_H
#define NETWORK_DELAY_H

#include "tools/DistributionSampler.h"
#include "Customer.h"

class Delay {

public:
	Delay() ;
	Delay( DistributionSampler *s ) ;

	virtual const char *getDelayDistributionSampleType();

	virtual double sample() ;
	virtual double sample( Customer *c ) ;
protected:

	DistributionSampler *sampler ;

};

#endif
