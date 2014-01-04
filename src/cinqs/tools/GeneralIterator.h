#ifndef TOOLS_GENERALITERATOR_H
#define TOOLS_GENERALITERATOR_H

class Customer;

typedef Customer *Object;

class GeneralIterator {
public:
	virtual bool canAdvance() = 0;
	virtual void advance() = 0;
	virtual Object getValue() = 0;
	virtual void replaceValue( Object d ) = 0;
	virtual void add( Object o ) ;
	virtual void remove() = 0;
};

#endif
