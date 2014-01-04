#include "tools/List.h"
#include "network/Customer.h"
#include <cassert>
#include <iostream>
using namespace std;
//-----------------------------------------------------------------------------
//
//  An implementation of Lists with a usable list iterator
//  Implemented by Tony Field
//
//  Built on the example in Figure 22.3 in Deitel and Deitel's book on
//  Java2 - How to Program.
//
//-----------------------------------------------------------------------------

List::List( const string &s ) {
	name = s;
	firstNode = lastNode = NULL;
}

List::List() {
	name = "list" ;
	firstNode = lastNode = NULL;
}

List::~List() {
	ListNode *iterator = firstNode;
	ListNode *temp;
	while (iterator != NULL) {
		temp = iterator;
		iterator = iterator->next;
		delete temp;
	}
}

Object List::first() {
	if ( isEmpty() ) {
		assert(false);
		return NULL;
	} else {
		return firstNode->data;
	}
}

void List::insertAtFront( Object insertItem ) {
	if ( isEmpty() )
		firstNode = lastNode = new ListNode( insertItem );
	else
		firstNode = new ListNode( insertItem, firstNode );
}

Object List::last() {
	if ( isEmpty() ) {
		assert(false);
		return NULL;
	} else {
		return lastNode->data;
	}
}

void List::insertAtBack( Object insertItem ) {
	if ( isEmpty() )
		firstNode = lastNode = new ListNode( insertItem );
	else
		lastNode = lastNode->next = new ListNode( insertItem );
}

void List::remove( Object o ) {
	ListIterator *it = this->getIterator() ;
	while ( it->canAdvance() && it->getValue() != o ) {	// NOTE: replaced equals with ==
		it->advance() ;
	}
	if ( it->getValue() == o ) {// NOTE: replaced equals with ==
		it->remove() ;
	} else {
		cout <<  "Error: attempt to remove non-existent object from list" << endl ;
	}
	delete it;
}

void List::remove( Event *o ) {
	ListIterator *it = this->getIterator() ;
	while ( it->canAdvance() && it->getValue() != (Object)o ) {// NOTE: replaced equals with ==
		it->advance() ;
	}
	if ( it->getValue() == (Object)o ) {// NOTE: replaced equals with ==
		it->remove() ;
	} else {
		cout <<  "Error: attempt to remove non-existent object from list" << endl ;
	}
	delete it;
}

Object List::removeFromFront() { //throws EmptyListExceptionception
	Object removeItem = NULL;

	if ( isEmpty() )
		assert(2 == 1);

	removeItem = firstNode->data;

	if ( firstNode == lastNode ) {
		delete firstNode;
		firstNode = lastNode = NULL;
	} else {
		ListNode *temp = firstNode;
		firstNode = firstNode->next;
		delete temp;
	}

	return removeItem;
}

Object List::removeFromBack() {//throw
	Object removeItem = NULL;

	if ( isEmpty() )
		assert(2 == 1);

	removeItem = lastNode->data;

	if ( firstNode == lastNode ) {
		if (lastNode != NULL) {
			delete lastNode;
			lastNode = NULL;
		}
		firstNode = lastNode = NULL;
	} else {
		ListNode *current = firstNode;

		while ( current->next != lastNode ) {
			current = current->next;
		}

		if (lastNode != NULL) {
			delete lastNode;
		}
		lastNode = current;
		current->next = NULL;
	}

	return removeItem;
}

bool List::isEmpty() {
	return firstNode == NULL;
}

void List::print() {
	if ( isEmpty() ) {
		cout <<  "Empty " << name << endl;
		return;
	}

	cout <<  "The " << name << " is: " << endl;

	ListNode *current = firstNode;

	while ( current != NULL ) {
		cout <<  current->data->toString() << " " << endl;
		current = current->next;
	}

	cout <<  "\n" << endl;
}

ListIterator *List::getIterator() {
	return new ListIterator(this) ;
}




