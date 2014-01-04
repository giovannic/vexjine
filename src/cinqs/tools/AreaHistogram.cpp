#include "tools/AreaHistogram.h"

using namespace std;

AreaHistogram::~AreaHistogram() {
	delete[] bucket;
}

void AreaHistogram::add( double t ) {
	double current = SystemMeasure::currentValue() ;
	double lastChange = SystemMeasure::timeLastChanged() ;
	if ( current < low )
		underflows++ ;
	else if ( current >= high )
		overflows++ ;
	else {
		int index = (int)( ( current - low ) / width ) ;
		bucket[ index ] += *Sim::vtime - lastChange ;
	}
	SystemMeasure::add( t ) ;
}

double AreaHistogram::bucketContent( int i ) {
	return bucket[ i ] ;
}

void AreaHistogram::display() {
//	DecimalFormat decimal = new DecimalFormat( "000000.00" ) ;
//	DecimalFormat area    = new DecimalFormat( "0000000.00" ) ;
//	DecimalFormat general = new DecimalFormat( "######.##" ) ;
	const int maxHeight = 20 ;
	cout <<  "\nNo. of state changes = " << SystemMeasure::count() <<
			"   Mean = " << ( SystemMeasure::mean() ) <<
			"   Variance = " << ( SystemMeasure::variance() ) << endl ;
	double max = 0 ;
	for ( int i = 0 ; i < n ; i++ )
		if ( bucket[i] > max )
			max = bucket[i] ;
	if ( max == 0 )
		cout <<  "Histogram is empty\n" << endl ;
	else {
		for ( int i = 0 ; i < n ; i++ ) {
			cout <<  ( i * width ) << " - " <<
					( ( i + 1 ) * width ) << "   " <<
					( (double) bucket[i] ) << "  |" << endl ;
			string stars = "" ;
			for ( int j = 0 ; j < (double) bucket[i] / max * maxHeight ; j++ )
				stars += "*" ;
			cout <<  stars + "\n" << endl ;
		}
		cout <<  "Underflows = " << underflows << " Overflows = " << overflows << "\n" << endl ;
	}

}

