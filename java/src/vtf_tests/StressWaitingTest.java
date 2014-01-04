package vtf_tests;


public class StressWaitingTest implements Runnable {
	private int threadId;
	private static int threads;
	StressWaitingTest(int _id) {
		threadId = _id;
	}
/*
	public void run() {
		
		int waitingTime = 1;
		for (int i =0 ; i<1000000; i++) {
//			while (waitingTime < 10) {
				try {
					synchronized(StressWaitingTest.class) {	
						StressWaitingTest.class.wait(waitingTime);
					}
				} catch (InterruptedException x) {
					System.out.println("in run() - interrupted while sleeping");
				}	
//				waitingTime += 1;
//			}
		}

	}
*/

	/*
	public void run() {
		int waitingTime = threadId * 10;
		for (int i = 0; i<10; i++) {
			synchronized(StressWaitingTest.class) {	
				try {
					StressWaitingTest.class.wait();			
					if (threadId == 1) {
						StressWaitingTest.class.wait(waitingTime);
						StressWaitingTest.class.notifyAll();
					}
					System.out.println(Thread.currentThread().getName() + " finishing");
				} catch (InterruptedException x) {
					System.out.println(Thread.currentThread().getName() + " interrupted while waiting for " + waitingTime);
				}	
			}
		}
	}
	*/
	private static int loops = 0;
	private static int counter = 0;
	private static long startingTime = System.nanoTime();
	
	public void run() {
//		long end, start = System.nanoTime();
		
		for(int i=0;i<loops;i++) {
			synchronized (StressWaitingTest.class) {	
				try {
					if(++counter == threads) {
						counter = 0;
						StressWaitingTest.class.notifyAll();
					} else {
//						StressWaitingTest.class.wait(100000);
						StressWaitingTest.class.wait();
					}

				}
				catch (InterruptedException e) {}
			}
		}

//		end = System.nanoTime();
//System.out.println(Thread.currentThread().getName() + " " + ((end-startingTime)/1000000) + "ms: " + (end-start)/ 1000000000.0);
		
	}

	public static void main(String[] args) {
		long end, start = System.nanoTime();
		if (args.length < 1) {
			System.err.println("Syntax: java StressWaitingTest <threads>");
			System.exit(0);
		}
		
		threads = 32; 
		loops = Integer.parseInt(args[0]);
		Thread[] t = new Thread[threads];


		for (int i=0; i<threads; i++) {
			t[i] = new Thread(new StressWaitingTest(i+1));
			t[i].start();
		}
		
		try {
			for (int i=0; i<threads; i++) {
				t[i].join();
			}		
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		
		end = System.nanoTime();
		System.out.println((end-start)/ 1000000000.0);
	}
}
