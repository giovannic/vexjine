#include "tools/Geometric.h"


Geometric::Geometric( double p ) {
	q = log( 1 - p ) ;
}

double Geometric::next() {
	return (int) ( log( ((double)rand()/(double)RAND_MAX) ) / q ) ;
}

double Geometric::geometric( double p ) {
	return (int) ( log( ((double)rand()/(double)RAND_MAX) ) / log( 1 - p ) ) ;
}
