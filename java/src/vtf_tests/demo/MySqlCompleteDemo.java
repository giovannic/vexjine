package vtf_tests.demo;

import java.util.Random;

import vtf_tests.demo.servers.MySqlDBCached;
import vtf_tests.demo.servers.MySqlDBCachedAccelerated;
import vtf_tests.demo.servers.ServerBehaviour;
import vtf_tests.serverClientTest.ServerInterface;
import vtf_tests.serverClientTest.SynchronizedTimers;
import vtf_tests.serverClientTest.ThinkingBehaviour;
import vtf_tests.serverClientTest.clients.RandomInfoThinkingMultiThreadedClient;
import vtf_tests.serverClientTest.clients.ThinkingClient;
import vtf_tests.serverClientTest.thinking.SleepingThinkingBehaviour;
import vtf_tests.serverClientTest.tools.Exp;
import junit.framework.TestCase;

public class MySqlCompleteDemo extends TestCase {
	protected static int warmupRequests = DemoConfiguration.warmupRequests;
	protected static int normalRequests = DemoConfiguration.normalRequests;
	public static int totalRequests = (warmupRequests + normalRequests);
	
	protected static CacheImpl completeCacheImpl;
	protected static CountLettersServer completeServerImpl;
	protected static SleepingThinkingBehaviour completeThinkingBehavriourImpl;
	protected static SqlDriverInfo completeSqlDriverInfo;
	
	public MySqlCompleteDemo(String name) {
		super(name);
	}
	
	public static void testTemplate(ServerInterface server, ThinkingBehaviour thinkingBehaviour) {
		SynchronizedTimers requestTimes = new SynchronizedTimers();
		ThinkingClient client = new RandomInfoThinkingMultiThreadedClient(server, thinkingBehaviour, requestTimes, new Random());
		server.start();
		try {
			client.issueRequests(warmupRequests);
			client.getTimes().reset();
			client.issueRequests(normalRequests);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		server.stop();
		System.out.println(requestTimes.getAverageResponseTime());
	}
	
	protected void setUp() throws Exception {
		DemoConfiguration.setLoggers();

		completeCacheImpl = new ArrayCache(DemoConfiguration.defaultCacheSize);
		completeCacheImpl.prefill();
System.out.println("real cache size : " + DemoConfiguration.defaultCacheSize);
		completeSqlDriverInfo = new SqlDriverInfo("com.mysql.jdbc.Driver", "jdbc:mysql://irmabunt.doc.ic.ac.uk:3306/tpcwdb?user=nb605&password=admin&useReadAheadInput=false", true);
		ServerBehaviour serverBehaviour = null;
		if (System.getProperty("accelerated") != null) {
			serverBehaviour = new MySqlDBCachedAccelerated(completeCacheImpl, completeSqlDriverInfo);
		} else {
			serverBehaviour = new MySqlDBCached(completeCacheImpl, completeSqlDriverInfo);
		}
		completeServerImpl = new CountLettersServer(serverBehaviour, totalRequests);
		completeThinkingBehavriourImpl = new SleepingThinkingBehaviour(new Exp(0.4));
		
		// fill in cache during setUp period
		int originalwarmup = warmupRequests; 
		warmupRequests = 3;
		int originalnormal = normalRequests;
		normalRequests = 36; 
		completeServerImpl.resetTo(warmupRequests+normalRequests);
		testTemplate(completeServerImpl, completeThinkingBehavriourImpl);
		warmupRequests = originalwarmup;
		normalRequests = originalnormal;
		completeServerImpl.resetTo(warmupRequests+normalRequests);
	}
	
	public static void testCompleteWithMySql() {	// 5ms avg sleeping time + cache implemented
		testTemplate(completeServerImpl, completeThinkingBehavriourImpl);
		completeServerImpl.cleanup();
	}	
	
	protected void tearDown() {
		DemoConfiguration.printLoggers();
	}
	
	
	public static void main(String[] args) {
		completeCacheImpl = new ArrayCache(39);
		completeCacheImpl.prefill();

		completeSqlDriverInfo = new SqlDriverInfo("com.mysql.jdbc.Driver", "jdbc:mysql://irmabunt.doc.ic.ac.uk:3306/tpcwdb?user=nb605&password=admin&useReadAheadInput=false", true);
		ServerBehaviour serverBehaviour = null;
		if (System.getProperty("accelerated") != null) {
			serverBehaviour = new MySqlDBCachedAccelerated(completeCacheImpl, completeSqlDriverInfo);
		} else {
			serverBehaviour = new MySqlDBCached(completeCacheImpl, completeSqlDriverInfo);
		}
		completeServerImpl = new CountLettersServer(serverBehaviour, totalRequests);
		completeThinkingBehavriourImpl = new SleepingThinkingBehaviour(new Exp(DemoConfiguration.lambda));
		
		// fill in cache during setUp period
		int originalwarmup = warmupRequests; 
		warmupRequests = 3;
		int originalnormal = normalRequests;
		normalRequests = 36; 
		completeServerImpl.resetTo(warmupRequests+normalRequests);
		testTemplate(completeServerImpl, completeThinkingBehavriourImpl);
		warmupRequests = originalwarmup;
		normalRequests = originalnormal;
		completeServerImpl.resetTo(warmupRequests+normalRequests);
		testCompleteWithMySql();
	}
	
}
