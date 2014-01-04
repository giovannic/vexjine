#ifndef TOOLS_EMPTYLISTEXCEPTION_H
#define TOOLS_EMPTYLISTEXCEPTION_H

#include <exception>



class EmptyListException : public std::exception {
	virtual const char* what() const throw() {
		return "EmptyListException happened";
	}
} ;

#endif
