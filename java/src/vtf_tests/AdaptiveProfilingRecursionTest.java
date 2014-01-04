/***
 * Class that tests whether adaptive profiling techniques affect the methods already 
 * loaded by other threads if the method is recursive
 */

package vtf_tests;

public class AdaptiveProfilingRecursionTest implements Runnable {

	private int id ;
	public static int threads = 1;
	public AdaptiveProfilingRecursionTest(int id) {
		this.id = id;
	}
	public static double total = 0.0;
	private void sleepSeconds(long seconds) {
		try {
			Thread.sleep(seconds*1000);
		} catch (InterruptedException ie) {
			ie.printStackTrace();
		}	
	}
	

	public void run() {
		if (id == 1) {
			sleepSeconds(2);
			for (int i = 0; i<10+(2*threads); i++) {
				total += recursiveMethodToBeAdaptiveProfiled(id, 10);
			}
		} else {
			total += recursiveMethodToBeAdaptiveProfiled(id, 10);	
		}
		System.out.println(Thread.currentThread().getName() + " finished with " + total);
	}
	
	private double recursiveMethodToBeAdaptiveProfiled(int i, int depth) {
		if (depth <= 0) {	// to cover negative depths
			if (id != 1) {
				sleepSeconds(3 + (int)Math.log(threads));
			}
			return i * Math.PI * Math.E;
		} else {
			return recursiveMethodToBeAdaptiveProfiled(i, depth - 1);
		}
	}

	public static void main(String[] args) {
		
		try {
			threads = Integer.parseInt(args[0]);
		} catch (Exception e) {
			threads = 1;
		}
		Thread threadToTriggerAdaptiveProfiling = new Thread(new AdaptiveProfilingRecursionTest(1), "T1");
		Thread[] threadsThatShouldNotUseVex = new Thread[threads];
		
		for (int i = 0; i<threads; i++) {
			threadsThatShouldNotUseVex[i] = new Thread(new AdaptiveProfilingRecursionTest(i+2), "T"+ (i+2));
			threadsThatShouldNotUseVex[i].start();
		}
		
		threadToTriggerAdaptiveProfiling.start();
		
		try {
			threadToTriggerAdaptiveProfiling.join();
			for (int i = 0; i<threads; i++) {
				threadsThatShouldNotUseVex[i].join();
			}
			// no interrupted exceptions are thrown anyway
		} catch (InterruptedException ie) {
			ie.printStackTrace();
		}	
		
	}


}
