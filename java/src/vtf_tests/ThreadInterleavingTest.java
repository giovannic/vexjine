package vtf_tests;

import java.util.Random;

public class ThreadInterleavingTest implements Runnable {
	private int id;
	private static double pi1, pi2, pi3, pi4;
	private static long globalStartingTime = System.currentTimeMillis();
	ThreadInterleavingTest(int id) {
		this.id = id;
	}
	
	private double before1() {
		Random r = new Random();
		double x,y;
		int counter = 0;
		double iterations = 10000000.0;
		for (int i = 0; i<iterations; i++) {
			x = r.nextDouble();
			y = r.nextDouble();
			if (Math.pow(x, 2) + Math.pow(y, 2) <= 1) {
				++counter;
			}
		}
		return 4*(double)counter/iterations;
	}
	
	private double before2() {
		Random r = new Random();
		double x,y;
		int counter = 0;
		double iterations = 20000000.0;
		for (int i = 0; i<iterations; i++) {
			x = r.nextDouble();
			y = r.nextDouble();
			if (Math.pow(x, 2) + Math.pow(y, 2) <= 1) {
				++counter;
			}
		}
		return 4*(double)counter/iterations;
	}

	private double during1() {
		Random r = new Random();
		double x,y;
		int counter = 0;
		double iterations = 20000000.0;
		for (int i = 0; i<iterations; i++) {
			x = r.nextDouble();
			y = r.nextDouble();
			if (Math.pow(x, 2) + Math.pow(y, 2) <= 1) {
				++counter;
			}
		}
		return 4*(double)counter/iterations;

	}
	
	 private double during2() {
		Random r = new Random();
		double x,y;
		int counter = 0;
		double iterations = 100000.0;
		for (int i = 0; i<iterations; i++) {
			x = r.nextDouble();
			y = r.nextDouble();
			if (Math.pow(x, 2) + Math.pow(y, 2) <= 1) {
				++counter;
			}
		}
		return 4*(double)counter/iterations;
	}
	
	 public void comparePis(double mypi, double hispi) {
		synchronized(ThreadInterleavingTest.class) {
			if (hispi == 0) {
				System.out.println(Thread.currentThread().getName() + " calculated first pi to be " + mypi + " at " + (System.currentTimeMillis()-globalStartingTime) + "ms");
			} else {
				System.out.println(Thread.currentThread().getName() + " was the second to calculate his pi to be " + mypi + " at " + (System.currentTimeMillis()-globalStartingTime) + "ms");
			}
			
			if (Thread.currentThread().getName().equals("T1")) {
				pi3 = during1();
			} else {
				pi4 = during1();
			}
		}
		
	}
	
	
	private void t1Routine() {
		pi1 = before1();
		comparePis(pi1, pi2);
		pi1 += before1();
	}

	private void t2Routine() {
		pi2 = before2();
		comparePis(pi2, pi1);
		pi2 += before1();
	}
	
	public void run() {
		double value;
		
		if (id == 1) {
			t1Routine();
	
		} else {
			t2Routine();

		}
	}
	
	
	public static void main(String[] args) {
		long end;
		
		Thread t1 = new Thread(new ThreadInterleavingTest(1), "T1");
		Thread t2 = new Thread(new ThreadInterleavingTest(2), "T2");
		t1.start();
		t2.start();
		try {
			t1.join();
			t2.join();
		} catch (InterruptedException ie) {
			ie.printStackTrace();
		}
		end = System.currentTimeMillis();
		System.out.println("Total duration " + (end - globalStartingTime) + "ms");
	}
}
