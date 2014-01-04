package vtf_tests.concurrent;

import java.util.concurrent.Semaphore;

public class Locking {
	
	public static void main(String[] args) {
		int permits = 1;
		int threads = 5;
		if (args.length == 2) {
			permits = Integer.parseInt(args[0]);
			threads = Integer.parseInt(args[1]);
		} else {
			throw new IllegalArgumentException("Syntax: <permits> <threads>");
		}
		
		Semaphore sem = new Semaphore(permits);
		
		
		Thread[] threadsAll = new Thread[threads];
		long start = System.currentTimeMillis();
		for (int i = 0; i<threads; i++) {
			threadsAll[i] = new Thread(new ThreadMethod(sem), "sem" + i);
			threadsAll[i].start();
		}
		
		for (int i = 0; i<threads; i++) {
			try {
				threadsAll[i].join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		long end = System.currentTimeMillis();
		System.out.println((end-start)/1e3);
	}
}


class ThreadMethod implements Runnable {
	private Semaphore sem;
	public ThreadMethod(Semaphore sem) {
		this.sem = sem;
	}
	
	@Override
	public void run() {
		try {
			sem.acquire();
			Thread.sleep(1000);
			
		} catch (InterruptedException e) {
			e.printStackTrace();
			
		} finally {
			sem.release();
			
		}

	}
	
}