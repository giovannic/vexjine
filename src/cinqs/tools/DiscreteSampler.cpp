#include "tools/DiscreteSampler.h"


DiscreteSampler::DiscreteSampler( double *probs ) {
	this->probs = probs ;
}

int DiscreteSampler::next() {
	double acc = probs[ 0 ] ;
	int index = 0 ;
	double r = ((double)rand()/(double)RAND_MAX) ;
	while ( acc < r ) {
		index++ ;
		acc += probs[ index ] ;
	}
	return index ;
}
