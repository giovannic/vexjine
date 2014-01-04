#ifndef TOOLS_LIST_H
#define TOOLS_LIST_H

#include "Event.h"
//#include "Customer.h"
class Customer;
//#include "GeneralIterator.h"
#include "EmptyListException.h"

#include <cstddef>

#include <string>



// TODO: IMPROVEMENT: could template that, but would require framework wide change
typedef Customer *Object;

class ListNode {
public:
	Object data;
	ListNode *next;

	void init(Object o, ListNode *nextNode) {
		data = o;
		next = nextNode;
	}
	ListNode( Object o ) {
		init(o, NULL);
	}

	ListNode( Object o, ListNode *nextNode ) {
		init(o, nextNode);
	}

	Object getObject() { return data; }
	ListNode *getNext() { return next; }
};

class ListIterator;
class List {

public:
	List() ;
	List( const std::string &s ) ;
	virtual ~List();

	Object removeFromFront();
	Object removeFromBack();

	Object first() ;
	void insertAtFront( Object insertItem ) ;
	void insertAtBack( Object insertItem ) ;

	Object last() ;

	void remove( Object o ) ;
	void remove( Event *o );
	bool isEmpty() ;
	void print() ;
	ListIterator *getIterator() ;

	void setFirstAndLastEqualTo(ListNode *newNode) {
		firstNode = lastNode = newNode ;
	}

	void setFirstEqualTo(ListNode *newNode) {
		firstNode = newNode ;
	}

	void setLastEqualTo(ListNode *newNode) {
		lastNode = newNode ;
	}

	ListNode *getFirstNode() {
		return firstNode;
	}

protected:
	ListNode *firstNode;
	ListNode *lastNode;
	std::string name ;
};
#include <iostream>


class ListIterator {

public:
	ListIterator(List *l) {
		p = NULL ;
		q = l->getFirstNode() ;
		this->l = l;
	}

	bool canAdvance() {
		return !( q == NULL ) ;
	}

	void advance() {//throw
//		if ( q == NULL )
//			assert(2 == 1) ;
		p = q ;
		q = q->next ;
	}

	Object getValue() { //throws EmptyListExceptionception {
//		if ( q == NULL )
//			assert(2 == 1) ;
		return q->data ;
	}

	void replaceValue( Object d ) { //throws EmptyListExceptionception {
//		if ( q == NULL )
//			assert(2 == 1) ;
		q->data = d ;
	}

	void add( Object o ) {
		ListNode *newNode = new ListNode( o, q ) ;
		if ( ( p == NULL ) && ( q == NULL ) ) {
			l->setFirstAndLastEqualTo(newNode);
			q = newNode ; }
		else
			if ( p == NULL ) {
				l->setFirstEqualTo(newNode);
				p = newNode ; }
			else {
				p->next = newNode ;
				p = newNode ;
				if ( q == NULL )
					l->setLastEqualTo(newNode);}
	}

	void remove() { //throws EmptyListExceptionception {
//		if ( q == NULL )
//			assert(2 == 1) ;
		if ( p == NULL ) {
			//ListNode *temp = q;
			l->setFirstEqualTo(q->next) ;
			//delete temp;temp = NULL;
		} else {
			//.ListNode *temp = q;
			p->next = q->next ;
//			delete temp;temp = NULL;
		}
		q = q->next ;
		std::cout << "edw"<<std::endl;
		if ( q == NULL )
			l->setLastEqualTo(p) ;
	}
private:
	ListNode *p, *q ;
	List *l;
};
#endif
