package vtf_tests.demo.tests;

import vtf_tests.demo.ModelDemo;

public class TestDemoWithInsert {
	protected static double testedTimePerRequest = 10;
	protected static long maxElapsedTime = (long)(ModelDemo.totalRequests * testedTimePerRequest) + 600 ;	// 600 ms overhead from JUnitPerf
}
