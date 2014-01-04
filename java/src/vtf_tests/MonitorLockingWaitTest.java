package vtf_tests;

public class MonitorLockingWaitTest implements Runnable {
	boolean normBarrier;
	public static double total;
	
	public Object finishedSync;
	private static int finished = 0;
	public static int threadsCount = 2;
	public static int stupidCount = 0;
	public MonitorLockingWaitTest(Object mutex, boolean first) {
		this.normBarrier = first;
		finishedSync = mutex;
	}
	
	
	@Override
	public void run() {
		runFirstCode();
	}
	
	
	private void runFirstCode()  {
		double temp = 1.0;
		if (normBarrier) {
			temp += runSharedMutex(temp, 400000000 / threadsCount);
			barrier();
		} else {
			temp += runSharedMutex(temp, 20000000 / threadsCount);
			regressingBarrier();
		}
//		System.out.println(Thread.currentThread().getName() + " finished with " + temp);
	}

	private void barrier() {
		synchronized(finishedSync) {
			++finished;
			while (finished != threadsCount) {
				try {
					finishedSync.wait();
				} catch (InterruptedException ie) {
					ie.printStackTrace();
				}
			}
			finishedSync.notifyAll();
		}
	}

	private static synchronized void regressingBarrier() {
		if (++stupidCount % 5 == 0) {
			++finished;

		} else {
			try {
				long timeout = (long)(Math.random() * 50);
				Thread.sleep(timeout);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			regressingBarrier();
		}
	}
	
	private double runSharedMutex(double temp, int iterations) {
		total += temp;
		for (int i =0 ; i < iterations; i++) {
			total += (total / (i+1));	
		}
		return total;
	}
	
	public static void programLegend() {
		System.out.println("Syntax error: java vtf_tests.MonitorWaitManaging <barrier|counter> <# threads>");
		System.exit(-1);	
	}
	
	public static void main(String[] args) {
		if (args.length != 2) {
			programLegend();
		}
		boolean normalBarrier = true;
		if (args[0].equals("perftest")) {
			MonitorLockingManagingTest.performanceTest1();
			return;
		} else if (args[0].equals("counter")) {
			normalBarrier = false;
		} else if (args[0].equals("barrier")) {
			normalBarrier = true;
		} else {
			programLegend();
		}
		threadsCount = 2;
		try {
			threadsCount = Integer.parseInt(args[1]);
		} catch (Exception ie) {
			threadsCount = 2;
		}
		Thread[] thread = new Thread[threadsCount];
		Object mutex = new Object();
		for (int i = 0 ; i<threadsCount; i++) {
			thread[i] = new Thread(new MonitorLockingWaitTest(mutex, normalBarrier), "thread" + i);
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
