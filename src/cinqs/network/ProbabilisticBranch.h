#ifndef NETWORK_PROBABILISTICBRANCH_H
#define NETWORK_PROBABILISTICBRANCH_H

#include "Link.h"
#include "DiscreteSampler.h"

class ProbabilisticBranch : public Link {

public:
	ProbabilisticBranch( double *probs, CinqsNode **nodes ) ;
	~ProbabilisticBranch();

	void move( Customer *c ) ;

protected:
	DiscreteSampler *dist ;
	CinqsNode **nodes ;
};

#endif
