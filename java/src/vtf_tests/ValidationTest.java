/*
 * This class is used to determine the features of VTF that are enabled and working correctly
 */

package vtf_tests;

import virtualtime.EventNotifier;
public class ValidationTest {

	private static boolean validateMethod(int methodId) {
		EventNotifier.registerMethod("testmethod", methodId, 0);
		
		long startTime1 = EventNotifier.getVtfTime();
		long startTime2 = EventNotifier.getThreadVirtualTime();
		long startTime = System.nanoTime();
		
		EventNotifier._afterMethodEntry(methodId);
		double temp = 0.0;
		for (int i = 0; i<10000000; i++) {
			temp = temp + (temp+1.0)/(double)(i+1.0);
		}
		EventNotifier._beforeMethodExit(methodId);
		
		long d = System.nanoTime() - startTime;
		long d2 = EventNotifier.getThreadVirtualTime() - startTime2;
		long d1 = EventNotifier.getVtfTime() - startTime1;
		
		//System.out.println(startTime+ " " + d + " " + d2 + " " + d1);
		return TestUtils.checkRatio(d, d2, 0.1) && TestUtils.checkRatio(d, d1, 0.1); 
	}



	private static boolean validateIoMethod(int methodId, int ioMethodId, int ioDuration) {
		
		long startTime = EventNotifier.getVtfTime();
		
		EventNotifier._afterMethodEntry(methodId);
		Thread.currentThread().setVtfIoInvocationPoint(10);
		EventNotifier.afterIoMethodEntry(Thread.currentThread(), ioMethodId);
		
		EventNotifier._simulateIoDuration(ioDuration);
		
		EventNotifier.beforeIoMethodExit(Thread.currentThread(), ioMethodId);
		EventNotifier._beforeMethodExit(methodId);
		
		long d = EventNotifier.getVtfTime() - startTime;

		return TestUtils.checkRatio(1000000.0 * ioDuration, d, 0.05); 
	}

	
	private static boolean validateTimedWaitingMethod(int methodId, long twDuration) {
		
		long startTime = EventNotifier.getVtfTime();
		EventNotifier._afterMethodEntry(methodId);
		try {
			synchronized(ValidationTest.class) {
				EventNotifier.waitInVirtualTime(ValidationTest.class, twDuration);
			}
		} catch(Exception e) {
			
		}
		EventNotifier._beforeMethodExit(methodId);
		long d = EventNotifier.getVtfTime() - startTime;
		
		return TestUtils.checkRatio(1000000.0 * twDuration, d, 0.05);  
	}	
	
	
	private static void check(boolean result) {
		if (result) {
			System.out.println("PASSED");
		} else {
			System.out.println("FAILED");
		}
	}
	
	public static void main(String[] args) {
		EventNotifier.registerMethod("main", 100, 100);

		EventNotifier._afterMethodEntry(100);
		
		EventNotifier.registerMethod("testIOmethod", 101, 100);
		
		System.out.print("Method logging...");check(validateMethod(101));
		System.out.print("I/O Method logging...");check(validateIoMethod(101, 13, 50));
		System.out.print("Timed-waiting logging...");check(validateTimedWaitingMethod(101, 10));
		
		EventNotifier._beforeMethodExit(100);
		
	}
}
