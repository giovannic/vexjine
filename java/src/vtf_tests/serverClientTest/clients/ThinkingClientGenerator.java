package vtf_tests.serverClientTest.clients;

import vtf_tests.serverClientTest.ClientInterface;
import vtf_tests.serverClientTest.ServerInterface;
import vtf_tests.serverClientTest.Timers;
import vtf_tests.serverClientTest.thinking.SleepingThinkingBehaviour;
import vtf_tests.serverClientTest.tools.DistributionSampler;

public class ThinkingClientGenerator implements ClientTypeGenerator {
	public ThinkingClientGenerator(DistributionSampler sampler, ServerInterface server) {
		this.sampler = sampler;
		this.server = server;
	}
	
	public ClientInterface generateClient() {
		return new ThinkingClient(server, new SleepingThinkingBehaviour(sampler), new Timers());
	}

	DistributionSampler sampler;
	ServerInterface server;
}
