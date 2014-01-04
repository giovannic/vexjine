package vtf_tests.demo;

import vtf_tests.demo.servers.MySqlDBCached;
import vtf_tests.demo.servers.MySqlDBCachedWithInsert;
import vtf_tests.serverClientTest.thinking.SleepingThinkingBehaviour;
import vtf_tests.serverClientTest.tools.Exp;

public class MySqlLocalCompleteDemoWithInsert extends MySqlCompleteDemoWithInsert {
	public MySqlLocalCompleteDemoWithInsert(String name) {
		super(name);
	}

	protected void setUp() throws Exception {
		DemoConfiguration.setLoggers();

		completeCacheImpl = new ArrayCache(39);
		completeSqlDriverInfo = new SqlDriverInfo("com.mysql.jdbc.Driver", "jdbc:mysql://localhost:3306/tpcwdb?user=nb605&password=admin&useReadAheadInput=false", true);
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
	
	public static void main(String[] args) {	// 5ms avg sleeping time + cache implemented
		DemoConfiguration.setLoggers();

		completeCacheImpl = new ArrayCache(39);
		completeSqlDriverInfo = new SqlDriverInfo("com.mysql.jdbc.Driver", "jdbc:mysql://localhost:3306/tpcwdb?user=nb605&password=admin&useReadAheadInput=false", true);
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
		
		ModelDemo.startDbInsertingClients(new MySqlDBCachedWithInsert(completeCacheImpl, completeSqlDriverInfo), new SleepingThinkingBehaviour(new Exp(0.1))); //0.1428571	
		testTemplate(completeServerImpl, completeThinkingBehavriourImpl);
		
		
		ModelDemo.insertingIntoDBThread.interrupt();
		barrierSelectAndInsertStreams();
		
	}	
}
