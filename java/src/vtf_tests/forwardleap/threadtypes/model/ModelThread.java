package vtf_tests.forwardleap.threadtypes.model;

public class ModelThread implements Runnable {
	private double temp;
	private int iterations;
	public ModelThread() {
		temp = 1.0;
		iterations = 100000;
	}
	
	public ModelThread(int iterations) {
		temp = 1.0;
		this.iterations = iterations;
	}
	
	protected void execute() {
		
	}
	
    public boolean requiresResource() {
    	return true;
    }
    
    protected int resourceUsageInMsPerIteration() {
    	return 0;
    }
	
	protected void updateTemp(int millionIterations) {
    	double doSthIterations = millionIterations * 1000000;
	    for (double i = 1; i<doSthIterations; i++) {
	    	temp += Math.pow(1.0 + 1.0/i, i); 
	    }
	    temp /= doSthIterations;
    }
	
	@Override
	public void run() {
		for (int i = 0; i<iterations; i++) {
			execute();
		}
	}
	
	public int getApproximateExecutionTime() {
		return iterations * resourceUsageInMsPerIteration();
	}
}
