package vtf_tests.demo;

import vtf_tests.demo.servers.MySqlDBCached;
import vtf_tests.serverClientTest.thinking.SleepingThinkingBehaviour;
import vtf_tests.serverClientTest.tools.Exp;

public class MySqlLocalCompleteDemo extends MySqlCompleteDemo {
	public MySqlLocalCompleteDemo(String name) {
		super(name);
	}

	protected void setUp() throws Exception {
		DemoConfiguration.setLoggers();

		completeCacheImpl = new ArrayCache(39);
		completeCacheImpl.prefill();

		
		completeSqlDriverInfo = new SqlDriverInfo("com.mysql.jdbc.Driver", "jdbc:mysql://localhost:3306/tpcwdb?user=nb605&password=admin&useReadAheadInput=false", true);
		completeServerImpl = new CountLettersServer(new MySqlDBCached(completeCacheImpl, completeSqlDriverInfo), totalRequests);
		completeThinkingBehavriourImpl = new SleepingThinkingBehaviour(new Exp(0.2));
		
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
}
