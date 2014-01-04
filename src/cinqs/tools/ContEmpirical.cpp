#include "tools/ContEmpirical.h"

#include "Check.h"
#include <cassert>

ContEmpirical::ContEmpirical( double *xs, double *fs, int xlength, int flength) {
	assert(false);
	Check::check( xlength == flength + 1 && flength > 0, "Empirical distribution array mismatch" ) ;
	this->xs = new double[ xlength ] ;
	this->xs = xs ;
	this->cs = new double[ xlength ] ;
	double fTotal = 0.0 ;
	for ( int i = 0 ; i < flength ; i++ ) {
		fTotal += fs[ i ] ;
	}
	cs[ 0 ] = 0 ;
	for ( int i = 0 ; i < flength ; i++ ) {
		cs[ i + 1 ] = cs[ i ] + fs[ i ] / fTotal ;
	}
}

double ContEmpirical::next() {
	double r = ((double)rand()/(double)RAND_MAX) ;
	int index = 0 ;
	while ( r >= cs[ index + 1 ] ) {
		index++ ;
	}
	return xs[ index ] +
	( r - cs[ index ] ) / ( cs[ index + 1 ] - cs[ index ] ) *
	( xs[ index + 1 ] - xs[ index ] ) ;
}
