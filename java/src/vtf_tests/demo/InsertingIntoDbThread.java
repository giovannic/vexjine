package vtf_tests.demo;

import vtf_tests.demo.servers.ServerBehaviour;
import vtf_tests.serverClientTest.ThinkingBehaviour;
public class InsertingIntoDbThread implements Runnable {

	protected ServerBehaviour serverBehaviour;
	protected ThinkingBehaviour thinkingBehaviour;
	
	public InsertingIntoDbThread(ServerBehaviour serverBehaviour, ThinkingBehaviour thinkingBehaviour) {
		this.serverBehaviour = serverBehaviour;
		this.thinkingBehaviour = thinkingBehaviour;
	}
	
	@Override
	public void run() {
		ModelDemo.testTemplate(serverBehaviour, thinkingBehaviour);
	}
	

}
