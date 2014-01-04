package vtf_tests.forwardleap.threadtypes;

public class SleepingThread implements Runnable {
	private long timeout;
	private int iterations;
	
	public SleepingThread() {
		iterations = 1;
		timeout = 1000;
	}
	
	public SleepingThread(int iterations) {
		timeout = 1000;
		this.iterations = iterations;
	}
	
	public SleepingThread(long timeout, int iterations) {
		this.timeout = timeout;
		this.iterations = iterations;
	}
	
	@Override
	public void run() {
		for (int i = 0; i<iterations; i++) {
			try {
				Thread.sleep(timeout);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}							
		}
		
	}
	
}
