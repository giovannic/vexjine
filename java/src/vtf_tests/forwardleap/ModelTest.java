package vtf_tests.forwardleap;

import vtf_tests.forwardleap.threadtypes.model.*;

public class ModelTest {

	public static int threadIterations;
	public static int cores;
	static {
		try {
			threadIterations = Integer.parseInt(System.getProperty("iterations"));
		} catch (Exception e) {
			threadIterations = 1;
		}
		
	}
	
	static {
		try {
			cores = Integer.parseInt(System.getProperty("cores"));
		} catch (Exception e) {
			cores = 1;
		}
		
	}
	
	public static void genericTest(String[] threadTypes) {
		int threads = 0;
		
		for (int i = 0; i<threadTypes.length; i++) {
			try {
				threads += Integer.parseInt(threadTypes[i]);
			} catch (NumberFormatException ne) {
				threads += 0;
			}
		}
		
		if (threads <= 0) {
			System.out.println("Generic test order: <LLA> <LLI> <LRA> <LRI> <SLA> <SLI> <SRA> <SRI>");
			System.out.println("<L__>: Long\t\t<S__>: Short (duration of method body)");
			System.out.println("<_L_>: Local\t\t<_R_>: Remote (type of server virtual resource)");
			System.out.println("<__A>: Active\t\t<__I>: Inactive (method replaced by stub)");
			System.out.println("Example <LRI> = 1 means that 1 thread is supposed to execute a long body method (L), but its performance is determined by a remote virtual server (R) and the method body is replaced by a stub (I)");
			return;
		}
		
		Thread[] testThreads = new Thread[threads];
		ModelThread[] modelTestThreads = new ModelThread[threads];
		int count = 0;
		for (int i = 0; i<threadTypes.length; i++) {
			try {
				int threadsOfNextCategory = Integer.parseInt(threadTypes[i]);
				for (int k = 0; k<threadsOfNextCategory; k++) {
					switch (i) {
						case 0:
							testThreads[count] = new Thread((modelTestThreads[count] = new ModelLongLocalActiveThread(threadIterations)), "LLA" + k);
							break;
						case 1:
							testThreads[count] = new Thread((modelTestThreads[count] = new ModelLongLocalInactiveThread(threadIterations)), "LLI" + k);
							break;
						case 2:
							testThreads[count] = new Thread((modelTestThreads[count] = new ModelLongRemoteActiveThread(threadIterations)), "LRA" + k);
							break;
						case 3:
							testThreads[count] = new Thread((modelTestThreads[count] = new ModelLongRemoteInactiveThread(threadIterations)), "LRI" + k);
							break;
						case 4:
							testThreads[count] = new Thread((modelTestThreads[count] = new ModelShortLocalActiveThread(threadIterations)), "SLA" + k);
							break;
						case 5:
							testThreads[count] = new Thread((modelTestThreads[count] = new ModelShortLocalInactiveThread(threadIterations)), "SLI" + k);
							break;
						case 6:
							testThreads[count] = new Thread((modelTestThreads[count] = new ModelShortRemoteActiveThread(threadIterations)), "SRA" + k);
							break;
						case 7:
							testThreads[count] = new Thread((modelTestThreads[count] = new ModelShortRemoteInactiveThread(threadIterations)), "SRI" + k);
							break;
						default:
							break;
					}
					++count; 
				}

			} catch (NumberFormatException ne) {
				ne.printStackTrace();
			}
		}


		long startTime = System.nanoTime();
		for (int i = 0; i<threads; i++) {
			testThreads[i].start();
		}

		
		long totalMsOfOccupiedResource = 0;
		long totalMsOfWaitingResource = 0;
		for (int i = 0; i<threads; i++) {
			try {
				testThreads[i].join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			if (modelTestThreads[i].requiresResource()) {
				totalMsOfOccupiedResource += modelTestThreads[i].getApproximateExecutionTime(); 
			} else {
				totalMsOfWaitingResource += modelTestThreads[i].getApproximateExecutionTime();
			}
		}
		
		long expectedModelTime;
		if (totalMsOfWaitingResource > totalMsOfOccupiedResource/cores) {
			expectedModelTime = totalMsOfWaitingResource;
		} else {
			expectedModelTime = totalMsOfOccupiedResource/cores;
		}
		
		long totalTime = System.nanoTime() - startTime;
		System.out.println((double)totalTime/1e9 + " " + expectedModelTime/1e3);
	}
	
	
	public static void main(String[] args) {
		genericTest(args);
	}
}
