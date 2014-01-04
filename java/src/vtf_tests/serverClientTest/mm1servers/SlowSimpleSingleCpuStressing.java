package vtf_tests.serverClientTest.mm1servers;

import vtf_tests.serverClientTest.tools.DistributionSampler;

public class SlowSimpleSingleCpuStressing extends SimpleSingleCpuStressing {

	private static final double factor = 1.333; 
	public SlowSimpleSingleCpuStressing(DistributionSampler jobSizeEstimator, int totalRequests) {
		super(jobSizeEstimator, totalRequests);
	}
	
	public long getTheoreticalValue(double mi, double lambda) {
		return (long)(1e9 / ((mi/factor) - lambda));
	}
	
	public int[] generateInputValues(long averageServiceTime, double lambda, int totalValues, double from, double to) {
		int[] inputValues = new int[totalValues];

		double maxValue = (1000000000.0 / ((lambda + 1.0/to) * factor * averageServiceTime));

		if (from == 0.0) {
			double step = (maxValue - from)/ totalValues;
			for (int i=0 ; i<totalValues; i++) {
				inputValues[i] = (int)(from + step * (i+1)); 
			}
		} else {
			double lowValue = (1000000000.0 / ((lambda + 1.0/from) * factor * averageServiceTime));

			double step = (maxValue - lowValue)/ totalValues;
			for (int i=0 ; i<totalValues; i++) {
				inputValues[i] = (int)(lowValue + step * (i+1)); 
			}
		}
		return inputValues;
	}
	
	@virtualtime.Accelerate (speedup = 1.333)
	protected void doWork(double y, int iterations) {
		double x = y, a = 0;
		while (a < iterations) {
			a += Math.sin(x);
			x += Math.cos(x);
		}
	}
}