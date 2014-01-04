#include "tools/Resource.h"
#include "Check.h"

#include <iostream>
Resource::Resource() {
	resourceCount = new SystemMeasure();
	resources = 1 ;
	nresources = 1 ;
}

Resource::Resource( int n ) {
	resourceCount = new SystemMeasure();
	resources = n ;
	nresources = n ;
}

Resource::~Resource() {
	delete resourceCount;
}
void Resource::claim() {
	Check::check( resourceIsAvailable(), "Attempt to claim unavailable resource" ) ;
	resources-- ;
	resourceCount->add( (float)( nresources - resources ) ) ;
}

void Resource::release() {
	Check::check( resources < nresources, "Attempt to release non-existent resource in Resource::release") ;
	resources++ ;
	resourceCount->add( (float)( nresources - resources ) ) ;
}

int Resource::numberOfAvailableResources() {
	return resources ;
}

bool Resource::resourceIsAvailable() {
	return resources > 0 ;
}

double Resource::utilisation() {
	return resourceCount->mean() / nresources ;
}

void Resource::resetMeasures() {
	resourceCount->resetMeasures() ;
}

