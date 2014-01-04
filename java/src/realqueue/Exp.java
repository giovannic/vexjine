package realqueue ;

/**
 * Exponential distribution class. 
 * Copied from the Simulation and Modelling course toolkit.
 */
public class Exp  {
  private double rate ;

  public Exp( double r ) {
    rate = r ;
  }

  public double next() {
    return -Math.log( Math.random() ) / rate ;
  }

  public static double exp( double lam ) {
    return -Math.log( Math.random() ) / lam ;
  }
}
