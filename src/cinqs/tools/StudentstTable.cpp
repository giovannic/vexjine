#include "tools/StudentstTable.h"

#include <cmath>

double StudentstTable::alphas[4] = { 0.01, 0.025, 0.05, 0.10 } ;
int StudentstTable::minDof = 1 ;
int StudentstTable::maxDof = 20 ;

double StudentstTable::t01[21] =
{ 1000000.0, 31.82, 6.92, 4.54, 3.75, 3.36, 3.14, 3.00, 2.90, 2.82,
		2.76, 2.72, 2.68, 2.65, 2.62, 2.60, 2.58, 2.57, 2.55, 2.54,
		2.53 } ;
double StudentstTable::t025[21] =
{ 1000000.0, 12.71, 4.3, 3.18, 2.78, 2.57, 2.45, 2.36, 2.31, 2.26,
		2.23, 2.20, 2.18, 2.16, 2.14, 2.13, 2.12, 2.11, 2.10, 2.09,
		2.09 } ;
double StudentstTable::t05[21] =
{ 1000000.0, 6.31, 2.92, 2.35, 2.13, 2.02, 1.94, 1.90, 1.86, 1.83,
		1.81, 1.80, 1.78, 1.77, 1.76, 1.75, 1.75, 1.74, 1.73, 1.73,
		1.72 } ;
double StudentstTable::t10[21] =
{ 1000000.0, 3.08, 1.89, 1.64, 1.53, 1.48, 1.44, 1.42, 1.40, 1.38,
		1.37, 1.36, 1.36, 1.35, 1.34, 1.34, 1.34, 1.33, 1.33, 1.33,
		1.32 } ;

bool StudentstTable::checkConfidenceLevel( double alpha ) {
	for ( int i = 0 ; i < 4 ; i++ ) {
		if ( fabs( alpha - alphas[ i ] ) < 0.00000001 ) {
			return true ;
		}
	}
	return false ;
}

bool StudentstTable::checkDegreesOfFreedom( int dof ) {
	return ( dof >= minDof && dof <= maxDof ) ;
}

double StudentstTable::table( int dof, double alpha ) {
	if ( fabs( alpha - 0.01 ) < 0.00001 ) {
		return t01[ dof ] ;
	} else if ( fabs( alpha - 0.025 ) < 0.00001 ) {
		return t025[ dof ] ;
	} else if ( fabs( alpha - 0.05 ) < 0.00001 ) {
		return t05[ dof ] ;
	} else if ( fabs( alpha - 0.1 ) < 0.00001 ) {
		return t10[ dof ] ;
	} else {
		return 0.0 ;
	}
}
