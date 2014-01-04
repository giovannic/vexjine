package vtf_tests.serverClientTest;

public interface ClientInterface {
	
	public void issueRequests(int requests) throws InterruptedException;
	public Timers getTimes();
}
