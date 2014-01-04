package vtf_tests.serverClientTest;

public interface ServerInterface {

	public void start();
	public void stop();
	
	public void makeRequest(Request r);
	public void service(Request r);
	public long getAverageServiceTime();
}
