#ifndef NETWORK_CLASSDEPENDENTDELAY_H
#define NETWORK_CLASSDEPENDENTDELAY_H

#include "Delay.h"

class ClassDependentDelay : public Delay {
public:
	ClassDependentDelay( int *classes, DistributionSampler **samplers , int classesCount);

	virtual const char *getDelayDistributionSampleType();

	virtual double sample() ;
	virtual double sample( Customer *c ) ;

protected:
	int *classes ;
	CinqsNode *nodes ;
	int classesCount;
	DistributionSampler **samplers;
};

#endif
