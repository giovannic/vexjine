#include "examples/MM1Sim.h"


  MM1Sim::MM1Sim() {
    Network::initialise() ;
    Delay serveTime = new Delay( new Exp( 4 ) ) ;

    Source source    = new Source( "Source", new Exp( 2 ) ) ;
    QueueingNode mm1 = new QueueingNode( "MM1", serveTime, 1 ) ;
    Sink sink        = new Sink( "Sink" ) ;
  
    source.setLink( new Link( mm1 ) ) ;
    mm1.setLink( new Link( sink ) ) ;

    simulate() ;

    Network::logResult( "Utilisation", mm1.serverUtilisation() ) ;
  }

   bool MM1Sim::stop() {
    return Network::completions == 100000 ;
  }

   static void MM1Sim::main( string args *) {
    new MM1Sim() ;
    new MM1Sim() ;
    new MM1Sim() ;
    Network::displayResults( 0.01 ) ;
  }
