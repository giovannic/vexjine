#include "tools/Logger.h"
#include <cmath>

using namespace std;

int Logger::maxEntries;
int Logger::maxReplications;
int Logger::logEntries ;

int Logger::entries[ 500 ] ;
string Logger::ids[ 500 ] ;
double Logger::values[500][21];

void Logger::initialize() {
	maxEntries = 500;
	maxReplications = 21;

	for (int i =0 ;i<maxEntries; i++) {
		for (int j =0 ;j<maxReplications; j++) {
			values[i][j] = 0;
		}
		entries[i] = 0;
		ids[i] = "";
	}
	logEntries = 0;
}

void Logger::logResult( const string &id, double value ) {
	int i = 0;
	for ( i = 0 ; i < logEntries ; i++ ) {
		if ( ids[ i ] == id ) {
			break ;
		}
	}
	if ( i == logEntries ) {
		entries[ i ] = 1 ;
		ids[ i ] = id ;
		logEntries++ ;
	} else {
		entries[ i ] += 1 ;
	}
	values[ i ][ entries[ i ] - 1 ] = value ;
}

double Logger::sampleMean( double *xs, int n ) {
	double acc = 0.0 ;
	for ( int i = 0 ; i < n ; i++ ) {
		acc += xs[ i ] ;
	}
	return acc / n ;
}

double Logger::sampleVariance( double *xs, int n, double mean ) {
	double acc = 0.0 ;
	for ( int i = 0 ; i < n ; i++ ) {
		double diff = xs[ i ] - mean ;
		acc += diff * diff ;
	}
	return acc / ( n - 1 ) ;
}

void Logger::displayResults() {
	cout <<  "\nConfidence level not supplied\n" << endl ;
	display( 0.05, true ) ;
}

void Logger::displayResults( double alpha ) {
	if ( !StudentstTable::checkConfidenceLevel( alpha ) ) {
		cout <<  "\nNOTE: no table data is available for the " << "specified confidence level (" << alpha << ")" << endl ;
		display( alpha, false ) ;
	} else {
		display( alpha, true ) ;
	}
}

void Logger::display( double alpha, bool alphaOK ) {
	if ( logEntries > 0 ) {
		double mean = 0.0;
		cout <<  "\nSUMMARY OF STATISTICS\n" << endl ;
		if ( alphaOK ) {
			cout <<  "Confidence level: " << alpha*100 << "%\n" << endl ;
		}
		for ( int i = 0 ; i < logEntries ; i++ ) {
			mean = sampleMean( values[ i ], entries[ i ] ) ;
			cout <<  ids[ i ] << endl ;
			cout <<  "   Point estimate:  " << mean << endl ;

			int n = entries[ i ] ;
			if ( n > 1 ) {
				cout <<  "   Degrees of freedom: " << ( n-1 ) << endl ;
			}
			bool dofOK = StudentstTable::checkDegreesOfFreedom( n - 1 ) ;
			if ( n > 1 && !dofOK ) {
				cout <<  "No table data is available for the degrees of freedom (" << ( n-1 ) << ")" << endl ;
			}
			if ( alphaOK && dofOK ) {
				double s = sqrt( sampleVariance( values[ i ], n, mean ) ) ;
				double z = StudentstTable::table( n - 1, alpha ) ;
				if ( z > 0.0 ) {
					cout <<  "   C.I. half width: " << z * s / sqrt( (double)n ) << endl ;
				}
			}
			cout << endl ;
		}
	} else {
		cout <<  "No results were logged" << endl ;
	}
}


