package vtf_tests.serverClientTest.thinking;

import vtf_tests.demo.DemoConfiguration;
import vtf_tests.demo.loggers.Loggers;
import vtf_tests.serverClientTest.ThinkingBehaviour;
import vtf_tests.serverClientTest.tools.DistributionSampler;

public class SleepingThinkingBehaviour implements ThinkingBehaviour {
	
	public SleepingThinkingBehaviour(DistributionSampler thinkTime) {
		this.thinkTime = thinkTime;
	}
	
    protected double sampleThinkingTime() {
    	return thinkTime.next();
    }
	
	public void think() throws InterruptedException {
		double nextThinkTime = sampleThinkingTime();
		long thinkingTimeMilliSeconds = (long)nextThinkTime;
		int thinkingTimeNanoSeconds = (int)((nextThinkTime - thinkingTimeMilliSeconds) * 1000000);
		
//System.out.println(Thread.currentThread().getName() + ": " + nextThinkTime);
//			System.out.println(nextThinkTime + " " + thinkingTimeMilliSeconds + " " + thinkingTimeNanoSeconds);
		
		long startTime = Loggers.selectThinkingTimeLogger.onEventStart();
		Thread.sleep(thinkingTimeMilliSeconds, thinkingTimeNanoSeconds);

/*
		if ((System.nanoTime() - startTime) < (thinkingTimeMilliSeconds * 1e6 + thinkingTimeNanoSeconds)) {
			System.out.println("lathos lathos lathos: " + (System.nanoTime() - startTime) + " < " + (thinkingTimeMilliSeconds * 1e6 + thinkingTimeNanoSeconds));
		} else {
			System.out.println("swsta swsta swsta: " + (System.nanoTime() - startTime) + " >= " + (thinkingTimeMilliSeconds * 1e6 + thinkingTimeNanoSeconds));
		}
*/

		Loggers.selectThinkingTimeLogger.onEventEnd(startTime);
	}

	protected DistributionSampler thinkTime;
}
