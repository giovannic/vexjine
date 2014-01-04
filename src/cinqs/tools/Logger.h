#ifndef TOOLS_LOGGER_H
#define TOOLS_LOGGER_H

#include "StudentstTable.h"

#include <string>
#include <iostream>



class Logger {

public:
	static void initialize();
	static void logResult( const std::string &id, double value ) ;
	static void displayResults() ;
	static void displayResults( double alpha ) ;
	static void display( double alpha, bool alphaOK );
	static double sampleMean( double *xs, int n ) ;
	static double sampleVariance( double *xs, int n, double mean );

	static int maxEntries ;
	static int maxReplications ;
	static int logEntries ;

	static int entries[500] ;
	static std::string ids[500] ;
	static double values[500][21];
};

#endif
