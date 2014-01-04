package vtf_tests.serverClientTest.mm1servers;

import java.util.Collections;
import java.util.Vector;

import vtf_tests.serverClientTest.Request;
import vtf_tests.serverClientTest.tools.DistributionSampler;

public class SimpleSingleCpuStressing extends MM1Server {
	
	public SimpleSingleCpuStressing(DistributionSampler jobSizeEstimator, int totalRequests) {
		super(totalRequests);
		this.jobSizeEstimator = jobSizeEstimator;
		this.averageServiceTime = -1;//78;
	}

	
	public void setDistributionSampler(DistributionSampler jobSizeEstimator) {
		this.jobSizeEstimator = jobSizeEstimator;
	}
	
	public long getAverageServiceTime() {
		if (averageServiceTime == -1) {
			Vector<Long> results = new Vector<Long>(10);

			for (int i = 100; i<500; i++) {
				doWork(Math.PI, i);	
			}		
			int count = 0;
			long start = 0 , end = 0;
			for (int i = 100000; i<51200000; i=i*2) {
				start = System.nanoTime();
				doWork(Math.PI, i);
				end = System.nanoTime();
				++count;
				results.add((end-start)/i);
			}

			Collections.sort(results);
			averageServiceTime = (Long)results.elementAt(4);
		}
		return averageServiceTime;
	}
	
	public void service(Request r) {
		int jobSize = (int)jobSizeEstimator.next();
		doWork(Math.PI, jobSize);
	}

	protected void doWork(double y, int iterations) {
		double x = y, a = 0;
		while (a < iterations) {
			a += Math.sin(x);
			x += Math.cos(x);
		}
	}
	
	protected DistributionSampler jobSizeEstimator;
	protected long averageServiceTime;
}





