package vtf_tests.demo.tests;

import vtf_tests.demo.ModelDemo;
import junit.framework.Test;

import com.clarkware.junitperf.TimedTest;

public class TestRealClientToModelWithInsert extends TestDemoWithInsert {
    public static Test suite() {
        return new TimedTest(new ModelDemo("testThinkingClientToModelWithInsert"), maxElapsedTime);
    }
	
//	public static void testClientAndCacheImplDBserverModelWithInsertFailing();
//	public static void testClientAndCacheImplDBserverModelWithInsertSucceeding();
//	public static void testClientAndCacheImplDBserverModelWithInsert(int cacheSize);
//	public static void testCompleteWithInsertWithMySql();
	
}