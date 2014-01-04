package vtf_tests;

import queueing.Exp;

public class MulticoreTest implements Runnable {

	final static int VirtualLeapForwardTest = 1;
	final static int TimeScalingTest = 2;
	final static int ModelLocalNoBodyVirtualTest = 3;
	final static int ModelLocalWithShortBodyVirtualTest = 4;
	final static int ModelLocalWithLongBodyVirtualTest = 5;
	final static int LotsOfMethodsTest = 6;	// ALWAYS USE WITH -Xint OTHERWISE THE CODE IS INLINED
	final static int BackgroundLoadTest = 7;

	static long start;	
	
	private int id;
	public MulticoreTest(int _id) {
		id = _id;
	}
	
	private static boolean sharedFlag;

	public static double myLoops(long loopCount) {
		double temp = 0.0;
		double i;
		for(i = 0; i<loopCount; i++) {
			temp += (i * (8.0+i)) / (i+1.0);
		}
		temp += 1;
		return temp;
	}

	public static double myInternalLoops(long loopCount) {
		double temp = 0.0;
		long iterations = 10000000;
		long loopPerIteration = loopCount / iterations;
		for (long i = 0; i < iterations; i++) {
			temp += myLoops(loopPerIteration);
		}
		return temp;
	}
	
	@virtualtime.Accelerate(speedup = 0.25)	
	public static double myFastLoops() {
		return myLoops(200000000);
	}
	
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/waiting400_local.jsimg",
    		replaceMethodBody=true)  		
    public static void simulateLocalVirtualResourceWithNoBody() {

    }

    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/waiting400_local.jsimg",
    		replaceMethodBody=false)
    public static void simulateLocalVirtualResourceWithShortBody() {
    	myLoops(50000000);
    }

    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/waiting400_local.jsimg",
    		replaceMethodBody=false)
    public static void simulateLocalVirtualResourceWithLongBody() {
    	myLoops(200000000);
    }
    
	public void run() {
		try {
			switch (id) {
				case VirtualLeapForwardTest:
					Thread.sleep(3000);
					MulticoreTest.setSharedFlag();
					break;
				case TimeScalingTest:
					myFastLoops();
					break;
					
				case ModelLocalNoBodyVirtualTest:
					for (int i = 0; i<10; i++) {
						simulateLocalVirtualResourceWithNoBody();
					}
					MulticoreTest.setSharedFlag();
					break;
				case ModelLocalWithShortBodyVirtualTest:
					for (int i = 0; i<10; i++) {
						simulateLocalVirtualResourceWithNoBody();
					}
					MulticoreTest.setSharedFlag();
					break;
				case ModelLocalWithLongBodyVirtualTest:
					for (int i = 0; i<10; i++) {
						simulateLocalVirtualResourceWithNoBody();
					}
					MulticoreTest.setSharedFlag();
					break;
				case LotsOfMethodsTest:
					// ALWAYS USE WITH -Xint OTHERWISE THE CODE IS INLINED
					double temp = myInternalLoops(50000000);
					
					break;
				case BackgroundLoadTest:
					temp = myLoops(300000000);
					
				default:
			}
			System.out.println(Thread.currentThread().getName() + " finished  at " + (System.nanoTime() - start)/1e9);
		} catch (InterruptedException x) {
			System.out.println("otherThread interrupted");
		}
			
	}
	
	
	private synchronized void doWait() throws InterruptedException {
		wait(1000);	
	}

	private static synchronized void checkAndWait() throws InterruptedException { 
		while (!MulticoreTest.sharedFlag) {
			MulticoreTest.class.wait(1000);
			System.out.println("second passed");
		}
	}
	
	
	
	private synchronized void checkWaitInst() throws InterruptedException {
		while (!MulticoreTest.sharedFlag) {
			doWait();
			System.out.println("second passed");
		}
	}
	
	public static void programLegend() {
		System.out.println("Syntax error: java vtf_tests.MulticoreTest <0-7>");
		System.exit(-1);	
	}
	
	public static synchronized void testWhetherSharedFlagOffAndSet(double temp) {
		if (!sharedFlag) {
			sharedFlag = true;
			System.out.println("Correct " + temp); 
		} else {
			System.out.println("Wrong");
		}
	}
	
	public static synchronized void setSharedFlag() {
		sharedFlag = true;
	}
	
	public static void main(String[] args) {
		if (args.length != 1) {
			programLegend();
		}
		
		int testId = Integer.parseInt(args[0]);
		
		
		MulticoreTest.sharedFlag = false;
		MulticoreTest si = new MulticoreTest(testId);
		
		Thread t = new Thread(si, "otherThread");
		t.start();

		start= System.nanoTime();
		try {
			switch (testId) {
				case TimeScalingTest:
				case LotsOfMethodsTest:
				case BackgroundLoadTest:
					double temp = myLoops(400000000);
					break;
									
				default:
					temp = myLoops(100000000);
					MulticoreTest.testWhetherSharedFlagOffAndSet(temp);
			}

			System.out.println("main finished at " + (System.nanoTime() - start)/1e9);
			t.join();
		} catch (InterruptedException x) {
			System.out.println("main interrupted!");
		}
		
		
		System.out.println((System.nanoTime() - start)/1e9);
	}
	
	
	
}
