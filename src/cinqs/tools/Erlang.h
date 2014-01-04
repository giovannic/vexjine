#ifndef TOOLS_ERLANG_H
#define TOOLS_ERLANG_H

#include "DistributionSampler.h"

class Erlang : public DistributionSampler {

public:
	Erlang( int k, double theta ) ;
	double next() ;
	static double erlang( int k, double theta ) ;

private:
	int k ;
	double theta ;
	double acc ;

};

#endif
