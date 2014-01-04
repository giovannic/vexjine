package vtf_tests;

import virtualtime.EventNotifier;

public class AccelerationPaperSampleTest implements Runnable {
	boolean first;
	public static double total;
	private Object mutex;
	
	public AccelerationPaperSampleTest(Object mutex, boolean first) {
		this.first = first;
		this.mutex = mutex; 
	}
	
	
	@Override
	public void run() {
		if (first) {
			runFirstCode();
		} else {
			runSecondCode();
		}
	}
	
	private void runFirstCode() {
		double temp = 0.0;
		runShared(temp, 1);
		temp += methodBeforeFirst();
		System.out.println("first entering shared");
		runShared(temp, 150000000);
		temp += methodBeforeFirst();

		System.out.println(Thread.currentThread().getName() + " finished with " + temp);
	}
	
	private void runSecondCode() {
		
		double temp = 0.0;
		runShared(temp, 1);
		temp += methodBeforeSecond();
		System.out.println("second entering shared");
		runShared(temp, 12500000);
		temp += methodBeforeSecond();

		System.out.println(Thread.currentThread().getName() + " finished with " + temp);
	}

	private double methodBeforeFirst() {
		double temp = 1;
		for (int i =0 ; i < 3; i++) {
			temp += methodInner();	
		}
		return temp;
	}

	private double methodInner() {
		double temp = 1;
		for (int i =0 ; i < 10000000; i++) {
			temp += (temp / (i+1));	
		}
		return temp;
	}

	public double runShared(double temp, int iterations) {
		synchronized(mutex) {
			total += temp;
			for (int i =0 ; i < iterations; i++) {
				total += (total / (i+1));	
			}
		}
		return total;
	}
	
	private double methodBeforeSecond() {
		double temp = 1;
		for (int i =0 ; i < 5; i++) {
			temp += methodInner();	
		}
		return temp;
	}
	
	public synchronized double synchedMethod(double temp, int iterations) {
		total += temp;
		for (int i =0 ; i < iterations; i++) {
			total += (total / (i+1));	
		}
		return total;
	}
	public static void main(String[] args) {
		AccelerationPaperSampleTest test = new AccelerationPaperSampleTest(new Object(), true) ;
		test.runShared(0, 100000);
		test.synchedMethod(0, 100000);
		
//		while(EventNotifier._shouldMainIterationContinue()) {
//			int i =0;
//			i = i+1;
//			System.out.println("hello world!");
//		}
		/*
		Object obj = new Object();
		Thread first = new Thread(new AccelerationPaperSampleTest(obj, true), "FIRST");
		Thread second = new Thread(new AccelerationPaperSampleTest(obj, false), "SECOND");
		AccelerationPaperSampleTest.total = 0;
		long start, end;
		start = System.nanoTime();
		try {
			first.start();
			second.start();
			
			first.join();
			second.join();
		} catch (InterruptedException ie) {
			
		}
		end = System.nanoTime();
		System.out.println((end - start)/1e9);
		*/
	}

}
