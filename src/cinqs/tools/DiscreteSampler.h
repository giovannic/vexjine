#ifndef TOOLS_DISCRETESAMPLER_H
#define TOOLS_DISCRETESAMPLER_H

#include <cstdlib>

class DiscreteSampler {

public:
	DiscreteSampler( double *probs ) ;
	int next() ;

private:
	double *probs;
};

#endif
