package vtf_tests;

public class MonitorLockingManagingTest implements Runnable {
	boolean first;
	public static double total;
	private Object mutex;

	public static int threadsCount = 2;
	public MonitorLockingManagingTest(Object mutex, boolean first) {
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
		double temp = 1.0;
		temp += runSharedMutex(temp, 80000000 / threadsCount);
		System.out.println(Thread.currentThread().getName() + " finished with " + temp);
	}
	
	private void runSecondCode() {
		
		double temp = Math.random();
		temp += MonitorLockingManagingTest.synchedMethod(temp,  200000000 / threadsCount);
		System.out.println(Thread.currentThread().getName() + " finished with " + temp);
	}

	public double runSharedMutex(double temp, int iterations) {
		synchronized(mutex) {
			total += temp;
			for (int i =0 ; i < iterations; i++) {
				total += (total / (i+1));	
			}
		}
		return total;
	}

	public static synchronized double synchedMethod(double temp, int iterations) {
		total += temp;
		for (int i =0 ; i < iterations; i++) {
			total += (total / (i+1));	
		}
		System.out.println(temp + " " + iterations + " " + total);
		return total;
	}
	
	public static void performanceTest1() {
		Object m = new Object();
		double temp = 1.0;
		int iterations = 1;

		long start = System.nanoTime();
		for (int j=0; j<200000000; j++) {
			synchronized(m) {
				total += temp;
				for (int i =0 ; i < iterations; i++) {
					total += (total / (i+1));	
				}
			}
		}
		long end = System.nanoTime();
		System.out.println((double)(end-start)/1e9);
	}
	
	public static void programLegend() {
		System.out.println("Syntax error: java vtf_tests.MonitorLockingManaging <perftest|methodsync|externalsync> <# threads>");
		System.exit(-1);	
	}

	public static void main(String[] args) {
		if (args.length != 2) {
			programLegend();
		}
		
		boolean explicitSynchronizedLoop = true;
		if (args[0].equals("perftest")) {
			MonitorLockingManagingTest.performanceTest1();
			return;
		} else if (args[0].equals("methodsync")) {
			explicitSynchronizedLoop = false;
		} else if (args[0].equals("externalsync")) {
			explicitSynchronizedLoop = true;
		} else {
			programLegend();
		}
		threadsCount = 1;
		try {
			threadsCount = Integer.parseInt(args[1]);
		} catch (Exception ie) {
			threadsCount = 1;
		}
		Thread[] thread = new Thread[threadsCount];
		Object mutex = new Object();
		for (int i = 0 ; i<threadsCount; i++) {
			thread[i] = new Thread(new MonitorLockingManagingTest(mutex, explicitSynchronizedLoop), "thread" + i);
		}
		
		long start = System.nanoTime();
		for (int i = 0 ; i<threadsCount; i++) {
			thread[i].start();
		}
		for (int i = 0 ; i<threadsCount; i++) {
			try {
				thread[i].join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		long end = System.nanoTime();
		System.out.println((double)(end-start)/1e9);
	}
}
