#include "network/Delay.h"


Delay::Delay() {
}

Delay::Delay( DistributionSampler *s ) {
	sampler = s ;
}

const char *Delay::getDelayDistributionSampleType() {
	return sampler->getDistributionSampleType();
}

double Delay::sample() {
	return sampler->next() ;
}

double Delay::sample( Customer *c ) {
	return sampler->next() ;
}

