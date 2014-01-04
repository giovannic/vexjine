package vtf_tests.serverClientTest;

public class Timers {
	public Timers() {
		reset();
	}
	
	public void addRequestTimes(long requestTime) {
		totalRequestTimes += requestTime;
		++requests;
	}
	
	public void mergeWithTimer(Timers t) {
		totalRequestTimes += t.totalRequestTimes;
		requests += t.requests;
	}
	
	public void reset() {
		requests = 0;
		totalRequestTimes = 0;
	}
	
	public long getAverageResponseTime() {
//		System.out.println("requests: " + requests + " accResponseTime: " + (double)totalRequestTimes/1e9 + " mean:" + (double)totalRequestTimes/((double)1e9*requests));
		return totalRequestTimes/requests;
	}
	protected long totalRequestTimes;
	protected int requests;
	
}
