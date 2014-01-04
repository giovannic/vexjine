package vtf_tests;

import java.util.Random;
import java.util.concurrent.locks.LockSupport;


public class ParkTest {
	
	public static boolean testTimeOut = false;
	public static long startingTestTime = 0; 
	static {
		startingTestTime = System.nanoTime();
	}
	
	public class ParkingThread implements Runnable {	
		@Override
		public void run() {
			while (!testTimeOut) {
				System.out.println(Thread.currentThread().getName() + " parking");
				LockSupport.park(this);
				System.out.println(Thread.currentThread().getName() + " unparking");
			}
		}
		
	}

	public static double myLoops(long loopCount) {
    	double temp = 0.0;
    	double i;

  		for(i = 0; i<loopCount; i++) {
  			temp += (i * (8.0+i)) / (i+1.0);		
  		}	 
  		temp += 1;
  		return temp;
	}

	public static void testIterativeIndefinitePark(String[] args) {
		
		int threads = Integer.parseInt(args[0]);
		
		Thread[] thread = new Thread[threads];
		ParkTest j = new ParkTest();

		for (int i = 0; i<threads; i++) {
			thread[i] = new Thread(j.new ParkingThread(), "ParkingThread" + i);
			thread[i].start();
		}
		
		try {
			Thread.sleep(1000);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		
		for (int i = 0; i<5; i++) {
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			for (int k = 0; k<threads; k++) {
				LockSupport.unpark(thread[k]);
			}
		}
		ParkTest.testTimeOut = true;
		
		for (int i = 0; i<threads; i++) {
			try {
				LockSupport.unpark(thread[i]);
				thread[i].join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}	
	}
	
	
	public class TwParkingThread implements Runnable {
		private long timeout;
		public TwParkingThread(long timeout) {
			this.timeout = timeout;
		}
		
		@Override
		public void run() {
			System.out.println(Thread.currentThread().getName() + " parking for " + timeout/1e9);
			LockSupport.parkNanos(timeout);
			System.out.println(Thread.currentThread().getName() + " unparking after " + timeout/1e9 + " at " + (System.nanoTime()-startingTestTime)/1e9);
		}
		
	}
	public static void testFewTimedWaitingParks(String[] args) {
		int threads = Integer.parseInt(args[0]);
		
		Thread[] thread = new Thread[threads];
		ParkTest j = new ParkTest();

		for (int i = 0; i<threads; i++) {
			long timeout = (long)(i+1) *  2000000000;
			thread[i] = new Thread(j.new TwParkingThread(timeout), "ParkingThread" + i);
			thread[i].start();
		}
				
		for (int i = 0; i<threads; i++) {
			try {
				thread[i].join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}	
	}
	
	
	public static void testAnInterruptedTimedWaitingPark(String[] args) {
		int threads = Integer.parseInt(args[0]);
		
		Thread[] thread = new Thread[threads];
		ParkTest j = new ParkTest();

		for (int i = 0; i<threads; i++) {
			long timeout = (long)(i+1) *  2000000000;
			thread[i] = new Thread(j.new TwParkingThread(timeout), "ParkingThread" + i);
			thread[i].start();
		}
		try {
			Thread.sleep(1000);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		
		for (int i = 0; i<threads; i++) {
			thread[i].interrupt();
		}
		
		for (int i = 0; i<threads; i++) {
			try {
				thread[i].join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}	
	}
	
	
	public static void testAnUnparkedTimedWaitingPark(String[] args) {
		int threads = Integer.parseInt(args[0]);
		
		Thread[] thread = new Thread[threads];
		ParkTest j = new ParkTest();

		for (int i = 0; i<threads; i++) {
			long timeout = (long)(i+1) *  2000000000;
			thread[i] = new Thread(j.new TwParkingThread(timeout), "ParkingThread" + i);
			thread[i].start();
		}
		try {
			Thread.sleep(1000);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		
		for (int i = 0; i<threads; i++) {
			LockSupport.unpark(thread[i]);
		}
		
		for (int i = 0; i<threads; i++) {
			try {
				thread[i].join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}	
	}
	
	
	public class SimpleParkingThread implements Runnable {	
		@Override
		public void run() {
			System.out.println(Thread.currentThread().getName() + " parking");
			LockSupport.park(this);
			System.out.println(Thread.currentThread().getName() + " unparking");
		
		}
		
	}
	public static void testUnparkBefore(String[] args) {
		int threads = Integer.parseInt(args[0]);
		System.out.println("BEWARE THIS SHOULD HANG");
		
		Thread[] thread = new Thread[threads];
		ParkTest j = new ParkTest();
		
		for (int i = 0; i<threads; i++) {
			thread[i] = new Thread(j.new SimpleParkingThread(), "ParkingThread" + i);
			LockSupport.unpark(thread[i]);
			thread[i].start();
		}

		for (int i = 0; i<threads; i++) {
			try {
				thread[i].join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}		
	}
	
	
	public class SimpleWaitingFirstParkingThread implements Runnable {	
		@Override
		public void run() {
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			System.out.println(Thread.currentThread().getName() + " parking");
			LockSupport.park(this);
			System.out.println(Thread.currentThread().getName() + " unparking");
		
		}
		
	}
	public static void testCorrectUnparkBefore(String[] args) {
		int threads = Integer.parseInt(args[0]);
		
		Thread[] thread = new Thread[threads];
		ParkTest j = new ParkTest();

		
		for (int i = 0; i<threads; i++) {
			thread[i] = new Thread(j.new SimpleParkingThread(), "ParkingThread" + i);
			thread[i].start();
		}

		for (int i = 0; i<threads; i++) {
			LockSupport.unpark(thread[i]);
		}

		for (int i = 0; i<threads; i++) {
			try {
				thread[i].join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}	
	}
	
	
	public static void testGcAndParking(String[] args) {
		int threads = Integer.parseInt(args[0]);
		
		Thread[] thread = new Thread[threads];
		ParkTest j = new ParkTest();

		for (int i = 0; i<threads; i++) {
			thread[i] = new Thread(j.new SimpleParkingThread(), "ParkingThread" + i);
			thread[i].start();
		}
		Thread loopThread = new Thread(new Runnable() {
			@Override
			public void run() {
				double temp = 0.0;
				for (int i = 0; i<1000000; i++) {
					temp += i;
					for (int j1 = 0; j1<(2000000+new Random().nextInt()); j1++) {
						temp += new Random().nextDouble();
					}
				}
				System.out.println(temp);	
			}
		}, "loopsThread");
		loopThread.start();
		
		double temp = 0.0;
		for (int i = 0; i<5000000; i++) {
			temp += i;
			for (int j1 = 0; j1<(2000000+new Random().nextInt()); j1++) {
				temp += new Random().nextDouble();
			}
		}
		System.out.println(temp);
		
		for (int i = 0; i<threads; i++) {
			LockSupport.unpark(thread[i]);
		}

		for (int i = 0; i<threads; i++) {
			try {
				thread[i].join();
				
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}	
		try {
			loopThread.join();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
	
	
	public static void main(String[] args) {
		int experimentId = 1;
		try {
			experimentId = Integer.parseInt(args[1]);
		} catch (Exception e) {
			System.out.println("Syntax error: java vtf_tests.ParkTest <threads> <exp_id 1 - 7>");
			System.exit(-1);
		}
		
		switch (experimentId) {
			case 1: testIterativeIndefinitePark(args); break;
			case 2: testFewTimedWaitingParks(args); break;
			case 3: testAnInterruptedTimedWaitingPark(args); break;
			case 4: testUnparkBefore(args); break;	// SHOULD HANG
			case 5: testAnUnparkedTimedWaitingPark(args);break; 
			case 6: testGcAndParking(args); break;
			case 7: testCorrectUnparkBefore(args); break;
			default: break;
			
		}
		
	}

		
}
