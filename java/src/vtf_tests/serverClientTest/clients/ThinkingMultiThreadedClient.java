package vtf_tests.serverClientTest.clients;

import vtf_tests.serverClientTest.ServerInterface;
import vtf_tests.serverClientTest.SynchronizedTimers;
import vtf_tests.serverClientTest.ThinkingBehaviour;

/***
 * The client spawns one thread for each request, then thinks and then spawns another
 * @author root
 *
 */
public class ThinkingMultiThreadedClient extends ThinkingClient {

	public ThinkingMultiThreadedClient(ServerInterface server, ThinkingBehaviour thinkingBehaviour, SynchronizedTimers threadTimes) {
		super(server, thinkingBehaviour, threadTimes);
	}
		
	public void issueRequests(int requests) throws InterruptedException {
		
		int request = 0;
        while (request++ < requests) {
        	Thread thd = new RequestingThread("threadRequest" + request, server, threadTimes);
        	thd.setDaemon(true);	
            thd.start();
            think();
        }     
	}

}


