package vtf_profiler;
import java.io.PrintStream;
import java.lang.reflect.Type;
import java.util.*;

import virtualtime.EventNotifier;

//import virtualtime.EventNotifier;
public class ExplicitHostProfiler implements Runnable {

	private static Random ra = new Random();
	/*
	 * Basic looping method 
	 */
	
	private int threadLoops = 0;
	public ExplicitHostProfiler(int _threadLoops) {
		threadLoops = _threadLoops;
	}
	public void run() {
		double temp = 0.0;
		temp += ExplicitHostProfiler.yieldLoops(threadLoops);
	}
	
	public static double recursiveInner(long loopCount) {
		if (loopCount == 0) {
			return 1;
		} else {
			return recursiveInner(loopCount-1)+1;
		}
		
		/*
		if (loopCount <= 0) {
			return Math.sin(loopCount); 
		} else if (loopCount == 1) {
			return Math.cos(loopCount);
		} else {
//			int nextRandom = ra.nextInt();
			if (loopCount % 2 == 0) {
				return recursiveInner(loopCount - 3);
			} else {
				return recursiveInner(loopCount + 1);
			}
		}
		*/
	}	
	public static double recursiveWarmup(long loopCount) {
		double temp = 0.0;
    	for (int i =0 ; i< 4; i++) {
    		temp += recursiveInner(loopCount);
    	}
    	return temp;
	}
	public static double recursiveLoops(long loopCount) {
		double temp = 0.0;
		for (int i = 0; i<loopCount; ++i) {
			temp += recursiveInner(2000);
		}
    	return temp;
	}
	
	
	
	
	public static double yieldWarmup(long loopCount) {
		double temp = 0.0;
    	for (int i =0 ; i< loopCount; i++) {
//    		temp += i;
    		Thread.yield();
    	}
    	return temp;
	}
	public static double yieldLoops(long loopCount) {
		double temp = 0.0;
		for (int i = 0; i<loopCount; ++i) {
//			temp += i;
			Thread.yield();
		}
    	return temp;
	}

		
	
	// These should call the same methods
	public static double warmup(long loopCount) {
    	double temp = 0.0;
//    	for (int i =0 ; i< 4; i++) {
//    		temp += inner(1);//loopCount);
//    	}
    	return temp;
	}
	public static double myLoops(long loopCount) {
//		EventNotifier._afterMethodEntry(110);
    	double temp = inner(loopCount);
//    	EventNotifier._beforeMethodExit(110);
    	return temp;
	}
	
	static int indx[];  // Index into random keys  

	public static double inner(long loopCount) {
		
		double temp = 0.0;
		double lt;
    	long start = System.nanoTime();
    	int i, a,b;
  		for(i = 0; i<loopCount; i++) {
  			a = i % 10;
  			b =(i+1) % 10;
//  			EventNotifier._afterMethodEntry(111);
  			swap(a, b);
//  			EventNotifier._beforeMethodExit(111);
  			temp += ExplicitHostProfiler.doit(i);
  			
  		
  			//temp += (i * (8.0+i)) / (i+1.0);
  		}
  		temp += 1;
  		
  		if (loopCount >1000) {
  		long end = System.nanoTime();
		System.out.println((end-start)/1000000000.0);

  		}
  		return temp;
	}
	public static double doit(double i) {
		
		int limit = ra.nextInt(10);
		for (int k = 0;k<limit;k++) {
			i = Math.sqrt(i * (8.0+i)) / Math.pow(i+1.0,2);
//		i = (i * (8.0+i)) / (i+1.0);
		}
		
		return i;//3.1415;
	}	
	
	
	public static void swap(int a, int b) {
	    int itemp;
	    itemp=indx[a];
	    indx[a]=indx[b];
	    indx[b]=itemp;
	}
	
	
	// These should call the same methods
	public static double syncWarmup(long loopCount) {
    	double temp = 0.0;
    	for (int i =0 ; i< 4; i++) {
      		for(i = 0; i<loopCount; i++) {
      			temp += ExplicitHostProfiler.syncdoit(i, true);
      		}
    	}
    	return temp;
	}
	public static double syncLoops(long loopCount) {
		
		double temp = 0.0;
    	double i;
  		for(i = 0; i<loopCount; i++) {
  			temp += ExplicitHostProfiler.syncdoit(i, true);
  			if (temp > 1000) {
  				++i;
  			}
  		}
  		temp += 1;

  		return temp;
  		
	}
	public synchronized static double syncdoit(double i, boolean flag) {
		//int limit = ra.nextInt(10000);
		//for (int k = 0;k<limit;k++) {
		if (flag) {
			i = (i * (8.0+i)) / (i+1.0);
		}
		//}
		return i;//3.1415;
	}	
	


	// Syncinthronization po
	public static double innerSyncWarmup(long loopCount) {
    	double temp = 0.0;
    	for (int i =0 ; i< 4; i++) {
      		for(i = 0; i<loopCount; i++) {
      			synchronized(ExplicitHostProfiler.class) {
    				temp += (i * (8.0+i)) / (i+1.0);
    			}
      		}
    	}
    	return temp;
	}	
	public static double innerSyncLoops(long loopCount) {
		double temp = 0.0;
    	double i;
  		for(i = 0; i<loopCount; i++) {
  			synchronized(ExplicitHostProfiler.class) {
				temp += (i * (8.0+i)) / (i+1.0);
			}
  		}
  		temp += 1;
  		return temp;
	}

	
	public synchronized void testingNewInstr() {
		System.out.println("hello world");
	}
	public synchronized void testingNewInstr(boolean shouldDo, int value, PrintStream output) {
		if (shouldDo) {
			output.println("hello world " + (value++));
		}
	}
	
	public synchronized int testingNewInstr(boolean shouldThrowException) throws ArrayIndexOutOfBoundsException {
		int[] i = new int[10];
		int limit =0 ;
		if (shouldThrowException) {
			limit = 152;
		} else {
			limit = 10;
		}
		
		int sum =0 ;
		for (int k = 0; k<limit; k++) {
			i[k] = k;
			sum += (i[k] + k);
		}
		return sum;
		
	}
	
	private static long getTime() {
//		return System.nanoTime();
		
		return EventNotifier.getThreadVirtualTime();
		
	}
	
	/**
	 * Host profiling test: Run a number of small methods to identify the overhead involved in calling VTF
	 * argv[0]: loops: how many loops will there be performed
	 * argv[1]: sync|nosync: if we will have the method in a synchronized block
	 * @param argv
	 */
	
	private static final int METHOD_DELAY = 1;
	private static final int INTERACTION_POINT_DELAY = 2;
	private static final int SYNCHRONIZED_METHOD_DELAY = 3;
	private static final int IO_POINT_DELAY = 4;
	private static final int RECURSIVE_DELAY = 5;
	private static final int YIELD_DELAY = 6;
	private static final int THREAD_START_DELAY = 7;
	
	/*
	public enum ProfilingType {
		METHOD_DELAY,
		INTERACTION_POINT_DELAY,
		SYNCHRONIZED_METHOD_DELAY,
		IO_POINT_DELAY,
		RECURSIVE_DELAY,
		YIELD_DELAY,
		THREAD_START_DELAY
	}*/
	public static void main(String[] argv) {
		if (argv.length < 1) {
			System.out.println("Error: ExplicitHostProfiler syntax: java vtf_profiler.ExplicitHostProfiler <# loops> <sync|nosync>");
			return;
		}
//		
//		ExplicitHostProfiler hp = new ExplicitHostProfiler();
//		hp.testingNewInstr();
//		hp.testingNewInstr(true, 6, System.err);
//		System.out.println(hp.testingNewInstr(false));
//		
		indx = new int[10];
		for (int i =0 ; i< 10; i++) {
			indx[i] = ra.nextInt();
		}
		
		EventNotifier.registerMethod("vtf_profiler/ExplicitHostProfiler main ([Ljava/lang/String;)V", 121, 121);
		EventNotifier.registerMethod("vtf_profiler/ExplicitHostProfiler warmup (J)D", 108, 121);
		EventNotifier.registerMethod("vtf_profiler/ExplicitHostProfiler myLoops (J)D", 109, 121);
		EventNotifier.registerMethod("vtf_profiler/ExplicitHostProfiler inner (J)D", 110, 121);
		EventNotifier.registerMethod("vtf_profiler/ExplicitHostProfiler <clinit> ()V", 100, 121);
		EventNotifier.registerMethod("vtf_profiler/ExplicitHostProfiler getTime ()J", 120, 121);
		EventNotifier.registerMethod("vtf_profiler/ExplicitHostProfiler doit (D)D", 111, 121);
				
		int threadsNum;
		try {
			threadsNum = Integer.parseInt(argv[2]);
		} catch (ArrayIndexOutOfBoundsException ar) {
			threadsNum = 1;
		}
		Thread[] threads = null;
		
		int profilingVersion = METHOD_DELAY;
		try {
			if (argv[0].equals("sync")) {
				profilingVersion = INTERACTION_POINT_DELAY;
			} else if (argv[0].equals("methodsyn")) {
				profilingVersion = SYNCHRONIZED_METHOD_DELAY;
			} else if (argv[0].equals("iodelay")) {
				profilingVersion = IO_POINT_DELAY;
			} else if (argv[0].equals("recursive")) {
				profilingVersion = RECURSIVE_DELAY;
			} else if (argv[0].equals("yield")) {
				profilingVersion = YIELD_DELAY;
			} else if (argv[0].equals("thread")) {
				profilingVersion = THREAD_START_DELAY;
			}
		} catch (ArrayIndexOutOfBoundsException ne) {
			profilingVersion = METHOD_DELAY;
		}
		

		long start=0,stop=0;
		
		int loops = Integer.parseInt(argv[1]);
//		System.out.println("---------------------------------------------------------");

		double temp = 0.0;
		
//System.out.println(profilingVersion);
		long diff1 =0 ;
		switch (profilingVersion) {
			case INTERACTION_POINT_DELAY:
				temp += innerSyncWarmup(1000000);
				temp = temp +1;	
				start = getTime();
				ExplicitHostProfiler.innerSyncLoops(loops);
				stop = getTime();
				break;
				
			case METHOD_DELAY:
//				temp += warmup(1000000);
				temp = temp +1;
				start = getTime();
//				EventNotifier._afterMethodEntry(109);
				ExplicitHostProfiler.myLoops(loops);
//				EventNotifier._beforeMethodExit(109);
				stop = getTime();
				break;
				
			case SYNCHRONIZED_METHOD_DELAY:
				temp += innerSyncWarmup(100000);
				temp = temp +1;
				
				start = getTime();
				
//				long s1,s2,e1,e2;
//				s1 = EventNotifier.getThreadVirtualTime();
//				s2 = EventNotifier.getVtfTime();
				
				
				ExplicitHostProfiler.syncLoops(loops);
				
				
//		  		e1 = EventNotifier.getThreadVirtualTime();
//				e2 = EventNotifier.getVtfTime();
//				System.out.println((e1-s1)+ " " + (e2-s2));
//				
				stop = getTime();
				break;
				
			case RECURSIVE_DELAY:
				temp += recursiveWarmup(5000);
				temp = temp +1;
				start = getTime();
				temp += ExplicitHostProfiler.recursiveLoops(loops);
				temp += 1;
				stop = getTime();
				break;
			case YIELD_DELAY:
				temp += yieldWarmup(5000);
				temp = temp +1;
				
				threads = new Thread[threadsNum];
				for (int i =0 ; i<threadsNum ; i++) {
					threads[i] = new Thread(new ExplicitHostProfiler(loops));
				}
				start = getTime();
				for (int i =0 ; i<threadsNum ; i++) {
					threads[i].start();
				}

				try {
					for (int i =0 ; i<threadsNum ; i++) {
						threads[i].join();
					}
				} catch (InterruptedException e) {

				}
				stop = getTime();
				break;	
			case THREAD_START_DELAY:
				temp = temp +1;
				
				threads = new Thread[threadsNum];
				for (int i =0 ; i<threadsNum ; i++) {
					threads[i] = new Thread(new ExplicitHostProfiler(0));
				}
				start = getTime();
				for (int i =0 ; i<threadsNum ; i++) {
					threads[i].start();
				}

				try {
					for (int i =0 ; i<threadsNum ; i++) {
						threads[i].join();
					}
				} catch (InterruptedException e) {

				}
				stop = getTime();
				break;	
			default:break;
				
		}
		
//		System.out.println((diff1)/1000000000.0 + " " + (stop-start)/1000000000.0);
		
		
//		System.out.println((stop-start)/1000000000.0);
		
	}
	
}
