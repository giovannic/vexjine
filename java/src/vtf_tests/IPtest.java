package vtf_tests;

import virtualtime.EventNotifier;
import virtualtime.EventNotifiervtonly;

public class IPtest {

	public static long getTime(String argv) {

		if( argv.equals("1")) {
			return System.nanoTime();
		} else if( argv.equals("2")) {
			return EventNotifiervtonly.getThreadVirtualTime();
		} else if( argv.equals("3")) {
			return EventNotifier.getThreadVirtualTime();
		} else if( argv.equals("4")) {
			return EventNotifiervtonly.getPapiRealTime();
		} else if( argv.equals("5")) {
			return EventNotifiervtonly.getJvmtiVirtualTime();
		} else if( argv.equals("6")) {
			return EventNotifiervtonly.getJvmtiRealTime();				
		} else { 
			return EventNotifiervtonly.getThreadVirtualTime();
		}
	}
	
	
	public static double myKernel() {
		Thread[] waitingThreads= new Thread[threads];
		
		for (int i =0 ; i <threads; i++) {
			waitingThreads[i] = new Thread(new parallelIpThread("a", true, 100)); 
		}
		 
		Thread t2 = new Thread(new parallelIpThread("b", false, 100));
		
		
		for (int i =0 ; i <threads; i++) {
			waitingThreads[i].start();
		}
		t2.start();
		
		try {
			for (int i =0 ; i <threads; i++) {
				waitingThreads[i].join();
			}
			t2.join();
		} catch (Exception e) {
			
		}
		return 1.0;
	}
	
	
	private final static int threads = 2;  
	private static void warmup() {
		myKernel();
		System.out.println("----------------------------------------------------------------------------------------------------------------");
	}
	
	public static void test() {
		myKernel();
	}
	
	public static void main(String[] argv) {
		
		for (int i = 0; i<2 ;i++) {
			warmup();
		}
		
		
		
		long start,end;
		start = System.nanoTime();
		test();
		end = System.nanoTime();
		System.out.println(end-start);
		
	}
	
}

class parallelIpThread implements Runnable {
	private String mode;
	private boolean toJoin;
	private long myLimit;
	
	parallelIpThread(String _mode, boolean _toJoin, long _myLimit) {
		mode = _mode;
		toJoin = _toJoin;
		myLimit = _myLimit;
		 
	}
	
	
	volatile static boolean barrier = false;
	public void run() {
//		System.out.println(Thread.currentThread().getName() + " started");
		double temp = 0;
		if (toJoin) {
			while (!barrier);
		} else {
			
			for (int i =0 ; i<20000000; i++) {
				temp += 1 + (temp) / (double)(i+1.0);
			}
			temp = temp + 1.0;
			
			barrier = true;
			
		}
//		System.out.println(Thread.currentThread().getName() + " " + temp +  " ended");
//		
	}
}
