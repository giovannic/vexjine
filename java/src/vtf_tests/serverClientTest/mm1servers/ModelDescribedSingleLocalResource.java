package vtf_tests.serverClientTest.mm1servers;

import vtf_tests.serverClientTest.Request;
import vtf_tests.serverClientTest.tools.Exp;

public class ModelDescribedSingleLocalResource extends SimpleSingleCpuStressing {

	private static int maximumRequests;
	public ModelDescribedSingleLocalResource(int tRequests) {
		super(new Exp(1.0), tRequests);
		averageServiceTime = 200000000;
		totalRequests = maximumRequests;
	}
	

	public long getAverageServiceTime() {
		return averageServiceTime;
	}
	
	public int[] generateInputValues(long averageServiceTime, double lambda, int totalValues, double from, double to) {
		int[] values = new int[totalValues];
		for (int i=0; i<totalValues; i++) {
			values[i] = 1;
		}
		return values;
	}

	
	public double[] generateArrivalRates(int totalValues, double from, double to) {
		double[] rates = new double[totalValues];
		if (from < 0.2) {
			from = 0.2;
		}
		double lowArrivalRate = (5.0 - 1.0/from);
		double highArrivalRate = (5.0 - 1.0/to);
		
		double step = (highArrivalRate - lowArrivalRate) / (double)totalValues;
		for (int i=0; i<totalValues; i++) {
			rates[i] = lowArrivalRate + step * (i+1);
		}
		return rates;
	}
	  	    
    public void start() {
    	synchronized(this) {
    		totalRequests = maximumRequests;
    	}    	
    }
    
	// Average service time is 200ms and for avg waiting time 400ms, the expected
	// response time is 400ms. Parameters should be m = 5.0 and l = 2.5 respectively
    @virtualtime.ModelPerformance(
   		jmtModelFilename="/homes/nb605/VTF/src/models/mm1_local_server_200.jsimg",
   		replaceMethodBody=true)
    public void service(Request request) {
    	System.out.println("This should not be printed");
    }
    		
    private int totalRequests;
    public void makeRequest(Request request) {
    	service(request);
    	synchronized(this) {
    		if (--totalRequests <= 0) {
    			//System.out.println("NOTIFY TO FINISH SIMULATION AT REQS: " + totalRequests);
    			this.notifyAll();
    		}
    	}
	}
    
	public void stop() {
		synchronized(this) {
			while(totalRequests > 0) {
				try {
					wait();
				} catch (InterruptedException ie) {
					ie.printStackTrace();					
				}
			}
		}
		//System.out.println("FINISHING SIMULATION");
	}
}

