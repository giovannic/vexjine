#include "tools/ReplicatedSim.h"
#include "Logger.h"

#include <iostream>
using namespace std;

ReplicatedSim::ReplicatedSim( int n, double a ) {
	cout <<  "Progress (" << n << " runs): " << endl ;
	for ( int run = 0 ; run < n ; run++ ) {
		runSimulation() ;
		cout <<  ( run + 1 ) << " " << endl ;
	}
	cout << endl ;
	Logger::displayResults( a ) ;
}

void ReplicatedSim::runSimulation() {

}


