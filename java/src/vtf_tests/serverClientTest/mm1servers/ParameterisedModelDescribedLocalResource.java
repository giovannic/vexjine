package vtf_tests.serverClientTest.mm1servers;

import vtf_tests.serverClientTest.tools.DistributionSampler;

public class ParameterisedModelDescribedLocalResource extends SimpleSingleCpuStressing {

	ParameterisedModelDescribedLocalResource(DistributionSampler jobSizeEstimator, int totalRequests) {
		super(jobSizeEstimator, totalRequests);
	}
	
	//TODO:PARAMETERIZE MODEL
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/mm1_q_local.jsimg",
    		replaceMethodBody=true)  	
    protected void doWork(double y, int iterations) {
		double x = y, a = 0;
		while (a < iterations) {
			a += Math.sin(x);
			x += Math.cos(x);
		}
	}
}
