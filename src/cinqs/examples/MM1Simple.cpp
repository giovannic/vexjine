#include "examples/MM1Simple.h"

  Resource server = new Resource() ;
  int pop = 0 ;

     MM1Simple::Arrival( double t ) : Resource(t) {

    }
     void MM1Simple::invoke() {
      schedule( new Arrival( now() + Exp.exp( 2 ) ) ) ;
      pop++ ;
      if ( server.resourceIsAvailable() ) {
        server.claim() ;
        schedule( new Departure( now() + Exp.exp( 4 ) ) ) ;
      }
    }
  }

     MM1Simple::Departure( double t ) : Resource(t) {

    }
     void MM1Simple::invoke() {
      pop-- ;
      if ( pop > 0 ) 
        schedule( new Departure( now() + Exp.exp( 4 ) ) ) ;
      else
        server.release() ;
    }
  }

   void MM1Simple::resetMeasures() {
    server.resetMeasures() ;
  }

   bool MM1Simple::stop() {
    return now() > 100000 ;
  }

  MM1Simple::MM1Simple() {
    schedule( new Arrival( now() + Exp.exp( 2 ) ) ) ;
    simulate( 10000 ) ;
    Logger::logResult( "Utilisation", server.utilisation() ) ;
  }

   static void MM1Simple::main( string args *) {
    new MM1Simple() ;
    new MM1Simple() ;
    new MM1Simple() ;
    Logger::displayResults( 0.01 ) ;
  }
