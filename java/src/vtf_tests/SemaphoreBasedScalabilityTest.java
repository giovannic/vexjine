package vtf_tests;

import java.util.concurrent.Semaphore;

public class SemaphoreBasedScalabilityTest implements Runnable {
	Semaphore semaphore;
	double temp;
	public SemaphoreBasedScalabilityTest(Semaphore semaphore) {
		this.semaphore = semaphore;
		temp = 0.0;
	}
	
	double runLoops(int loopCount) {
		double temp = 0.0;
    	double i;
  		for(i = 0; i<loopCount; i++) {
  					temp += (i * (8.0+i)) / (i+1.0);

  		}
  		temp += 1;
  		return temp;
	}
	
	@Override
	public void run() {
		 try {
			semaphore.acquire();
			temp += runLoops(100000000);			
			semaphore.release();
			System.out.println(Thread.currentThread().getName() + " finished with " + temp);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
	
	
	public static void main(String[] args) {
		int totalThreads = 1;
		int permits = 1;
		try {
			totalThreads = Integer.parseInt(args[1]);
			try {
				permits = Integer.parseInt(args[0]); 
			} catch (Exception ie) {
				System.out.println("Syntax:  <permits> <total threads>");
				return;
			}
		} catch (Exception ie) {
			System.out.println("Syntax:  <permits> <total threads>");
			return;
		}
		
		long start, end;
		start = System.nanoTime();
		Semaphore sem = new Semaphore(permits);
		Thread[] threads = new Thread[totalThreads];
		for (int i = 0; i<totalThreads; i++) {
			threads[i] = new Thread(new SemaphoreBasedScalabilityTest(sem), "thread" + i);
		}
		for (int i = 0; i<totalThreads; i++) {
			threads[i].start();
		}
		for (int i = 0; i<totalThreads; i++) {
			try {
				threads[i].join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		end = System.nanoTime();
		System.out.println("Total time: " + (end - start)/1e9);
	}

}
