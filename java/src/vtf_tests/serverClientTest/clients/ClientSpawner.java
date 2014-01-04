package vtf_tests.serverClientTest.clients;

import vtf_tests.serverClientTest.ClientInterface;
import vtf_tests.serverClientTest.SynchronizedTimers;
import vtf_tests.serverClientTest.Timers;

/***
 * Spawns many threads together, which in turn issue requests to the server 
 * @author root
 */
public class ClientSpawner implements ClientInterface {
	public ClientSpawner(ClientTypeGenerator clientTypeGenerator, int requestsPerClient, SynchronizedTimers timers) {
		this.clientTypeGenerator = clientTypeGenerator;
		this.requestsPerClient = requestsPerClient;
		this.threadTimers = timers;
	}
	
	public void issueRequests(int threads) {
		int thread = 0;
	
        while (thread++ < threads) {
            Thread thd = new Thread("concurrentlyRequestingThread" + thread) {
                public void run() {
                	ClientInterface client = clientTypeGenerator.generateClient();
                	try {
						client.issueRequests(requestsPerClient);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
                	threadTimers.mergeWithTimer(client.getTimes());
                }
            };
            thd.setDaemon(true);	
            thd.start();
        }
	}

	public Timers getTimes() {
		return threadTimers;
	}
	
	protected ClientTypeGenerator clientTypeGenerator;
	protected int requestsPerClient;
	protected SynchronizedTimers threadTimers;
}
