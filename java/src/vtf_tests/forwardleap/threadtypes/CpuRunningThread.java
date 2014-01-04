package vtf_tests.forwardleap.threadtypes;

public class CpuRunningThread implements Runnable {
	private double temp;
	private int iterations;
	public CpuRunningThread() {
		temp = 1.0;
		iterations = 100000;
	}
	
	public CpuRunningThread(int iterations) {
		temp = 1.0;
		this.iterations = iterations;
	}
	
	@Override
	public void run() {
		for (int i = 1; i<iterations; i++) {
			temp += Math.pow(1.0 + 1.0/i, i);				
		}
		
	}
	
}
