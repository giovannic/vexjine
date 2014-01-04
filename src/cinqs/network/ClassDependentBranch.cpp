#include "network/ClassDependentBranch.h"
#include "Check.h"

ClassDependentBranch::ClassDependentBranch( int *classes, CinqsNode **nodes , int classesCount ) {
	this->classes = classes ;
	this->nodes = nodes ;
	this->classesCount = classesCount;
	this->network = nodes[0]->getNetwork();
}

CinqsNode *ClassDependentBranch::nextNode( Customer *c ) {
	for ( int i = 0 ; i < classesCount; i++ ) {
		if ( classes[i] == c->getclass() ) {
			return nodes[i] ;
		}
	}


//	Check::check( false, "ClassDependentBranch - class lookup failed.\n");
			//<<
			//"Associated node: " + owner.getName() + "\n" <<
			//"Offending customer class: " << c->getclass()) ;
	//return nodes[0]->getNetwork()->getNullNode();
	return Network::nullNode ;
}

void ClassDependentBranch::move( Customer *c ) {
	send( c, nextNode( c ) ) ;
}
