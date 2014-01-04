package vtf_tests.demo;

import java.util.Random;

import vtf_tests.demo.servers.MySqlDBCached;
import vtf_tests.demo.servers.MySqlDBCachedWithInsert;
import vtf_tests.serverClientTest.ServerInterface;
import vtf_tests.serverClientTest.SynchronizedTimers;
import vtf_tests.serverClientTest.ThinkingBehaviour;
import vtf_tests.serverClientTest.clients.RandomInfoThinkingMultiThreadedClient;
import vtf_tests.serverClientTest.clients.ThinkingClient;
import vtf_tests.serverClientTest.thinking.SleepingThinkingBehaviour;
import vtf_tests.serverClientTest.tools.Exp;

public class MySqlCompleteDemoWithInsert extends MySqlCompleteDemo {
	protected static int warmupRequests = DemoConfiguration.warmupRequests;
	protected static int normalRequests = DemoConfiguration.normalRequests;
	public static int totalRequests = (warmupRequests + normalRequests);
	
	protected static CacheImpl completeCacheImpl;
	protected static CountLettersServer completeServerImpl;
	protected static SleepingThinkingBehaviour completeThinkingBehavriourImpl;
	protected static SqlDriverInfo completeSqlDriverInfo;
	
	public MySqlCompleteDemoWithInsert(String name) {
		super(name);
	}
	
	
	public static synchronized void disableBarrier() {
		barrierCount = 2;
	}
	public static synchronized void enableBarrier() {
		barrierCount = 0;
	}
	protected static int barrierCount = 0;
	public static synchronized void barrierSelectAndInsertStreams() {
		++barrierCount;
		if (barrierCount < 2) {
			while (barrierCount < 2) {
				try {
					MySqlCompleteDemoWithInsert.class.wait();					
				} catch (InterruptedException e) {

				}
			}
			barrierCount = 0;
		} else {
			MySqlCompleteDemoWithInsert.class.notifyAll();
		}
	}
	
	public static void testTemplate(ServerInterface server, ThinkingBehaviour thinkingBehaviour) {
		SynchronizedTimers requestTimes = new SynchronizedTimers();
		ThinkingClient client = new RandomInfoThinkingMultiThreadedClient(server, thinkingBehaviour, requestTimes, new Random());
		server.start();
		try {
			client.issueRequests(warmupRequests);
			barrierSelectAndInsertStreams();
			
			
			client.getTimes().reset();
			client.issueRequests(normalRequests);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		server.stop();
		
	}

	protected void setUp() throws Exception {
		DemoConfiguration.setLoggers();

		completeCacheImpl = new ArrayCache(DemoConfiguration.defaultCacheSize);
		completeSqlDriverInfo = new SqlDriverInfo("com.mysql.jdbc.Driver", "jdbc:mysql://irmabunt.doc.ic.ac.uk:3306/tpcwdb?user=nb605&password=admin&useReadAheadInput=false", true);
		completeServerImpl = new CountLettersServer(new MySqlDBCached(completeCacheImpl, completeSqlDriverInfo), totalRequests);
		completeThinkingBehavriourImpl = new SleepingThinkingBehaviour(new Exp(0.2));
		
		// fill in cache during setUp period
		
		int originalwarmup = warmupRequests; 
		warmupRequests = 3;
		int originalnormal = normalRequests;
		normalRequests = 36; 
		completeServerImpl.resetTo(warmupRequests+normalRequests);
		
		disableBarrier();
		testTemplate(completeServerImpl, completeThinkingBehavriourImpl);
		enableBarrier();
		
		warmupRequests = originalwarmup;
		normalRequests = originalnormal;
		completeServerImpl.resetTo(warmupRequests+normalRequests);
		
	}
	
	public static void testCompleteWithInsertWithMySql() {	// 5ms avg sleeping time + cache implemented
		ModelDemo.startDbInsertingClients(new MySqlDBCachedWithInsert(completeCacheImpl, completeSqlDriverInfo), new SleepingThinkingBehaviour(new Exp(0.1))); //0.1428571
		testTemplate(completeServerImpl, completeThinkingBehavriourImpl);

		ModelDemo.insertingIntoDBThread.interrupt();
		barrierSelectAndInsertStreams();

	}	
	
	protected void tearDown() {
		completeServerImpl.cleanup();
		DemoConfiguration.printLoggers();
	}
	
	public static void main(String[] args) {	// 5ms avg sleeping time + cache implemented
		ModelDemo.startDbInsertingClients(new MySqlDBCachedWithInsert(completeCacheImpl, completeSqlDriverInfo), new SleepingThinkingBehaviour(new Exp(0.1))); //0.1428571
		testTemplate(completeServerImpl, completeThinkingBehavriourImpl);
	}	
	
}
