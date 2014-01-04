/***
 * Class that tests whether multiple threads adaptive profiling in parallel affect the z
 */

package vtf_tests;

public class AdaptiveProfilingParallelTest implements Runnable {

	private int id ;
	public static int threads = 1;
	public AdaptiveProfilingParallelTest(int id) {
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
	

	// The idea: threadToTriggerAdaptiveProfiling enters methodToBeAdaptiveProfiled 10 times,
	// after all other threads have already invoked it. These other threads are blocked in the method,
	// waiting for a 10 seconds timeout to expire, while threadToTriggerAdaptiveProfiling triggers the adaptive profiling.
	// We need to see whether, after the timeout expires, these threads will invoke the modified or
	// already loaded version of the methodToBeAdaptiveProfiled.
	// To do that, we have to use printouts in beforeMethodExit.
	public void run() {
		
		sleepSeconds(2);
		for (int i = 0; i<10+(2*threads); i++) {
			total += methodToBeAdaptiveProfiled(id);
		}
//		System.out.println(Thread.currentThread().getName() + " finished with " + total);
	}
	
	private double methodToBeAdaptiveProfiled(int i) {
		return i * Math.PI * Math.E;
	}

	public static void main(String[] args) {
		
		try {
			threads = Integer.parseInt(args[0]);
		} catch (Exception e) {
			threads = 1;
		}
		
		Thread[] threadsThatShouldNotUseVex = new Thread[threads];
		for (int i = 0; i<threads; i++) {
			threadsThatShouldNotUseVex[i] = new Thread(new AdaptiveProfilingParallelTest(i), "T"+ (i));
			threadsThatShouldNotUseVex[i].start();
		}
		
		try {
			for (int i = 0; i<threads; i++) {
				threadsThatShouldNotUseVex[i].join();
			}
			// no interrupted exceptions are thrown anyway
		} catch (InterruptedException ie) {
			ie.printStackTrace();
		}	
		System.out.println(threads + " threads finished, calculating a total of " + total);
	}


}
