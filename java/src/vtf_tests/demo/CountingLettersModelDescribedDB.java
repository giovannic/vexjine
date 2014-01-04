package vtf_tests.demo;

import org.junit.Assert;

import vtf_tests.demo.loggers.Loggers;
import vtf_tests.serverClientTest.Request;
import vtf_tests.serverClientTest.mm1servers.SimpleSingleCpuStressing;
import vtf_tests.serverClientTest.tools.Exp;

public class CountingLettersModelDescribedDB extends SimpleSingleCpuStressing {

	public CountingLettersModelDescribedDB(int requests) {
		super(new Exp(1.0), requests);
		maximumRequests = requests;
		totalRequests = maximumRequests;
	}
    	
	public void resetTo(int requests) {
		maximumRequests = requests;
	}
	
	@virtualtime.ModelPerformance(
   		jmtModelFilename="/homes/nb605/VTF/src/models/demo_select.jsimg",
   		replaceMethodBody=true)
    public void service(Request request) {
    	Assert.fail("This method body should be replaced during runtime");
    }

    
    public void makeRequest(Request request) {
    	Loggers.selectRequestStartTimestamp.onEventStart();
    	service(request);
		
    	synchronized(this) {
    		if (--totalRequests <= 0) {
    			this.notifyAll();
    		}
    	}
	}
	    
    public void start() {
    	synchronized(this) {
    		totalRequests = maximumRequests;
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
	}
	
	public void cleanup() {
		
	}
	
    private int totalRequests;
	private static int maximumRequests;
}
