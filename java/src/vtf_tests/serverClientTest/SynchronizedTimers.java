package vtf_tests.serverClientTest;

public class SynchronizedTimers extends Timers {
	public SynchronizedTimers() {
		super();
	}
		
	synchronized public void addRequestTimes(long requestTime) {
		totalRequestTimes += requestTime;
		++requests;
	}
		
	synchronized public void mergeWithTimer(Timers t) {
		totalRequestTimes += t.totalRequestTimes;
		requests += t.requests;
	}
}
