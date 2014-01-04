package vtf_tests;

import java.util.concurrent.Callable;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import vtf_tests.forwardleap.threadtypes.CpuRunningThread;

public class FuturesCallablesTest {
	private static final int NTHREADS = 4;
	public static long startTime = System.nanoTime();
	
	
	public static void test() {
		ExecutorService executor = Executors.newFixedThreadPool(NTHREADS);
		executor.submit(new CpuRunningThread(100000));
		executor.submit(new CpuRunningThread(10000));
		executor.submit(new CpuRunningThread(100002));
		executor.submit(new CpuRunningThread(100002));
		System.out.println("eduw");
//		executor.submit(new MyCallable());
		executor.submit(new CpuRunningThread(100000));
		executor.submit(new CpuRunningThread(10000));
		executor.submit(new CpuRunningThread(100002));
		executor.submit(new CpuRunningThread(100002));
//		executor.submit(new MyCallable());
		executor.shutdown();
	}
	
	public static void main(String[] args) {

		ExecutorService executor = Executors.newFixedThreadPool(NTHREADS);
		List<Future<Long>> list = new ArrayList<Future<Long>>();
		for (int i = 0; i < 1; i++) {
			Callable<Long> worker = new MyCallable();
			//Future<Long> submit = executor.submit(worker);
			Future<Long> submit = Executors.newSingleThreadExecutor().submit(worker);
			list.add(submit);
		}
		
		try {
			Thread.sleep(4000);
		} catch (InterruptedException e) {
			e.printStackTrace();
		} 
		long sum = 0;
		System.out.println(list.size());
		// Now retrieve the result
		for (Future<Long> future : list) {
			try {
				sum += future.get();
			} catch (InterruptedException e) {
				e.printStackTrace();
			} catch (ExecutionException e) {
				e.printStackTrace();
			}
		}
		System.out.println((System.nanoTime() -  startTime)/1e9 + " " + sum);
		executor.shutdown();
		
		test();
	}

}



class MyCallable implements Callable<Long> {
	@Override
	public Long call() throws Exception {
		long sum = 0;
		for (long i = 0; i <= 100; i++) {
			sum += i;
		}
		System.out.println(Thread.currentThread().getName() + " teleiwsa");

		return sum;
	}

} 