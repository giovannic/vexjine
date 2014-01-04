#include "tools/Exp.h"

#include <iostream>
#include <sstream>

Exp::Exp( double r ) {
	rate = r ;
}

const char *Exp::getDistributionSampleType() {
	std::stringstream str;
	str << "Exponential distribution with rate " << rate;
	return str.str().c_str();
}

double Exp::next() {
	return -log( ((double)rand()/(double)RAND_MAX) ) / rate ;
}

double Exp::exp( double lam ) {
	return -log( ((double)rand()/(double)RAND_MAX) ) / lam ;
}
