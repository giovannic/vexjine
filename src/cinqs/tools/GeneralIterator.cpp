#include "tools/GeneralIterator.h"

//
// A more useful list iteration mechanism than is provided in the
// standard Java platform.  It allows straightforward implementation
// of alternative insertion/removal schemes, e.g. ordered, random etc.
//
::interface GeneralIterator {
  ::bool canAdvance() ;

  ::void advance() throws EmptyListException ;
  
  ::Object getValue() throws EmptyListException ;

  ::void replaceValue( Object d ) throws EmptyListException ;

  ::void add( Object o ) ;
 
  ::void remove() throws EmptyListException ;
