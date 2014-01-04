package vtf_tests.serverClientTest.clients;

import vtf_tests.serverClientTest.Request;
import vtf_tests.serverClientTest.ServerInterface;
import vtf_tests.serverClientTest.Timers;

public class RequestingThread extends Thread {
	ServerInterface server;
	Timers timers;
	
	public RequestingThread(String name, ServerInterface server, Timers timers) {
		super(name);
		this.server = server;
		this.timers = timers;
	}
	
	protected void makeRequest(Request r) {
		long start = System.nanoTime();
		server.makeRequest(r);
    	long end = System.nanoTime();
		timers.addRequestTimes(end - start);	
	}
	
	public void run() {  
		Request r = new Request();
		makeRequest(r);
	}

}
