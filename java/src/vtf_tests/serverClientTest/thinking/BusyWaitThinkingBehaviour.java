package vtf_tests.serverClientTest.thinking;

import vtf_tests.serverClientTest.tools.DistributionSampler;

public class BusyWaitThinkingBehaviour extends SleepingThinkingBehaviour {

	static double iterationsPerNs = 0;
	
	public BusyWaitThinkingBehaviour(DistributionSampler thinkTime) {
		super(thinkTime);
		localVar = 1;
		if (iterationsPerNs == 0) {
			calculateIterationsPerNs();
		}
	}
	

	@Override
	public void think() throws InterruptedException {
		double nextThinkTime = sampleThinkingTime();
		long start = System.nanoTime();
		simulateExecutionOnCpuFor(nextThinkTime);
		long end = System.nanoTime();
		System.out.println("it took " + (end-start) + " ns to busy-think for " + nextThinkTime + " ns");
	}

	private double localVar;
	
	
	/*
	 * Busy waiting simulation support method:
	 * calculate the iterations that will be executed
	 * during the simulation of each nanosecond
	 */
	void calculateIterationsPerNs() {

//		iterationsPerNs = 0.283156;
	
	//	iterationsPerNs = 0.805468;
	

		long totalstart = System.nanoTime();
				
		double totalTime;
		double previousIterationsPerNs = 0;
		iterationsPerNs = 1.0;
		long iterations = 10000000;
		while (true) {
			long start = System.nanoTime();
			simulateExecutionOnCpuFor(iterations);
			long end = System.nanoTime();
		
			totalTime = end - start;

			if (previousIterationsPerNs == 0) {
				previousIterationsPerNs = totalTime / iterations;
			} else {
				iterationsPerNs = totalTime / (iterationsPerNs * iterations);
				if (Math.abs(previousIterationsPerNs - iterationsPerNs)/previousIterationsPerNs > 0.05) {
					previousIterationsPerNs	= (previousIterationsPerNs + iterationsPerNs)/2;
					iterations *= 2;
				} else {
					iterationsPerNs	= (previousIterationsPerNs + iterationsPerNs)/2;
					break;
				}
			}
		}

		int count = 0;
		double factor = 0.0;
		for (int i = 1000; i<15000000; i *= 2) {
			iterations = i;
			long start = System.nanoTime();
			simulateExecutionOnCpuFor(iterations);
			long end = System.nanoTime();
			
			factor += (double)i/(double)((end - start)/1e9 * 1000);
			++count;
		}
		factor /= count;
		iterationsPerNs *= factor;
		System.out.println("final " + iterationsPerNs);
		
//		gettimeofday(&totalend, NULL);
//		timersub(&totalend, &totalstart, &totaldiff);
//		totalTime = totaldiff.tv_sec * 1000000000 + totaldiff.tv_usec * 1000;
//		////cout << "Total simulateExecutionOnCpuFor testing time: " << totalTime / 1000000000.0 << endl;
	}


	/*
	 * Busy waiting for duration of service time
	 */
	void simulateExecutionOnCpuFor(double time) {
		double iterations = time * iterationsPerNs;
		for (double j=0; j<iterations; j++) {
			localVar += j;
		}
	}

	
}
