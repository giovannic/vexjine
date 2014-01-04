#include "tools/Normal.h"

#include <cmath>
#include <sstream>

Normal::Normal( double mu, double sigma ) {
	this->mu = mu ;
	this->sigma = sigma ;
	mustRedo = false;
}

const char *Normal::getDistributionSampleType() {
	std::stringstream str;
	str << "Normal distribution (" << mu << ", " << sigma << ")";
	return str.str().c_str();
}

double Normal::next() {
	mustRedo = !mustRedo ;
	if ( mustRedo ) {
		r1 = ((double)rand()/(double)RAND_MAX) ;
		r2 = ((double)rand()/(double)RAND_MAX) ;
		k = sqrt( -2 * log( r1 ) ) ;
		return k * cos( twoPI * r2 ) * sigma + mu ;
	}
	else
		return k * sin( twoPI * r2 ) * sigma + mu ;
}

double Normal::normal(  double m, double s ) {
	double r1 = ((double)rand()/(double)RAND_MAX) ;
	double r2 = ((double)rand()/(double)RAND_MAX) ;
	double k  = sqrt( -2 * log( r1 ) ) ;
	return k * cos( twoPI * r2 ) * s + m ;
}
