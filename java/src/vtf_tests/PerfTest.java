package vtf_tests;

import virtualtime.EventNotifier;

public class PerfTest {
	public static void profileMethods() {
		int iterations = 1000000;
		EventNotifier.registerMethod("main", 100, 100);
		EventNotifier._afterMethodEntry(100);
		for (int i=0; i<12000; i++) {
			EventNotifier._afterMethodEntry(101);
			EventNotifier._beforeMethodExit(101);	
		}
//		System.out.println("-------------------------");
		EventNotifier.registerMethod("javaPerformanceTest", 101, 100);
		long start = System.nanoTime();
		for (int i=0; i<iterations; i++) {
			EventNotifier._afterMethodEntry(101);
			EventNotifier._beforeMethodExit(101);	
		}
		long end = System.nanoTime();
		System.out.println((end-start)/(double)iterations);
		
		EventNotifier._beforeMethodExit(100);
	}

	public static void profileIo() {
		int iterations = 1000000;
		EventNotifier.registerMethod("main", 100, 100);
		EventNotifier._afterMethodEntry(100);

		for (int i=0; i<12000; i++) {
			Thread thread = Thread.currentThread();
			thread.setVtfIoInvocationPoint(534);
			EventNotifier.afterIoMethodEntry(thread, 4);
			EventNotifier.beforeIoMethodExit(thread, 4);	
		}
		
//		System.out.println("-------------------------");
		long start = System.nanoTime();
		for (int i=0; i<iterations; i++) {
			Thread thread = Thread.currentThread();
			thread.setVtfIoInvocationPoint(534);
			EventNotifier.afterIoMethodEntry(thread, 4);
			EventNotifier.beforeIoMethodExit(thread, 4);	
		}
		long end = System.nanoTime();
		System.out.println((end-start)/(double)iterations);
		
		EventNotifier._beforeMethodExit(100);
	}
	
	public static void main(String[] args) {

		if (args[0].equals("methods")) {
			PerfTest.profileMethods();
		} else if (args[0].equals("io")) {
			PerfTest.profileIo();
		} else {
			System.out.println("Wrong parameter");
		}
	}
}
