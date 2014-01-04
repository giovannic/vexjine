#ifndef TOOLS_PARETO_H
#define TOOLS_PARETO_H

#include "DistributionSampler.h"

class Pareto : public DistributionSampler {

public:
	Pareto( double k, double a, double b ) ;
	double next() ;
	static double pareto( double k, double a, double b ) ;

private:
	double k, a, b, ak, bk ;

};

#endif
