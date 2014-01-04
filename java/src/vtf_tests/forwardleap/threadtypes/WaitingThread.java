package vtf_tests.forwardleap.threadtypes;

public class WaitingThread implements Runnable {
	private long timeout;
	private int iterations;
	
	public WaitingThread() {
		iterations = 1;
		timeout = 1000;
	}
	
	public WaitingThread(int iterations) {
		timeout = 1000;
		this.iterations = iterations;
	}
	
	public WaitingThread(long timeout, int iterations) {
		this.timeout = timeout;
		this.iterations = iterations;
	}
	
	@Override
	public void run() {
		for (int i = 0; i<iterations; i++) {
			synchronized (this) {
				try {
					this.wait(timeout);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}			
			}				
		}
		
	}
	
}
