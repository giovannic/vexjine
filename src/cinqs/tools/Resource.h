#ifndef TOOLS_RESOURCE_H
#define TOOLS_RESOURCE_H

#include "SystemMeasure.h"

class Resource {

public:
	Resource() ;
	virtual ~Resource() ;
	Resource( int n ) ;
	void claim() ;
	void release() ;
	int numberOfAvailableResources() ;
	bool resourceIsAvailable() ;
	double utilisation() ;
	void resetMeasures() ;

protected:
	bool resourceAvailable ;
	SystemMeasure *resourceCount;
	int resources ;
	int nresources ;

};

#endif
