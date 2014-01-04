#ifndef TOOLS_GEOMETRIC_H
#define TOOLS_GEOMETRIC_H

#include "DistributionSampler.h"

class Geometric : public DistributionSampler {

public:
	Geometric( double p ) ;
	double next() ;
	static double geometric( double p ) ;

private:
	double q ;

};

#endif
