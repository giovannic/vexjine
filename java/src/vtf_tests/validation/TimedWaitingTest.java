package vtf_tests.validation;

import java.util.Random;

import vtf_tests.TestUtils;

import junit.framework.TestCase;

public class TimedWaitingTest extends TestCase {
	
	public void testMillisecondsWait() {
		TimedWaitingTest lock = new TimedWaitingTest();
		Random rand = new Random();
		long randomTimeout = rand.nextInt(10000);
		long start = System.currentTimeMillis();
		synchronized(lock) {
			try {
				lock.wait(randomTimeout);
			} catch (InterruptedException ie) {
				ie.printStackTrace();
			}
		}
		long end = System.currentTimeMillis();
		assertTrue("Timeout of " + randomTimeout + "ms", TestUtils.checkRatio((end-start), randomTimeout, 0.01));		
	}
	
	
	
	public void testNanosecondsWait1() {
		TimedWaitingTest lock = new TimedWaitingTest();
		Random rand = new Random();
		long randomTimeout = rand.nextInt(10000);
		int randomNsTimeout = rand.nextInt(1000000);
		long start = System.nanoTime();
		synchronized(lock) {
			try {
				lock.wait(randomTimeout, randomNsTimeout);
			} catch (InterruptedException ie) {
				ie.printStackTrace();
			}
		}
		long end = System.nanoTime();
		assertTrue("Timeout of " + randomTimeout + "ms and " + randomNsTimeout + "ns", TestUtils.checkRatio((double)(end-start)/1000000.0, (double)(randomTimeout + randomNsTimeout/1000000.0), 0.01));		
	}
	
	
	
	public void testNanosecondsWait2() {
		TimedWaitingTest lock = new TimedWaitingTest();
		Random rand = new Random();
		int randomNsTimeout = rand.nextInt(1000000);
		long start = System.nanoTime();
		synchronized(lock) {
			try {
				lock.wait(0, randomNsTimeout);
			} catch (InterruptedException ie) {
				ie.printStackTrace();
			}
		}
		long end = System.nanoTime();
		// ALLOW 10% error - since this is only ns
		assertTrue("Timeout of " + randomNsTimeout + "ns", TestUtils.checkRatio((end-start), randomNsTimeout, 0.1));		
	}
}
