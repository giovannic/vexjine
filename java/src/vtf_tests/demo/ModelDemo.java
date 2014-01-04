/***
 * VEX model usage demo class, based on the counting letters database paradigm.
 * The counting letters database 
 */
package vtf_tests.demo;

import java.util.Random;

import junit.framework.TestCase;
import vtf_tests.demo.loggers.DurationLogger;
import vtf_tests.demo.loggers.EmptyLogger;
import vtf_tests.demo.loggers.LoggerInterface;
import vtf_tests.demo.loggers.Loggers;
import vtf_tests.demo.servers.ModelDbAllUpdateModelCache;
import vtf_tests.demo.servers.ModelDbAllUpdateRealCache;
import vtf_tests.demo.servers.ModelDbSelectRealCache;
import vtf_tests.demo.servers.ModelDbSelectModelCache;
import vtf_tests.demo.servers.ModelDbAllSelectRealCache;
import vtf_tests.demo.servers.ModelDbAllSelectModelCache;
import vtf_tests.demo.servers.ServerBehaviour;
import vtf_tests.serverClientTest.ServerInterface;
import vtf_tests.serverClientTest.SynchronizedTimers;
import vtf_tests.serverClientTest.ThinkingBehaviour;
import vtf_tests.serverClientTest.clients.RandomInfoThinkingMultiThreadedClient;
import vtf_tests.serverClientTest.clients.ThinkingClient;
import vtf_tests.serverClientTest.thinking.SleepingThinkingBehaviour;
import vtf_tests.serverClientTest.tools.Exp;

public class ModelDemo extends TestCase {
	protected static int warmupRequests = DemoConfiguration.warmupRequests;
	protected static int normalRequests = DemoConfiguration.normalRequests;
	
	public static int totalRequests = (warmupRequests + normalRequests);
	public static Thread insertingIntoDBThread = null;
	
	
	public ModelDemo(String name) {
		super(name);
	}
	
	protected void setUp() throws Exception {
		DemoConfiguration.setLoggers();
	}
	
	public static void testTemplate(ServerBehaviour serverBehaviour, ThinkingBehaviour thinkingBehaviour) {
		long start = System.currentTimeMillis();
		long end = 0;
		SynchronizedTimers requestTimes = new SynchronizedTimers();
		ServerInterface server = new CountLettersServer(serverBehaviour, totalRequests);
		ThinkingClient client = new RandomInfoThinkingMultiThreadedClient(server, thinkingBehaviour, requestTimes, new Random());
		server.start();
		try {
			client.issueRequests(warmupRequests);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		client.getTimes().reset();
		
		if (insertingIntoDBThread != null) {
			MySqlCompleteDemoWithInsert.barrierSelectAndInsertStreams();
		}

		try {
			client.issueRequests(normalRequests);
			server.stop();

		} catch (InterruptedException e) {
			// Expected
//			e.printStackTrace();
		}
		
		end = System.currentTimeMillis();
		System.out.println(end-start);
		requestTimes.getAverageResponseTime();
		
		if (insertingIntoDBThread != null) {
			MySqlCompleteDemoWithInsert.barrierSelectAndInsertStreams();
			insertingIntoDBThread.stop();
		}
	}
	
	/***
	 * Original version: clients request from server
	 */
	public static void testEmpty() {
		testTemplate(new ModelDbSelectModelCache(), new CountingLettersModelClient());
		
	}
	
	public static void testThinkingClientToModel() {	// 5ms avg sleeping time
		testTemplate(new ModelDbSelectModelCache(), new SleepingThinkingBehaviour(new Exp(DemoConfiguration.lambda)));
		
	}
	
	public static void testClientAndCacheImplDBserverModelFailing() {	// 5ms avg sleeping time + cache implemented
		testClientAndCacheImplDBserverModel(13);
		
	}

	public static void testClientAndCacheImplDBserverModelSucceeding() {	// 5ms avg sleeping time + cache implemented
		testClientAndCacheImplDBserverModel(DemoConfiguration.defaultCacheSize);//DemoConfiguration.defaultCacheSize);
		
	}
	
	public static void testClientAndCacheImplDBserverModel(int cacheSize) {	// 5ms avg sleeping time + cache implemented
		CacheImpl cache = new ArrayCache(cacheSize);
		cache.prefill();
//		testTemplate(new ModelDbSelectRealCache(cache), new CountingLettersModelClient()); 
		testTemplate(new ModelDbSelectRealCache(cache), new SleepingThinkingBehaviour(new Exp(DemoConfiguration.lambda)));
		
	}
	

	/***
	 * Extended version: new client class that inserts into DB invalidating the cache
	 * I want to prove that the simulated version loses the performance degradation due to cache invalidation.
	 */
	public static void startDbInsertingClients(ServerBehaviour serverBehaviour, ThinkingBehaviour thinkingBehaviour) {
		insertingIntoDBThread = new Thread(new InsertingIntoDbThread(serverBehaviour, thinkingBehaviour), "insertingIntoDBWaitingThread");
		insertingIntoDBThread.setDaemon(true);
		insertingIntoDBThread.start();
	}
	
	public static void testEmptyWithInsert() {
		startDbInsertingClients(new ModelDbAllUpdateModelCache(), 	new DbInsertingModelClient());
		testTemplate(			new ModelDbAllSelectModelCache(), 	new CountingLettersModelClient());
//		testTemplate(new ModelDbAllUpdateModelCache(), 	new DbInsertingModelClient());
	}

	public static void testThinkingClientToModelWithInsert() {
		startDbInsertingClients(new ModelDbAllUpdateModelCache(), 	new SleepingThinkingBehaviour(new Exp(0.1)));
		testTemplate(			new ModelDbAllSelectModelCache(), 	new SleepingThinkingBehaviour(new Exp(DemoConfiguration.lambda)));
	}
	
	
	public static void testClientAndCacheImplDBserverModelWithInsertFailing() {	// 5ms avg sleeping time + cache implemented
		testClientAndCacheImplDBserverModelWithInsert(13);
	}
	
	
	public static void testClientAndCacheImplDBserverModelWithInsertSucceeding() {	// 5ms avg sleeping time + cache implemented
		testClientAndCacheImplDBserverModelWithInsert(DemoConfiguration.defaultCacheSize);
	}
	
	public static void testClientAndCacheImplDBserverModelWithInsert(int cacheSize) {	// 5ms avg sleeping time + cache implemented
		CacheImpl cache = new ArrayCache(cacheSize);
		cache.prefill();
		
		startDbInsertingClients(new ModelDbAllUpdateRealCache(cache), 	new SleepingThinkingBehaviour(new Exp(0.1)));
		testTemplate(			new ModelDbAllSelectRealCache(cache), 	new SleepingThinkingBehaviour(new Exp(DemoConfiguration.lambda)));
	}
	

	protected void tearDown() {
		DemoConfiguration.printLoggers();
	}
	
//	public static void testCompleteWithInsertWithMySql() {	// 5ms avg sleeping time + cache implemented
//		startDbInsertingClients(new MySqlDBCachedWithInsert(completeCacheImpl, completeSqlDriverInfo), new SleepingThinkingBehaviour(new Exp(0.1)));
//		testTemplate(completeServerImpl, completeThinkingBehavriourImpl);
//		completeServerImpl.cleanup();
//	}	
//	

	public static void main(String[] args) {
		DemoConfiguration.setLoggers();
		testClientAndCacheImplDBserverModel(DemoConfiguration.defaultCacheSize);		
		DemoConfiguration.printLoggers();
	}
}
