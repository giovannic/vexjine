package virtualtime.generators;

import java.util.Iterator;
import java.util.Random;
import java.util.Vector;

public class PrimeNumberGenerator {
	private int currentPrime;
	
	static int[] ioIds = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 39, 41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107};
	Vector alreadyFoundPrimes = null;
	public PrimeNumberGenerator() {
		currentPrime = 107;
		
		alreadyFoundPrimes = new Vector();
		for (int i =0; i <ioIds.length; i++) {
			alreadyFoundPrimes.add(ioIds[i]);	
		}
		
		
	}
	
	public int getNext() {
		if (currentPointer > 0) {
			return (Integer)alreadyFoundPrimes.elementAt(++currentPointer);
		} 
		currentPrime += 2;
		int number;
		Iterator iter = alreadyFoundPrimes.iterator();
		for (; iter.hasNext();) {
			number = (Integer)iter.next();
			if (currentPrime % number == 0) {
				return getNext();
			}
		}			
		alreadyFoundPrimes.add(currentPrime);
		return currentPrime;
	}
		
	public int getCurrentPrime() {
		if (currentPointer > 0) {
			return (Integer)alreadyFoundPrimes.elementAt(currentPointer);
		}
		return currentPrime;
	}
	
	private int getExistingRandomPrime() {
		Random r = new Random();
		return (Integer)alreadyFoundPrimes.elementAt(r.nextInt(alreadyFoundPrimes.size()));
	}
	
	private void decomposePrime(long numToBeFactored) {
		/*
		IntegerArray ia = new IntegerArray(10);
		
		int number = 1;
		Iterator iter = alreadyFoundPrimes.iterator();
		for (; iter.hasNext() && number*number<=numToBeFactored; ) {
			number = (Integer)iter.next();
            if (numToBeFactored % number == 0) {
            	ia.add(number);
                //System.out.print(number + " * ");
            }
        }*/

	}


	private int currentPointer = -1;		// getNextAfterPointer will return primes already on the list
		
	public void setCurrentPointerTo(int previous) {
		currentPointer = 0;
		Iterator iter = alreadyFoundPrimes.iterator();
		int number = 0;
		for (; iter.hasNext() && number < previous; ) {
			number = (Integer)iter.next();
			if (number == previous) {
				return;
			}
			++currentPointer; 
		}
		currentPointer = -1;	// not found
	}
	
	public void clearCurrentPointer() {
		currentPointer = -1;
	}
	
	/**
	 * @param argsSystem.out.println("max ll " + temp);
	 */
	public static void main(String[] args) {		
		PrimeNumberGenerator pm = new PrimeNumberGenerator();
		long start, end;
		int primesToGenerate = Integer.parseInt(args[0]);
		start = System.nanoTime();
		for (int i=0; i<primesToGenerate; i++) {
			System.out.println(pm.getNext());
		}
		end = System.nanoTime();
		System.out.println("Produced " + primesToGenerate + " prime numbers in " + ((end-start)/1000000000.0) + " seconds");
		
		System.out.println("Largest prime produced "+ pm.getCurrentPrime());
		
		double dtemp = Double.MAX_VALUE;
		int count = 0;
		System.out.println("max dd " + dtemp);
		while (dtemp > 0) {
			dtemp = dtemp / (long)pm.getCurrentPrime();
//			System.out.println("poses fores dd " + dtemp );
			count++;
		}	
		System.out.println("poses fores sto double" + count);
		
		
		pm.setCurrentPointerTo(571);
		for (int i =0; i<10; i++) {
			System.out.println(pm.getNext());
		}
		
		pm.clearCurrentPointer();
		
		for (int i =0; i<10; i++) {
			System.out.println(pm.getNext());
		}
		
		long temp = Long.MAX_VALUE;
		count = 0;
		System.out.println("max ll " + temp);
		while (temp > 0) {
			temp = temp / (long)pm.getCurrentPrime();
//			System.out.println("poses fores ll " + temp );
			count++;
		}	
		System.out.println("poses fores sto long " + count);
		start = System.nanoTime();
		for (int i = 0; i<Integer.parseInt(args[0]); i++) {
			long ran = (long)pm.getExistingRandomPrime()*(long)pm.getExistingRandomPrime()*(long)pm.getExistingRandomPrime();
			pm.decomposePrime(ran);
		}
		end = System.nanoTime();
		System.out.println("Produced " + primesToGenerate + " prime numbers in " + ((end-start)/1000000000.0) + " seconds");
		
	}

}
