#include "tools/Histogram.h"

using namespace std;

Histogram::Histogram( double l, double h, int b ) {
	bucket = new int [b] ;
	overflows = 0;
	low = l ;
	high = h ;
	n = b ;
	width = ( high - low ) / n ;
}

Histogram::~Histogram() {
	delete[] bucket;
}


void Histogram::add( double x ) {
	CustomerMeasure::add( x ) ;
	if ( x < low )
		underflows++ ;
	else if ( x >= high )
		overflows++ ;
	else {
		int index = (int)( ( x - low ) / width ) ;
		bucket[ index ]++ ;
	}
}

int Histogram::bucketContent( int i ) {
	return bucket[ i ] ;
}

void Histogram::display() {
//	DecimalFormat decimal = new DecimalFormat( "000000.000" ) ;
//	DecimalFormat integer = new DecimalFormat( "0000000" ) ;
//	DecimalFormat general = new DecimalFormat( "######.###" ) ;
	const int maxHeight = 20 ;
	cout <<  "\nObservations = " << CustomerMeasure::count() <<
			"   Mean = " << ( CustomerMeasure::mean() ) <<
			"   Variance = " << ( CustomerMeasure::variance() ) << endl ;
	int max = 0 ;
	for ( int i = 0 ; i < n ; i++ )
		if ( bucket[i] > max )
			max = bucket[i] ;
	if ( max == 0 )
		cout <<  "Histogram is empty\n" << endl ;
		else {
			for ( int i = 0 ; i < n ; i++ ) {
				cout <<  ( low + i * width ) << " - " <<
						( low + ( i + 1 ) * width ) << "   " <<
						( (double) bucket[i] ) << "  |" << endl ;
				string stars = "" ;
				for ( int j = 0 ; j < (double) bucket[i] / max * maxHeight ; j++ )
					stars += "*" ;
				cout <<  stars + "\n" << endl ;
			}
			cout <<  "Underflows = " << underflows <<
					"   Overflows = " << overflows << "\n" << endl ;
		}
}

