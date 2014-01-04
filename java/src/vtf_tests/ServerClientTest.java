package vtf_tests;


import junit.framework.TestCase;
import vtf_tests.TestUtils;
import vtf_tests.serverClientTest.*;
import vtf_tests.serverClientTest.clients.ThinkingClient;
import vtf_tests.serverClientTest.clients.ThinkingMultiThreadedClient;
import vtf_tests.serverClientTest.mm1servers.*;
import vtf_tests.serverClientTest.thinking.ModelThinkingBehaviour;
import vtf_tests.serverClientTest.thinking.SleepingThinkingBehaviour;
import vtf_tests.serverClientTest.tools.Exp;

public class ServerClientTest extends TestCase {
	static private boolean verbose = true;
	static private int defaultTimes = 10;
	static private int warmupRequests = 10;
	static private int actualRequests = 400;
	static private int totalRequests = warmupRequests+actualRequests;
	
    public ServerClientTest() {
    	
    }
    public ServerClientTest(String name) {
        super(name);
    }

	public void testMM1() {
		if (verbose) { System.out.println("Testing sleeping client with looping server with variable service time"); }
		SynchronizedTimers requestTimes = new SynchronizedTimers();
		SimpleSingleCpuStressing server = new SimpleSingleCpuStressing(new Exp(1.0), totalRequests);
		ClientInterface client = new ThinkingMultiThreadedClient(server, new SleepingThinkingBehaviour(new Exp(1.0/400.0)), requestTimes);	
		testGeneratedVariableServerRate(server, client, defaultTimes);
	}
	
	public void testFastMM1() {
		if (verbose) { System.out.println("Testing sleeping client with accelerated looping server with variable service time"); }
		SynchronizedTimers requestTimes = new SynchronizedTimers();
		SimpleSingleCpuStressing server = new FastSimpleSingleCpuStressing(new Exp(1.0), totalRequests);
		ClientInterface client = new ThinkingMultiThreadedClient(server, new SleepingThinkingBehaviour(new Exp(1.0/400.0)), requestTimes);	
		testGeneratedVariableServerRate(server, client, defaultTimes);
	}
	
	public void testSlowMM1() {
		if (verbose) { System.out.println("Testing sleeping client with decelerated looping server with variable service time"); }
		SynchronizedTimers requestTimes = new SynchronizedTimers();
		SimpleSingleCpuStressing server = new SlowSimpleSingleCpuStressing(new Exp(1.0), totalRequests);
		ClientInterface client = new ThinkingMultiThreadedClient(server, new SleepingThinkingBehaviour(new Exp(1.0/400.0)), requestTimes);
		testGeneratedVariableServerRate(server, client, defaultTimes);
	}
	/*
	public void testModelledClientMM1() {
		if (verbose) { System.out.println("Testing model-waiting client with looping server with variable service time"); }
		SynchronizedTimers requestTimes = new SynchronizedTimers();
		SimpleSingleCpuStressing server = new SimpleSingleCpuStressing(new Exp(1.0), totalRequests);
		ClientInterface client = new ThinkingMultiThreadedClient(server, new ModelThinkingBehaviour(), requestTimes);
		testGeneratedVariableServerRate(server, client, defaultTimes);
	}
    
	public void testModelledClientFastMM1() {
		if (verbose) { System.out.println("Testing model-waiting client with accelerated looping server with variable service time"); }
		SynchronizedTimers requestTimes = new SynchronizedTimers();
		SimpleSingleCpuStressing server = new FastSimpleSingleCpuStressing(new Exp(1.0), totalRequests);
		ClientInterface client = new ThinkingMultiThreadedClient(server, new ModelThinkingBehaviour(), requestTimes);	
		testGeneratedVariableServerRate(server, client, defaultTimes);
	}
	
	public void testModelledClientSlowMM1() {
		if (verbose) { System.out.println("Testing model-waiting client with decelerated looping server with variable service time"); }
		SynchronizedTimers requestTimes = new SynchronizedTimers();
		SimpleSingleCpuStressing server = new SlowSimpleSingleCpuStressing(new Exp(1.0), totalRequests);
		ClientInterface client = new ThinkingMultiThreadedClient(server, new ModelThinkingBehaviour(), requestTimes);
		testGeneratedVariableServerRate(server, client, defaultTimes);
	}

	
	public static int testRequests = 2;
	public void testModelledServerMM1() {
		if (verbose) { System.out.println("Testing sleeping client with model-described server with constant service time"); }
		SynchronizedTimers requestTimes = new SynchronizedTimers();
		SimpleSingleCpuStressing server = new ModelDescribedSingleLocalResource(totalRequests);
		ThinkingClient client = new ThinkingMultiThreadedClient(server, new SleepingThinkingBehaviour(new Exp(1.0/400.0)), requestTimes);
		testGeneratedVariableArrivalRate(server, client, defaultTimes);
	}
	
	
	public void testModelsBothMM1() {
		if (verbose) { System.out.println("Testing model-waiting client with model-described server with constant service time"); }
		SynchronizedTimers requestTimes = new SynchronizedTimers();
		SimpleSingleCpuStressing server = new ModelDescribedSingleLocalResource(totalRequests);
		ThinkingClient client = new ThinkingMultiThreadedClient(server, new ModelThinkingBehaviour(), requestTimes);
		double expected = 1.0 / (5.0 - 2.5);
		double actual   = runSimpleMM1ServerTest(server, client)/(double)1e9;
		System.out.print("mi=5.0 lambda=2.5: " + expected + " " + actual);
//		assertTrue(TestUtils.checkRatio(expected, actual, 0.2));
		System.out.println();		
				//);
	}
	*/
	public void testGeneratedVariableServerRate(SimpleSingleCpuStressing server, ClientInterface client, int times) {
		long averageServiceTime = server.getAverageServiceTime();
		int[] values = server.generateInputValues(2.5, times);
		
		for (int i = 0; i < times; i++) {
			double expected = server.getTheoreticalValue(1.0/((averageServiceTime/ 1000000000.0)*values[i]), 2.5)/(double)1e9;
			double actual   = testMM1ForAverageIterations(server, client, values[i])/(double)1e9;
			System.out.print(values[i] + " " + expected + " " + actual);
//			assertTrue(TestUtils.checkRatio(expected, actual, 0.2));
			System.out.println();
		}
	}
	
	public long testMM1ForAverageIterations(SimpleSingleCpuStressing server, ClientInterface client, int averageIterations) {
		server.setDistributionSampler(new Exp(1.0/(double)averageIterations));
		return runSimpleMM1ServerTest(server, client);
	}

	public long testMM1ForAverageThinkingTime(ServerInterface server, ThinkingClient client, ThinkingBehaviour thinkingBehaviour) {
		client.setThinkingBehaviour(thinkingBehaviour);
		return runSimpleMM1ServerTest(server, client);
	}
	
	public long runSimpleMM1ServerTest(ServerInterface server, ClientInterface client) {
		server.start();
		//client.issueRequests(testRequests);	// warmup
		try {
			client.issueRequests(warmupRequests);
			client.getTimes().reset();
			client.issueRequests(actualRequests);
		} catch (InterruptedException ie) {
			ie.printStackTrace();
		}
		server.stop();
		return client.getTimes().getAverageResponseTime(); 
	}
	
	
	public void getServerTimes(MM1Server server, int times) {

		long averageServiceTime = server.getAverageServiceTime();
		int[] values = server.generateInputValues(averageServiceTime, 2.5, times);
		
		for (int i = 0; i < times; i++) {
			System.out.println(values[i] + " " + server.getTheoreticalValue(1.0/((averageServiceTime/ 1000000000.0)*values[i]), 2.5)/(double)1e9);
		}
	}
	
	
	
	public void testGeneratedVariableArrivalRate(SimpleSingleCpuStressing server, ThinkingClient client, int times) {
		long averageServiceTime = server.getAverageServiceTime();
//		times = 10;
//		double[] arrivalRates = new double[1];
		double[] arrivalRates = server.generateArrivalRates(times, 0.1, 5.0);
//		times = 1;
//		arrivalRates[0] = 3.36;
//
		for (int i = 0; i < times; i++) {
			double expected = server.getTheoreticalValue(1.0/(averageServiceTime/ 1000000000.0), arrivalRates[i])/(double)1e9;			
			double actual   = testMM1ForAverageThinkingTime(server, client, new SleepingThinkingBehaviour(new Exp(arrivalRates[i]/1000.0)))/(double)1e9;
			
			System.out.print(1.0/(averageServiceTime/ 1000000000.0) + " " + arrivalRates[i] + ": " + expected + " " + actual);
//			assertTrue(TestUtils.checkRatio(expected, actual, 0.2));
			System.out.println();		
					
		}
	}
	
	public static void main(String[] args) {
		ServerClientTest test = new ServerClientTest();
//		ServerClientTest.testRequests = Integer.parseInt(args[0]);
//		test.getServerTimes(new SimpleSingleCpuStressing(new Exp(1.0)), times);
//		test.getServerTimes(new FastSimpleSingleCpuStressing(new Exp(1.0)), times);
//		test.getServerTimes(new SlowSimpleSingleCpuStressing(new Exp(1.0)), times);

		test.testMM1();

		test.testFastMM1();

		test.testSlowMM1();
/*
		test.testModelledClientMM1();

		test.testModelledClientFastMM1();

		test.testModelledClientSlowMM1();

		test.testModelledServerMM1();
		
		test.testModelsBothMM1();
*/
		
	}	
}
