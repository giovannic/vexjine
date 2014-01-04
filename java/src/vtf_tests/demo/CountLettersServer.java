package vtf_tests.demo;

import vtf_tests.demo.loggers.Loggers;
import vtf_tests.demo.servers.ServerBehaviour;
import vtf_tests.serverClientTest.Request;
import vtf_tests.serverClientTest.mm1servers.SimpleSingleCpuStressing;
import vtf_tests.serverClientTest.tools.Exp;

public class CountLettersServer extends SimpleSingleCpuStressing {

	protected ServerBehaviour serviceBehaviour;
	public CountLettersServer(ServerBehaviour serviceBehaviour, int requests) {
		super(new Exp(1.0), requests);
		maximumRequests = requests;
		totalRequests = maximumRequests;
		this.serviceBehaviour = serviceBehaviour;
	}
    	
	public void resetTo(int requests) {
		maximumRequests = requests;
	}
	
    public void service(Request request) {
//    	long send = System.nanoTime();
//		System.out.println(send);
    	long startingTime = Loggers.requestResponseTimeLogger.onEventStart();    		
    	serviceBehaviour.service(request);
    	
    	Loggers.requestResponseTimeLogger.onEventEnd(startingTime);
    	
//    	System.out.println("requesting letter " + request.getChar());

    }

    public void makeRequest(Request request) {
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
		serviceBehaviour.cleanup();
	}
	
    private int totalRequests;
	private static int maximumRequests;
}
