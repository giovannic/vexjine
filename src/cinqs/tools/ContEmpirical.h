#ifndef TOOLS_CONTEMPIRICAL_H
#define TOOLS_CONTEMPIRICAL_H

#include "DistributionSampler.h"

class ContEmpirical : public DistributionSampler {

public:
	ContEmpirical( double *xs, double *fs, int xlength, int flength ) ;
	double next() ;

private:
	double *xs, *cs ;

};

#endif
