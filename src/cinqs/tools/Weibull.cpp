#include "tools/Weibull.h"


Weibull::Weibull( double alpha, double beta ) {
	this->alpha = alpha ;
	this->beta = beta ;
}

double Weibull::next() {
	return alpha * pow( -log( ((double)rand()/(double)RAND_MAX) ), 1.0 / beta ) ;
}

double Weibull::weibull(  double alpha, double beta ) {
	return alpha * pow( -log( ((double)rand()/(double)RAND_MAX) ), 1.0 / beta ) ;
}
