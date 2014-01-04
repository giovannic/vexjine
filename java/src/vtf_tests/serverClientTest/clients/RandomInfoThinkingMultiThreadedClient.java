package vtf_tests.serverClientTest.clients;

import java.util.Random;

import vtf_tests.serverClientTest.ServerInterface;
import vtf_tests.serverClientTest.SynchronizedTimers;
import vtf_tests.serverClientTest.ThinkingBehaviour;

/***
 * The client spawns one thread for each request, then thinks and then spawns another
 * @author root
 *
 */
public class RandomInfoThinkingMultiThreadedClient extends ThinkingClient {

	private String generatedThreadPrefix;
	public RandomInfoThinkingMultiThreadedClient(ServerInterface server, ThinkingBehaviour thinkingBehaviour, SynchronizedTimers threadTimes, Random random) {
		super(server, thinkingBehaviour, threadTimes);
		this.random = random;
		generatedThreadPrefix = Thread.currentThread().getName();
	}
	
	public void issueRequests(int requests) throws InterruptedException {
		
		int request = 0;
        while (request++ < requests) {
        	Thread thd = new RandomRequestingThread(generatedThreadPrefix + request, server, threadTimes, random);
        	thd.setDaemon(true);	
            thd.start();           
            think();
        }     
	}
	
	protected Random random;

}