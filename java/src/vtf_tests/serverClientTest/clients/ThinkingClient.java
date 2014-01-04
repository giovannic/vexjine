package vtf_tests.serverClientTest.clients;

import vtf_tests.demo.loggers.Loggers;
import vtf_tests.serverClientTest.ClientInterface;
import vtf_tests.serverClientTest.Request;
import vtf_tests.serverClientTest.ServerInterface;
import vtf_tests.serverClientTest.ThinkingBehaviour;
import vtf_tests.serverClientTest.Timers;
import vtf_tests.serverClientTest.tools.*;

/***
 * Single client issuing requests to the server one at a time
 * @author root
 */
public class ThinkingClient implements ClientInterface {
	public ThinkingClient(ServerInterface server, ThinkingBehaviour thinkingBehaviour, Timers threadTimes) {
		this.server = server;
		this.threadTimes = threadTimes;
		this.thinkingBehaviour = thinkingBehaviour;
	}
	
	protected void think() throws InterruptedException {
		thinkingBehaviour.think();
	}
	
	public void setThinkingBehaviour(ThinkingBehaviour thinkingBehaviour) {
		this.thinkingBehaviour = thinkingBehaviour;
	}
	
	public void issueRequests(int requests) throws InterruptedException {
		Request r = new Request();
		for (int i = 0; i<requests; i++) {
			think();
			long start = System.nanoTime();
			server.makeRequest(r);
			threadTimes.addRequestTimes(System.nanoTime() - start);
		}

	}
	
	public Timers getTimes() {
		return threadTimes;
	}
	
	protected DistributionSampler thinkTime;	
	protected ServerInterface server;
	protected Timers threadTimes;
	protected ThinkingBehaviour thinkingBehaviour;
}
