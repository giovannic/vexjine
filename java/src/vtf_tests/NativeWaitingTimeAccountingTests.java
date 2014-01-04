package vtf_tests;

import vtf_tests.forwardleap.threadtypes.CpuRunningNativeWaitingThread;
import vtf_tests.forwardleap.threadtypes.CpuRunningThread;

public class NativeWaitingTimeAccountingTests {
	private static long start;
	private static long end;	
	private static void startTimer() {
		start = System.nanoTime();	
	}
	private static void endTimer() {
		end = System.nanoTime() - start;
		System.out.println("Total time: " + (end/ 1e9) + " ms");	
	}
		
	
	
	//Scenario 1: 2 threads running loops concurrently - 1 gets into native waiting for waiting 200ms before doing so.
	//				is the accounted virtual time the same
	public static void scenario1() {
		
		Thread normalThread = new Thread(new CpuRunningThread(3500000), "loopsThread1");
		Thread normalThread2 = new Thread(new CpuRunningNativeWaitingThread(3500000), "nwThread2");
		startTimer(); 
		normalThread.start();
		normalThread2.start();
		try {
			normalThread.join();
			normalThread2.join();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		endTimer();
	}
	
	
	public static void scenario2(int threads) {
		
		Thread normalThread = new Thread(new CpuRunningThread(3500000), "loopsThread1");
		Thread[] nwThreads = new Thread[threads];
		for (int i = 0; i<threads; i++) {
			nwThreads[i] = new Thread(new CpuRunningNativeWaitingThread(3500000), "nwThread" + i);
		}
		startTimer(); 
		normalThread.start();
		for (int i = 0; i<threads; i++) {
			nwThreads[i].start();
		}
		try {
			normalThread.join();
			for (int i = 0; i<threads; i++) {
				nwThreads[i].join();
			}		
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		endTimer();
	}
	
	// Syntax: NativeWaitingTimeAccountingTests <threads>
	public static void main(String[] args) {
		int threads = Integer.parseInt(args[0]);
//		scenario1();
		scenario2(threads);
		
		
	}
}
