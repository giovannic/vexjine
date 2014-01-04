#include "network/ProbabilisticBranch.h"

ProbabilisticBranch::ProbabilisticBranch( double *probs, CinqsNode **nodes ) {
	this->nodes = nodes ;
	dist = new DiscreteSampler( probs ) ;
	this->network = nodes[0]->getNetwork();
}

ProbabilisticBranch::~ProbabilisticBranch() {
	delete dist;
}

void ProbabilisticBranch::move( Customer *c ) {
	send( c, nodes[ dist->next() ] ) ;
}
