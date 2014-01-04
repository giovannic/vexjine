#include "tools/DiscEmpirical.h"

#include <cassert>

DiscEmpirical::DiscEmpirical( double *xs, double *fs) {
	assert(false);
	/*
	Check::check( xs.length == fs.length && fs.length > 0,
			"Empirical distribution array mismatch" ) ;
	this->xs = new double[ xs.length ] ;
	this->xs = xs ;
	this->cs = new double[ xs.length ] ;
	double fTotal = 0.0 ;
	for ( int i = 0 ; i < fs.length ; i++ ) {
		fTotal += fs[ i ] ;
	}
	cs[ 0 ] = fs[ 0 ] / fTotal ;
	for ( int i = 1 ; i < fs.length ; i++ ) {
		cs[ i ] = cs[ i - 1 ] + fs[ i ] / fTotal ;
	}
	*/
}

DiscEmpirical::~DiscEmpirical() {
	delete xs;
	delete cs;
}

double DiscEmpirical::next() {
	double r = ((double)rand()/(double)RAND_MAX) ;
	int index = 0 ;
	while ( r >= cs[ index ] ) {
		index++ ;
	}
	return xs[ index ] ;
}
