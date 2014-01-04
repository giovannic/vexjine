package vtf_tests.serverClientTest.thinking;

import vtf_tests.serverClientTest.tools.DistributionSampler;

public class WaitingThinkingBehaviour extends SleepingThinkingBehaviour {

	public WaitingThinkingBehaviour(DistributionSampler thinkTime) {
		super(thinkTime);
	}
	
	public void think() throws InterruptedException {
		double nextThinkTime = sampleThinkingTime();
		long thinkingTimeMilliSeconds = (long)nextThinkTime;
		int thinkingTimeNanoSeconds = (int)((nextThinkTime - thinkingTimeMilliSeconds) * 1000000);
		
		synchronized (this) {
			wait(thinkingTimeMilliSeconds, thinkingTimeNanoSeconds);	
		}
	}

}
