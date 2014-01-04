#ifndef TOOLS_UNIFORM_H
#define TOOLS_UNIFORM_H

#include "DistributionSampler.h"

class Uniform : public DistributionSampler {

public:
	Uniform( double a, double b ) ;
	double next() ;
	static double uniform(  double a, double b ) ;

private:
	double a, b ;

};

#endif
