package vtf_tests.modelLoops.tests;

import java.text.DecimalFormat;
import java.util.Vector;

import vtf_tests.TestUtils;
import vtf_tests.modelLoops.BasicFuncCombinations;
import junit.framework.TestCase;

public class TestBasicFunc extends TestCase {
	private static double timePerThread = 1.131252;
	private static int threads = 12;

	static {
		try {
			timePerThread = Double.parseDouble(System.getProperty("time_per_thread"));
		} catch (Exception e) {
			timePerThread = 1.131252;
		}
	}

	static {
		try {
			threads = Integer.parseInt(System.getProperty("threads"));
		} catch (Exception e) {
			threads = 12;
		}
	}
	private static int cores = 1;
	static {
		try {
			cores = Integer.parseInt(System.getProperty("cores"));
		} catch (Exception e) {
			cores = 2;
		}
	}

	private static double expectedTime = (threads * timePerThread) / cores;
	private static double acceptedError = 0.25;

	private static String getTestName() {
		StackTraceElement[] s = Thread.currentThread().getStackTrace();
		return s[3].getMethodName();
	}

	private static void testBasic(double actualTime) {
		DecimalFormat df = new DecimalFormat("#.######");
		System.out.println(getTestName() + ": " + threads + " " + cores + " " + df.format(actualTime) + " " + df.format(expectedTime));
		assertTrue(TestUtils.checkRatio(actualTime, expectedTime, acceptedError));
	}

	private static void testBasic(double actualTime, int waitingThreads) {
		double waitingExpectedTime = expectedTime - (waitingThreads * timePerThread)/cores;
		if (waitingThreads == threads) {
			waitingExpectedTime = timePerThread;
		}
		DecimalFormat df = new DecimalFormat("#.######");
		System.out.println(getTestName() + ": " + cores + " " + df.format(actualTime) + " " + df.format(waitingExpectedTime));
		assertTrue(TestUtils.checkRatio(actualTime, waitingExpectedTime, acceptedError));
	}

	// ALL THE SAME
	public static void testAllReal() {
		testBasic((double)new BasicFuncCombinations().testAllReal(threads)/1e9);
	}
	
	public static void testAllAccelerated() {
		testBasic((double)new BasicFuncCombinations().testAllAccelerated(threads)/1e9);
	}

	public static void testAllWaiting() {
		testBasic((double)new BasicFuncCombinations().testThreads(new Vector<Thread>(), 0, 0, 0, threads)/1e9, threads);
	}

	public static void testAllModel() {
		testBasic((double)new BasicFuncCombinations().testAllModel(threads)/1e9);
	}

	// PAIRS 
	public static void testRealAccelerated() {
		int half = threads/2;
		testBasic((double)new BasicFuncCombinations().testThreads(new Vector<Thread>(), half, 0, half, 0)/1e9);
	}

	public static void testRealWaiting() {
		int half = threads/2;
		testBasic((double)new BasicFuncCombinations().testThreads(new Vector<Thread>(), half, 0, 0, half)/1e9, half);
	}

	public static void testRealModel() {
		int half = threads/2;
		testBasic((double)new BasicFuncCombinations().testThreads(new Vector<Thread>(), half, half, 0, 0)/1e9);
	}

	public static void testAcceleratedWaiting() {
		int half = threads/2;
		testBasic((double)new BasicFuncCombinations().testThreads(new Vector<Thread>(), 0, 0, half, half)/1e9, threads/2);
	}


	public static void testAcceleratedModel() {
		int half = threads/2;
		testBasic((double)new BasicFuncCombinations().testThreads(new Vector<Thread>(), 0, half, half, 0)/1e9);		
	}

	public static void testWaitingModel() {
		int half = threads/2;
		testBasic((double)new BasicFuncCombinations().testThreads(new Vector<Thread>(), 0, half, 0, half)/1e9, threads/2);			
	}

	// TRIPLE 
	public static void testRealAcceleratedWaiting() {
		int third = threads/3;
		testBasic((double)new BasicFuncCombinations().testThreads(new Vector<Thread>(), third, 0, third, third)/1e9, third);
	}

	public static void testRealAcceleratedModel() {
		int third = threads/3;
		testBasic((double)new BasicFuncCombinations().testThreads(new Vector<Thread>(), third, third, third, 0)/1e9);
	}

	public static void testRealWaitingModel() {
		int third = threads/3;
		testBasic((double)new BasicFuncCombinations().testThreads(new Vector<Thread>(), third, third, 0, third)/1e9, third);
	}

	public static void testAcceleratedWaitingModel() {
		int third = threads/3;
		testBasic((double)new BasicFuncCombinations().testThreads(new Vector<Thread>(), 0, third, third, third)/1e9, third);
	}

	// ALL	
	public static void testAll() {
		int quarter = threads/4;
		testBasic((double)new BasicFuncCombinations().testThreads(new Vector<Thread>(), quarter, quarter, quarter, quarter)/1e9, quarter);			
	}
	
	public static void main(String[] args) {
		System.out.println((double)new BasicFuncCombinations().testAllReal(1)/1e9);		
	}
}
