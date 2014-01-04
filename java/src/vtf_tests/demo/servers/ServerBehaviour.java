package vtf_tests.demo.servers;

import vtf_tests.serverClientTest.Request;

public interface ServerBehaviour {

	public void service(Request r);
	public void cleanup();
}
