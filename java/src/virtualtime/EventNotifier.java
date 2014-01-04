/*
 * Created on Jun 9, 2008
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package virtualtime;

import java.io.PrintStream;

public class EventNotifier {
	public static final boolean debug = false;		
	
	/********
	 * 
	 * Profile instrumentation code
	 * 
	 */
	public static void profileInstrumentationCode(String profilingType) {
		//System.out.println("Profiling JINE instrumentation code!");
		try {
			System.out.println("Starting profiling instrumentation delays at host by thread " + Thread.currentThread().getName() + " using ***" + profilingType + "***");
			System.out.println("[Note: if running in debug mode, disable logging to avoid excessive delays (if GDB_USAGE=1, set DISABLE_LOGGING=1)]");
			long profilingStartTime = System.nanoTime();
			long methodInstrumentationDelay = InstrumentationProfilerFactory.createInstrumentationProfiler(profilingType, "method").profile();
			long ioMethodInstrumentationDelay = InstrumentationProfilerFactory.createInstrumentationProfiler(profilingType, "io").profile();
			long ipInstrumentationDelay = InstrumentationProfilerFactory.createInstrumentationProfiler(profilingType, "ip").profile();
			setInstrumentationDelays(methodInstrumentationDelay, ioMethodInstrumentationDelay, ipInstrumentationDelay);
			long profilingEndTime = System.nanoTime();
			System.out.println("Completed profiling instrumentation delays at host after " + (double)(profilingEndTime-profilingStartTime)/1e9 + " sec");
			System.out.println("Delays in ns for methods - I/O - Interaction Points: " + methodInstrumentationDelay + " - " + ioMethodInstrumentationDelay + " - " + ipInstrumentationDelay);
		
		} catch (Exception ae) {
			ae.printStackTrace(System.out);

		}
	}
	
	private static native void setInstrumentationDelays(long m, long i, long ip);
	
	
	/*********
	 * 
	 * Registering methods
	 * 
	 * These methods register:
	 * - Method ids to Method FQNs and method0 (id of the first instrumented method) of the belonging class 
	 * - Method ids to Method Time Scaling Factors
	 * - Method ids to queueing network XML descriptions
	 * - VEX invalidation policy
	 * - Whether the JVM executes in interpreter-only mode   
	 */
	private static native void _registerMethod(String fqn, int methodId, int method0);
	public static void registerMethod(String fqn, int methodId, int method0) {
		try {
			_registerMethod(fqn, methodId, method0);
		} catch (UnsatisfiedLinkError e) {
			e.printStackTrace();
		}
	}

	// Register method Time Scaling Factor (TSF)
	private static native void _registerMethodVirtualTime(int methodId, double tsf);
	public static void registerMethodVirtualTime(int methodId, double tsf) {
		if (tsf > 0.0 && tsf != 1.0) { 
			_registerMethodVirtualTime(methodId, tsf);
		}
	}
	
	// Register model file for method
	private static native void _registerMethodPerformanceModel(int methodId, String modelFileName, int customerClass, String sourceNodeLabel);
	public static void registerMethodPerformanceModel(int methodId, String modelFileName, int customerClass, String sourceNodeLabel) {
		_registerMethodPerformanceModel(methodId, modelFileName, customerClass, sourceNodeLabel);
	}


	// Register the profiling invalidation policy: =samples, :abs_time, :rel_percent
	private static native void _registerProfilingInvalidationPoilcy(String invalidationPolicyInfo);
	public static void registerProfilingInvalidationPoilcy(String invalidationPolicyInfo) {
		_registerProfilingInvalidationPoilcy(invalidationPolicyInfo);
	}
	
	private static native void _registerExcludedThreadName(String excludedThread);
	public static void registerExcludedJvmSystemThreads() {
		String javaVendor = System.getProperty("java.vendor"); 
		if (javaVendor.equals("Sun Microsystems Inc.")) {
			_registerExcludedThreadName("DestroyJavaVM");
		} else if (javaVendor.equals("IBM Corporation")) {
			_registerExcludedThreadName("DestroyJavaVM helper thread");
			_registerExcludedThreadName("Attach API wait loop");
			_registerExcludedThreadName("Attach API initializer");
		}
	}
	public static native boolean forcingNoWaitBecauseNotUsingScheduling();
	public static native boolean notRelyingOnJvmtiForMonitorHandling();
	
	// Register to VEX that the resulting profile should be per stack trace and not per method FQN
//	public static native void enableStackTraceMode();
	
	// Register lineLevelStackTrace is used to enable the prediction scheme for I/O method to be
	// differentiated for the same method stack trace, according to the lines in each method
	// where calls were made.   
	private static boolean lineLevelStackTrace = false;
	public static void enableLineLevelStackTrace() {
		lineLevelStackTrace = true;
	}
	
	
	// Methods called by java/lang/Thread instruments
	public static native void getTimeBeforeCreatingThread(long threadToBeSpawnedId);
	public static native void getTimeAfterCreatingThread();
	public static native void logJoiningThread(long joiningThreadId);
	
	// Method called before a thread that should be profiled is spawned, setting a flag to 
	// denote to the VTF agent that this thread's execution should be controlled and monitored
	// If we move the condition of the thread name into JINE,
	// the runtime complains that we are trying to acquire information on threads before the LIVE PHASE.
//	public static void beforeThreadStart(Thread thread) {
//		String name = thread.getName();
//		if (!name.equals("Reference Handler") && !name.equals("Finalizer")) {
//			thread.setVtfProfiled(true);
//			if (debug) { 
//				System.out.println("Profiling thread " + thread.getName() + " (" + thread.getId() + ")"); 
//			}
//		}
//	}	
	
	/***********************
	 * 
	 * Main method-wrapping methods that are used for VEX profiling
	 *  
	 */
	public final static native void _afterMethodEntry(int methodId);
	public final static native void _beforeMethodExit(int methodId);
	
	public final static native void _enterPerfModel(int methodId);
	public final static native void _exitPerfModel(int methodId);
		
	/*
	 * I/O method-wrapping methods that are used for I/O method simulation
	 * 
	 * The afterIoMethodEntry method is called whenever native I/O calls are triggered
	 * It contacts the agent to predict or profile the duration of this call, if and only if
	 * this I/O operation belongs to one of our profiling threads. 
	 * 
	 * For example, JVM threads that perform I/O operations should not trigger this
	 * 
	 * NOTE: If the writeBytes method is instrumented in the FileOutputStream class then NO System.out.println 
	 * 		 call should be made anywhere in EventNotifier 
	 */ 
	private static native void _afterIoMethodEntry(int methodId, int invocationPointHashValue);
	public static void afterIoMethodEntry(Thread curThread, int methodId) {

		if (curThread.getVtfIoInvocationPoint() > 0) { // && curThread.getVtfStackDepth() > 0) {
			
			if (lineLevelStackTrace) {
				int stackline_sum = 0;
				StackTraceElement[] st_array = curThread.getStackTrace();
				int stackTraceLength = st_array.length;
				
				for (int i =2 ;i<stackTraceLength; i++) {	// the last two methods are getStackTrace() and Thread.getStackTrace();
					stackline_sum += st_array[i].getLineNumber();
				}
				_afterIoMethodEntry(methodId, stackline_sum+curThread.getVtfIoInvocationPoint());
			} else {
				
				_afterIoMethodEntry(methodId, curThread.getVtfIoInvocationPoint());
			}
			
		}
	}
	

	private static native void _afterWritingIoMethodEntry(int methodId, int invocationPointHashValue, int fd, int bytesToWrite);
	public static void afterWritingIoMethodEntry(Thread curThread, int methodId, Object fileDescriptor, int bytesToWrite) {

		if (curThread.getVtfIoInvocationPoint() > 0) { // && curThread.getVtfStackDepth() > 0) {
			
			if (lineLevelStackTrace) {
				int stackline_sum = 0;
				StackTraceElement[] st_array = curThread.getStackTrace();
				int stackTraceLength = st_array.length;
				
				for (int i =2 ;i<stackTraceLength; i++) {	// the last two methods are getStackTrace() and Thread.getStackTrace();
					stackline_sum += st_array[i].getLineNumber();
				}
				_afterWritingIoMethodEntry(methodId, stackline_sum+curThread.getVtfIoInvocationPoint(), fileDescriptor.hashCode(), bytesToWrite);
			} else {
				
				_afterWritingIoMethodEntry(methodId, curThread.getVtfIoInvocationPoint(), fileDescriptor.hashCode(), bytesToWrite);
			}
			
		}

	}

	private static native void _beforeIoMethodExit(int methodId);
	public static void beforeIoMethodExit(Thread curThread, int methodId) {
		// Needed to filter-out methods that are called from classes besides the profiled ones (for example the ones before main)
		if (curThread.getVtfIoInvocationPoint() > 0) { // && curThread.getVtfStackDepth() > 0) {
			_beforeIoMethodExit(methodId);
			curThread.setVtfIoInvocationPoint(0);	// so that you don't enter the next point unless it's profiled
		} 
	}

	/*
	 * beforeSocketRead: Called before a socket calls the native call to read from any socket stream
	 * to denote the socket timeout value
	 * @param timeout: the defined timeout for the operation or 0 if no timeout is defined
	 */
	private static native void _beforeSocketRead(int methodId, int ioInvocationPoint, int timeout);	
	public static void beforeSocketRead(Thread curThread, int methodId, int timeout) {    
		if (curThread.getVtfIoInvocationPoint() > 0) {
			_beforeSocketRead(methodId, curThread.getVtfIoInvocationPoint(), timeout);
		}

	}

	private static native void _beforeSocketRead(int methodId, int ioInvocationPoint, int fd, int bytesToRead, int timeout);	
	public static void beforeSocketRead(Thread curThread, int methodId, Object fileDescriptor, int bytesToRead, int timeout) {    
		if (curThread.getVtfIoInvocationPoint() > 0) {
			_beforeSocketRead(methodId, curThread.getVtfIoInvocationPoint(), fileDescriptor.hashCode(), bytesToRead, timeout);
		}
//		} else {
//			// Socket read invoked from methods that are not instrumented	
//			System.out.println("Thread " + curThread.getName() + "(" + curThread.getId() + ") snubs before SOCKETREAD because inv point = " + curThread.getVtfIoInvocationPoint());
//			getStackTrace(System.out);
//		}
	}
	
	
	/************************
	 * 
	 * Synchronization calls - all these lead to threads being suspended
	 * 
	 */
	// If interaction points are actually monitored 
	public static native void _interactionPoint();
	public static void interactionPoint() {
		_interactionPoint();
	}

	// Special handling for yield
	public static native void _yield();

	//Methods to synchronise various calls in virtual time, like accept-connect, epoll_wait-interrupt
	public static native void beforeSocketAccept();
	public static native void afterSocketAccept();
//	public static void beforeSocketAcceptOnFd(Object fileDescriptor) throws InterruptedException, IllegalArgumentException { 
//		_beforeSocketAcceptOnFd(fileDescriptor.hashCode());  
//	}
	
//	private static native boolean _beforeSocketAcceptOnFd(int objectId);
//    private static native void _beforeSocketConnectOnFd(int objectId);
//    public static void beforeSocketConnectOnFd(Object fileDescriptor) {
//    	_beforeSocketConnectOnFd(fileDescriptor.hashCode());
//    }
	
	private static native boolean _beforeEpollWaitingOn(int objectId, long timeout);

	public static void beforeEpollWaitingOn(int fd, long timeoutInMilliSeconds) throws InterruptedException, IllegalArgumentException {
			_beforeEpollWaitingOn(fd, timeoutInMilliSeconds);  
	}
	public static native void afterEpollWaiting();
	public static native void _beforeInterruptingFd(int fd);
	
	
	//	park: block current thread, returning:
	//	- when a balancing unpark occurs, 
	//	- if a balancing unpark has already occurred, 
	//	- if the thread is interrupted, 
	//	- if !isAbsolute and timeout is not zero, the given time nanoseconds have elapsed, 
	//	- if isAbsolute, the given deadline in milliseconds since Epoch has passed, 
	//	- or spuriously (i.e., returning for no "reason"). 
	// Note: in contrast to other calls, timeout here is in nanoseconds!!!!
	public static native void park(boolean isAbsolute, long timeout);
	public static native void parked();

	public static void vexpark(long timeout, boolean isAbsolute) {
		if (isAbsolute) {
			System.out.println("PARK IS ABSOLUTE WHICH IS NOT CORRECTLY HANDLED BY VEX " + isAbsolute + " " + timeout);
		}
//		System.out.println("PARK " + isAbsolute + " " + timeout);
		park(isAbsolute, timeout);
//		Unsafe.getUnsafe().park(isAbsolute, timeout);
	}
	
	// unpark: Unblock the given thread blocked on park, or, 
	// if it is not blocked, cause the subsequent call to park not to block.  
	public static native void unpark(long objectId);
	public static void vexunpark(long objectId) {
		unpark(objectId);
//		Unsafe.getUnsafe().unpark(objectId);
	}
	
	public static void vexunpark(Object thread) {
		
		long threadId = ((Thread)thread).getId();
		System.out.println("UNPARK " + threadId + ((Thread)thread).getName());
		vexunpark(threadId);
	}
	
	/*
	 * Timed waiting methods
	 * 
	 * These methods are used to simulate the behaviour of the timed-waiting Object.wait(t) and Thread.sleep(t) in 
	 * virtual time. This means that t should elapse in virtual time and not in real time.
	 * 
	 * The approach is to register when the thread should timeout in virtual time and then wait indefinitely, until
	 * the VTF's scheduler wakes up the thread.
	 * 
	 * However, if another thread notifies or interrupts a waiting thread then the wake-up time should be the correct
	 * in virtual time. Therefore, we use the waking up methods below
	 * 
	 *  We have two arguments for the timeout arguments, because of some very high timeoutMillis values (LONG_MAX) that
	 * led to overflow on the Java level. Passing the arguments separately we ensure that.
	 *
	 */
	private static native boolean _waitInVirtualTime(Object obj, int objectId, long threadId, long timeout, int nanos);

	/*
	 * Simulate Object.wait (timed) in virtual time. Release the monitor and re-acquire it after timeoutInMilliseconds ms.
	 * 
	 * @param timeoutInMilliSeconds: Object.wait(0) leads to endless waiting.
	 * @throws InterruptedException: if the thread is interrupted while waiting in Virtual Time
	 * @throws IllegalArgumentException: if timeoutInMilliSeconds < 0
	 */
	public static void waitInVirtualTime(final Object obj) throws InterruptedException {
		waitInVirtualTime(obj, 0, 0);
	}
	
	public static void waitInVirtualTime(final Object obj, long timeoutInMilliSeconds) throws InterruptedException, IllegalArgumentException {
		waitInVirtualTime(obj, timeoutInMilliSeconds, 0);
	}
	
	public static void waitInVirtualTime(final Object obj, long timeoutInMilliSeconds, int nanos) throws InterruptedException {
		if (timeoutInMilliSeconds < 0) {
			throw new IllegalArgumentException("timeout value is negative");
		}
		if (obj == null) {
			throw new NullPointerException("VEX did not find object id. Did you do: ((Object)null).wait()?");
		} else {
			boolean timedOut = _waitInVirtualTime(obj, System.identityHashCode(obj), Thread.currentThread().getId(), timeoutInMilliSeconds, nanos);  
			wasThreadInterrupted(timedOut);
		}		
	}	

	
	/*
	 * Waiting calls when the limitedJvmtiUsage parameter is set
	 */
	private static native boolean _waitInVirtualTimeWithoutJvmti(Object obj, int identityHashCode, long id, long timeoutInMilliSeconds, int nanos, boolean useLowLevelMonitors);
	
	public static void waitInVirtualTimeWithoutJvmti(final Object obj, boolean useLowLevelMonitors) throws InterruptedException {
		waitInVirtualTimeWithoutJvmti(obj, 0, 0, useLowLevelMonitors);
	}
	
	public static void waitInVirtualTimeWithoutJvmti(final Object obj, long timeoutInMilliSeconds, boolean useLowLevelMonitors) throws InterruptedException, IllegalArgumentException {
		waitInVirtualTimeWithoutJvmti(obj, timeoutInMilliSeconds, 0, useLowLevelMonitors);
	}

	public static void waitInVirtualTimeWithoutJvmti(final Object obj, long timeoutInMilliSeconds, int nanos, boolean useLowLevelMonitors) throws InterruptedException {
		if (timeoutInMilliSeconds < 0) {
			throw new IllegalArgumentException("timeout value is negative");
		}
		if (obj == null) {
			throw new NullPointerException("VEX did not find object id. Did you do: ((Object)null).wait()?");
			
		} else {
			System.err.println("edw mesa sto _waitInVirtualTimeWithoutJvmti exoume monitor id : "  + System.identityHashCode(obj) + " " + obj.hashCode() );
			boolean timedOut = _waitInVirtualTimeWithoutJvmti(obj, System.identityHashCode(obj), Thread.currentThread().getId(), timeoutInMilliSeconds, nanos, useLowLevelMonitors);  
			wasThreadInterrupted(timedOut);
		}		
	}
	
	
	public static void beforeNotifyWithoutMonitors(Object object) {
		_notifyTimedWaitingThreads(System.identityHashCode(object)); 
	}

	public static void beforeNotifyAllWithoutMonitors(Object object) {	
		_notifyAllTimedWaitingThreads(System.identityHashCode(object));
	}



	/*
	 * Simulate Thread.sleep in virtual time. 
	 * @param timeoutInMilliSeconds: There is a difference from Object.wait, to that Thread.sleep(0) does not have any effect,
	 * while Object.wait(0) leads to endless waiting.
	 * @throws InterruptedException: if the thread is interrupted while sleeping in Virtual Time
	 * @throws IllegalArgumentException: if timeoutInMilliSeconds < 0
	 */
	public static void sleepInVirtualTime(long timeoutInMilliSeconds) throws InterruptedException, IllegalArgumentException {
		sleepInVirtualTime(timeoutInMilliSeconds,  0);
	}
	
	public static void sleepInVirtualTime(long timeoutInMilliSeconds, int nanos) throws InterruptedException, IllegalArgumentException {
		if (timeoutInMilliSeconds < 0) {
			throw new IllegalArgumentException("timeout value is negative");
		}
		if (timeoutInMilliSeconds+nanos > 0) {
			boolean timedOut = _waitInVirtualTime(null, 0, Thread.currentThread().getId(), timeoutInMilliSeconds, nanos);
			wasThreadInterrupted(timedOut);
		}
	}
	
	private static void wasThreadInterrupted(boolean timedOut) throws InterruptedException {
		if (!timedOut) {
			if (debug) System.out.println("virtual interrupt on timeout");
			throw new InterruptedException();
		}
		if (debug) System.out.println("Thread.sleep(t) virtual timeout");
	}
	
	/*
	 * Waking-up methods
	 * 
	 * These methods are handled by the VTF, because they might affect a thread that is waiting or sleeping for
	 * a period of time t in virtual time. We need to make sure that if this thread is blocked then it will be awaken, 
	 * if a notify(), notifyAll() or Thread.interrupt() method is sent and will throw an InterruptedException 
	 * . 
	 */
	private static native boolean _notifyTimedWaitingThreads(int objectId);
	public static void beforeNotify(Object object) {

		boolean foundTimedWaitingThread = _notifyTimedWaitingThreads(System.identityHashCode(object));
		if (!foundTimedWaitingThread) {	// if a VTF timed-waiting thread was interrupted by this notify, then do not send the notify again
			object.notify();	 
		} 
	}

	private static native boolean _notifyAllTimedWaitingThreads(int objectId);
	public static void beforeNotifyAll(Object object) {		
		_notifyAllTimedWaitingThreads(System.identityHashCode(object));
		object.notifyAll();		// in any case all threads will be notified
								// however, if a timed-waiting thread was woken up it might have entered the object monitor first
	}

	private static native boolean _beforeThreadInterrupt(long interruptedThreadId);
	public static void beforeThreadInterrupt(Thread thread) {
		_beforeThreadInterrupt(thread.getId());
//		return (_beforeThreadInterrupt(thread.getId()))?1:0;
	}
	

	/*
	 * Generic synchronization points management following the "update-before-the-event" principle
	 */
	public static native void _beforeAcquiringMonitor(int objectId);
	public static native void _beforeReleasingMonitor(int objectId);
	
	public static void beforeAcquiringMonitor(Object obj) {
		System.err.println("edw mesa sto acquire exoume monitor id : "  + System.identityHashCode(obj) + " " + obj.hashCode() );
		_beforeAcquiringMonitor(System.identityHashCode(obj));
	}
	public static void beforeReleasingMonitor(Object obj) {
		System.err.println("edw mesa sto release exoume monitor id : "  + System.identityHashCode(obj) + " " + obj.hashCode() );
		_beforeReleasingMonitor(System.identityHashCode(obj));
	}
	
	
	/*
	 * Cleanup method
	 */
	public static native void resetPerformanceMeasures();
	public static native void writePerformanceMeasures(String filename);

	
	
	/*
	 * Utility functions
	 */
	public static void getStackTrace() {
		getStackTrace(System.out);
	}
	
	public static void getStackTrace(PrintStream output) {
		Thread curThread = Thread.currentThread();
		StackTraceElement[] st_array = curThread.getStackTrace();
		int stackTraceLength = st_array.length;
		StackTraceElement st;
		for (int i = 2 ;i<stackTraceLength; i++) {	// the last two methods are getStackTrace() and Thread.getStackTrace();
			st = st_array[i];
			output.println(st.getClassName()+"."+st.getMethodName() + "():" +st.getLineNumber() + "<-");
		}
		System.out.println();
	}
	
	
	static long totalInstrumentationCPUTime = 0;
	static int totalInstrumentations = 0;
	public static native void onInstrumentationStart();
	public static native void onInstrumentationEnd();
	
	
	/***
	 * 
	 * Time returning calls
	 * 
	 */
	
	public static long getThreadVirtualTime() {
		return _getVtfTime();
	}
	
	private static native long _getPapiCpuTime();
	public static long getPapiCpuTime() {
		return _getPapiCpuTime();
	}
	
	private static native long _getPapiRealTime();
	public static long getPapiRealTime() {
		return _getPapiRealTime();
	}

	public static native long _getVtfTime();
	public static native long _getVtfTimeInMillis();

	public static long getVtfTime() {
		return _getVtfTime();
	}	
	
	public static long nanoTime() {
		return _getVtfTime();
	}
	
	public static long currentTimeMillis() {
		return _getVtfTimeInMillis();
	}
		
	public static void printException(Throwable e, int methodId) {
		System.out.println(e.getLocalizedMessage());
		e.printStackTrace(System.out);
	}

	
	/*
	 * Used to simulate the duration of I/O by letting the thread sleep (using the Thread.sleep
	 * within the VTF will make a leap) - debugging method
	 */
	public static native void _simulateIoDuration(int duration);
	
	
	/****
	 * 
	 * Methods regarding indefinite iterations of VEX to find optimal method scalings
	 * TODO: currently only experimental
	 */
	public static native boolean _shouldMainIterationContinue();
	
	// Export the results from this iteration of main
	public static native void _exportAndProcessMainIterationResults();

}

