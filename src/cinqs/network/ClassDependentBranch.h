#ifndef NETWORK_CLASSDEPENDENTBRANCH_H
#define NETWORK_CLASSDEPENDENTBRANCH_H

#include "Link.h"

class ClassDependentBranch : public Link {
public:
	ClassDependentBranch( int *classes, CinqsNode **nodes , int classesCount );

protected:
	void move( Customer *c ) ;

private:
	CinqsNode *nextNode( Customer *c ) ;

	int *classes ;
	CinqsNode **nodes ;
	int classesCount;
};

#endif
