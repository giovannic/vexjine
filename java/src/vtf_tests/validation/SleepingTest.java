package vtf_tests.validation;

import java.util.Random;

import vtf_tests.TestUtils;

import junit.framework.TestCase;

public class SleepingTest extends TestCase {

	public void testMillisecondsSleep() {

		Random rand = new Random();
		long randomTimeout = rand.nextInt(10000);
		long start = System.currentTimeMillis();
		try {
			Thread.sleep(randomTimeout);
		} catch (InterruptedException ie) {
			ie.printStackTrace();
		}
		long end = System.currentTimeMillis();
		assertTrue("Timeout of " + randomTimeout + "ms", TestUtils.checkRatio((end-start), randomTimeout, 0.01));		
	}



	public void testNanosecondsSleep1() {
		Random rand = new Random();
		long randomTimeout = rand.nextInt(10000);
		int randomNsTimeout = rand.nextInt(1000000);
		long start = System.nanoTime();

		try {
			Thread.sleep(randomTimeout, randomNsTimeout);
		} catch (InterruptedException ie) {
			ie.printStackTrace();
		}

		long end = System.nanoTime();
		assertTrue("Timeout of " + randomTimeout + "ms and " + randomNsTimeout + "ns", TestUtils.checkRatio((double)(end-start)/1000000.0, (double)(randomTimeout + randomNsTimeout/1000000.0), 0.01));		
	}



	public void testNanosecondsSleep2() {

		Random rand = new Random();
		int randomNsTimeout = rand.nextInt(1000000);
		long start = System.nanoTime();
		try {
			Thread.sleep(0, randomNsTimeout);
		} catch (InterruptedException ie) {
			ie.printStackTrace();
		}

		long end = System.nanoTime();
		// ALLOW 10% error - since this is only ns
		assertTrue("Timeout of " + randomNsTimeout + "ns", TestUtils.checkRatio((end-start), randomNsTimeout, 0.1));		
	}


	class SleepingThreadSubclass extends Thread {

	}

	class SleepingThreadSubSubclass extends SleepingThreadSubclass {

	}

	public void testSubclassSleep() {
		Random rand = new Random();
		long randomTimeout = rand.nextInt(1000);
		int randomNsTimeout = rand.nextInt(1000000);
		try {
			new SleepingThreadSubclass().sleep(randomTimeout, randomNsTimeout);
		} catch (InterruptedException ie) {
			ie.printStackTrace();
		}

		long start = System.nanoTime();

		try {
			new SleepingThreadSubclass().sleep(randomTimeout, randomNsTimeout);
		} catch (InterruptedException ie) {
			ie.printStackTrace();
		}

		long end = System.nanoTime();
		assertTrue("KNOWN PROBLEM: invoking sleep from subclass of java/lang/Thread like new SubclassOfJavaLangThread().sleep(...) instead of Thread.sleep(...)", TestUtils.checkRatio((double)(end-start)/1000000.0, (double)(randomTimeout + randomNsTimeout/1000000.0), 0.01));		
	} 
	public void testSubSubclassSleep() {

		Random rand = new Random();
		long randomTimeout = rand.nextInt(1000);
		int randomNsTimeout = rand.nextInt(1000000);
		long start = System.nanoTime();

		try {
			new SleepingThreadSubSubclass().sleep(randomTimeout, randomNsTimeout);
		} catch (InterruptedException ie) {
			ie.printStackTrace();
		}

		long end = System.nanoTime();
		assertTrue("KNOWN PROBLEM: invoking sleep from subclass of a subclass java/lang/Thread like new SubSubclassOfJavaLangThread().sleep(...) instead of Thread.sleep(...)", TestUtils.checkRatio((double)(end-start)/1000000.0, (double)(randomTimeout + randomNsTimeout/1000000.0), 0.01));		
	}

}

