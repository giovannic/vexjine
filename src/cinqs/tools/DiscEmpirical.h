#ifndef TOOLS_DISCEMPIRICAL_H
#define TOOLS_DISCEMPIRICAL_H

#include "DistributionSampler.h"

class DiscEmpirical : public DistributionSampler {

public:
	DiscEmpirical( double *xs, double *fs ) ;
	double next() ;
	~DiscEmpirical();

private:
	double *xs, *cs ;

};

#endif
