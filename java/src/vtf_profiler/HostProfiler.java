package vtf_profiler;
import java.io.PrintStream;
import java.util.*;

import org.objectweb.asm.MethodVisitor;

import virtualtime.EventNotifier;

//import virtualtime.EventNotifier;
public class HostProfiler implements Runnable {

	private static boolean warmupPeriod = false;
    static {
	try {
		if (System.getProperty("warmup").equals("true")) {
			warmupPeriod = true;
		}
	} catch (Exception e) {
		warmupPeriod = false;
	}
    }    	
	
	private static Random ra = new Random();
	/*
	 * Basic looping method 
	 */
	
	private int threadLoops = 0;
	public HostProfiler(int _threadLoops) {
		threadLoops = _threadLoops;
	}
	public void run() {
		double temp = 0.0;
		temp += HostProfiler.yieldLoops(threadLoops);
	}
	
	
	public static double recursiveInner(long loopCount) {
		if (loopCount == 0) {
			return 1;
		} else {
			return recursiveInner(loopCount-1)+1;
		}
	}	
	public static double recursiveWarmup(long loopCount) {
		if (!warmupPeriod) return 0.0;
		double temp = 0.0;
    	for (int i =0 ; i< 4; i++) {
    		temp += recursiveInner(loopCount);
    	}
    	return temp;
	}
	public static double recursiveLoops(long loopCount) {
		double temp = 0.0;
		for (int i = 0; i<loopCount; ++i) {
			//temp += recursiveInner(2000);
			temp += recursiveInner(2);
		}
    	return temp;
	}
	
	
	
	
	public static double yieldWarmup(long loopCount) {
		if (!warmupPeriod) return 0.0;
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
		if (!warmupPeriod) return 0.0;
    	double temp = 0.0;
    	for (int i =0 ; i< 4; i++) {
    		temp += inner(loopCount);
    	}
    	return temp;
	}
	public static double myLoops(long loopCount) {
    	double temp = inner(loopCount);
    	return temp;
	}
	
	static int indx[];  // Index into random keys  

	public static double inner(long loopCount) {
		
		double temp = 0.0;
  		for(int i = 0; i<loopCount; i++) {
  			temp += HostProfiler.doit(i);
  		}
  		temp += 1;
  		return temp;
	}
	
	public static double doit(double i) {
		
//		int limit = ra.nextInt(10);
//		for (int k = 0;k<limit;k++) {
//			i = Math.sqrt(i * (8.0+i)) / Math.pow(i+1.0,2);
		i = (i * (8.0+i)) / (i+1.0);
//		}	
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
		if (!warmupPeriod) return 0.0;
    	double temp = 0.0;
    	for (int i =0 ; i< 4; i++) {
      		for(i = 0; i<loopCount; i++) {
      			temp += HostProfiler.syncdoit(i, true);
      		}
    	}
    	return temp;
	}
	public static double syncLoops(long loopCount) {
		
		double temp = 0.0;
    	double i;
  		for(i = 0; i<loopCount; i++) {
  			temp += HostProfiler.syncdoit(i, true);
  			if (temp > 1000) {
  				temp += 1;
  			}
  		}
  		temp += 1;

  		return temp;
  		
	}
	public synchronized static double syncdoit(double i, boolean flag) {
		if (flag) {
			i = (i * (8.0+i)) / (i+1.0);
		}
		return i;
	}	
	


	// Synchronization points
	public static double innerSyncWarmup(long loopCount) {
		if (!warmupPeriod) return 0.0;
    	double temp = 0.0;
    	for (int i =0 ; i< 4; i++) {
      		for(i = 0; i<loopCount; i++) {
      			synchronized(HostProfiler.class) {
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
  			synchronized(HostProfiler.class) {
				temp += (i * (8.0+i)) / (i+1.0);
			}
  		}
  		temp += 1;
  		return temp;
	}
	
	public Object sssss() {
		return this;
	}
	
	public boolean returnBool() {
		return true;
	}

	public int getInt() {
		int i = 54023;
		System.out.println("heewlo");
		i = i*2;
		return i;
	}
	
	public String[] ffffffffff(String s) {
		return s.split("_");
	}
	
	short getS() {
		short gdsa = 24;
		return gdsa;
	}
	
	float getF() {
		float gdsa = 1.1F;
		return gdsa;
	}
	
	char getC() {
		char gdsa = 'c';
		return gdsa;
	}
	
	public static HostProfiler sssss2() {
		return (HostProfiler)new Object();
	}
	
	/*
			if (isSynchronized) {
				
	        	myVisitor.newMethod(cname, "_vtfsynced_"+name, desc, false);
	        	MethodVisitor mv2 = myVisitor.visitMethod(access, name, desc, signature, exceptions);
	        	callBasicBlockAdapter(mv2, access, name, desc, signature, exceptions);
	        	
	        } else if (!name.contains("init>")) {		// TODO: wrap constructors correctly
	        	MethodVisitor mv2 = null;
	        	if (name.equals("<init>")) {
	        		myVisitor.newMethod(cname, "_vtfmethod_constructor", desc, false);
	        		mv2 = myVisitor.visitMethod(access, "_vtfmethod_constructor", desc, signature, exceptions);
	        	} else {
	        		myVisitor.newMethod(cname, "_vtfmethod_" + name, desc, false);
	        		mv2 = myVisitor.visitMethod(access, "_vtfmethod_" + name, desc, signature, exceptions);	        		
	        	}
	        	
	        	callBasicBlockAdapter(mv2, access, name, desc, signature, exceptions);
	        	mv2.visitMaxs(0,0);

	        } 
	 */
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
		return System.nanoTime();
//		return EventNotifier.getThreadVirtualTime();
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
	

	public static void main(String[] argv) {
		if (argv.length != 2 && argv.length != 3) {
			System.out.println("Error: HostProfiler syntax: java vtf_profiler.HostProfiler <original|sync|methodsyn|iodelay|recursive|yield> <# loops> [<#threads>]");
			return;
		}

		indx = new int[10];
		for (int i =0 ; i< 10; i++) {
			indx[i] = ra.nextInt();
		}
				
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

		double temp = 0.0;

		switch (profilingVersion) {
			case INTERACTION_POINT_DELAY:
				temp += innerSyncWarmup(1000000);
				temp = temp +1;	
				start = getTime();
				HostProfiler.innerSyncLoops(loops);
				stop = getTime();
				break;
				
			case METHOD_DELAY:
				temp += warmup(100000);
				temp = temp +1;
				start = getTime();
				temp += HostProfiler.myLoops(loops);
				stop = getTime();
				break;
				
			case SYNCHRONIZED_METHOD_DELAY:
				temp += innerSyncWarmup(100000);
				temp = temp +1;
				
				start = getTime();
				temp += HostProfiler.syncLoops(loops);	
				stop = getTime();
				break;
				
			case RECURSIVE_DELAY:
				temp += recursiveWarmup(5000);
				temp = temp +1;
				start = getTime();
				temp += HostProfiler.recursiveLoops(loops);
				temp += 1;
				stop = getTime();
				break;
			case YIELD_DELAY:
				temp += yieldWarmup(5000);
				temp = temp +1;
				
				threads = new Thread[threadsNum];
				for (int i =0 ; i<threadsNum ; i++) {
					threads[i] = new Thread(new HostProfiler(loops));
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
					threads[i] = new Thread(new HostProfiler(0));
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
		System.out.println((stop-start)/1000000000.0);
		
	}
	
}
