package vtf_tests.serverClientTest.clients;

import java.util.Random;

import vtf_tests.serverClientTest.Request;
import vtf_tests.serverClientTest.ServerInterface;
import vtf_tests.serverClientTest.Timers;

public class RandomRequestingThread extends RequestingThread {
	ServerInterface server;
	Timers timers;
	Random random;
	
	public RandomRequestingThread(String name, ServerInterface server, Timers timers, Random random) {
		super(name, server, timers);
		this.random = random;
	}
	
	public void run() {  
		Request r = new Request(random);
		makeRequest(r);
	}
}