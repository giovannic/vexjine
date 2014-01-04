package queueing ;

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
  
  public static void main(String[] args) {
	  Exp ex = new Exp(Double.parseDouble(args[0]));
	  
	  long sum = 0;
	  int iterations  = Integer.parseInt(args[1]);
	  for (int i =0; i<iterations;i++) {
		  sum += (long)ex.next();
	  }
	  
	  sum /= iterations;
	  System.out.println("Average: " + (sum));
	  
	  
  }
  
}
