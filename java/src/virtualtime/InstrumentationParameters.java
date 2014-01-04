package virtualtime;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.Iterator;
import java.util.Vector;
import java.util.HashSet;
/*
 * Class used to encapsulate the parameters of JINE instrumentation
 */
public class InstrumentationParameters {
	private static final boolean debug = false;
	
    private static Vector<String> subclassesOfJavaLangThread; 
    public void addPossibleSubclassOfJavaLangThread(String className, String superClass) {
    	synchronized(subclassesOfJavaLangThread) {
    		if (superClass.equals("java/lang/Thread") || subclassesOfJavaLangThread.contains(superClass)) {
    			System.out.println("adding subclass " + className + " of " + superClass);
    			subclassesOfJavaLangThread.add(className);
    		}
    	}
    	
    }
    public boolean isExtendindJavaLangThread(String className) {
    	boolean returnValue = false;
       	synchronized(subclassesOfJavaLangThread) {
    		returnValue = subclassesOfJavaLangThread.contains(className);
    	}
    	return returnValue;
    }    

	private boolean transformWaits;
	private boolean profilingRun;
	private boolean adaptiveProfiling;
	private boolean externalInstrumentation; 	// a method is wrapped externally with the profiling instruments (so that JIT is not affected) 
	private boolean trapInteractionPoints; 
	private boolean isOptimizerEnabled;
	private boolean vexOptimizerSet;

//	private boolean stackTraceMode;
	
	private Vector<String> profiledMethodNames;
	private Vector<MethodAdaptationInfo> adaptedMethodNames;
	
	private HashSet<Integer> invalidatedMethodIds;
	private HashSet<Integer> recursiveMethodIds;
	
	private int classVersion;
	private int method0;
	
	private int retransformingMethodId;
	private boolean recursiveRetransformation;
	private boolean instrumentExceptionHandlingCodeEverywhere;
	private boolean modifyTimeCalls;
	private boolean jvmtiUsage;
	private boolean removingMonitors;
	
	private String retransformingMethodName;
	private String retransformingMethodDesc;
	
	protected boolean subclassOfJavaLangThread;			// small hack to identify sleep methods when the direct superclass is java.lang.Thread - we avoid the entire tree to avoid synchronization points on a common data structure 
	
	
	// Default options
	public InstrumentationParameters() {
		subclassOfJavaLangThread = false;
		transformWaits		 	= true;
		profilingRun 			= false;
		adaptiveProfiling 		= false;
		externalInstrumentation = false;
		trapInteractionPoints 	= false;
//		stackTraceMode 			= false;
		isOptimizerEnabled		= false;
		
		vexOptimizerSet 		= false;
		subclassesOfJavaLangThread = new Vector<String>();
		
		adaptedMethodNames  = new Vector<MethodAdaptationInfo>();
		profiledMethodNames = new Vector<String>();
	
		invalidatedMethodIds = new HashSet<Integer>(); 
		recursiveMethodIds   = new HashSet<Integer>(); 
		
		classVersion = 1;
		method0 = 0;
		
		modifyTimeCalls = true;	
		recursiveRetransformation = false;
		retransformingMethodId = 0;
		jvmtiUsage = true;
		removingMonitors = false;
		
		instrumentExceptionHandlingCodeEverywhere = false;
	} 

	public void removeMonitors() {
		removingMonitors = true;
	}

	public boolean shouldRemoveMonitors() {
		return removingMonitors;
	}
	
	public void limitJvmtiUsage() {
		jvmtiUsage = false;		
	}
	
	public boolean onlyLimitedJvmtiUsage() {
		return !jvmtiUsage;		
	}
	
	public boolean isRetransformingInstrumentation() {
		return retransformingMethodId != 0;
	}

	public int getRetransformingMethodId() {
		return retransformingMethodId;
	}
	
	public boolean isRecursiveRetransformation() {
		return recursiveRetransformation;	
	}
	
	public void setInstrumentExceptionHandlingEverywhere() {
		instrumentExceptionHandlingCodeEverywhere = true;
	
	}
	
	public void setWhetherJavaLangThreadIsSuperclass(String superName) {
		subclassOfJavaLangThread = superName.equals("java/lang/Thread"); 
	}
	
	public boolean isSubClassOfJavaLangThread() {
		return subclassOfJavaLangThread;
	}
	
	public void disableTimeCallsModification() {
		modifyTimeCalls = false;
	}
	
	public boolean shouldModifyTimeCalls() {
		return modifyTimeCalls;
	}
	
	public void setMethod0(int m) {
		method0 = m;
	}
	
	public int getMethod0() {
		return method0;
	}
	
	public void setTransformWaits(boolean value) {
		transformWaits = value;
	}
	
	public void setProfilingRun(boolean value) {
		profilingRun = value;
	}
	
	public void setExternalInstrumentation(boolean value) {
		externalInstrumentation = value;
	}
	
	public void setAdaptiveProfiling(boolean value) {
		adaptiveProfiling = value;
	}
	
	
//	public void setStackTraceMode(boolean value) {
//		stackTraceMode = value;
//	}
//	
//	public boolean inStackTraceMode() {
//		return stackTraceMode;
//	}
//	
	public void setInteractionPointsTrapping(boolean value) {
		trapInteractionPoints = value;
	}
	
	public void setVersion(int version) {
		classVersion = version;
	}
	

	public void setRetranformationInfo(int methodId, boolean _recursiveAdaptation, int _method0, String fqn) {
		retransformingMethodId = methodId;
		method0 = _method0;
		recursiveRetransformation = _recursiveAdaptation;
				
		String[] fqnParts = fqn.split(" ");
		retransformingMethodName = fqnParts[1];
		retransformingMethodDesc = fqnParts[2];
	}
	
	public String getRetransformingMethodName() {
		return retransformingMethodName;
	}
	
	public String getRetransformingMethodDesc() {
		return retransformingMethodDesc;
	}
	
    public boolean shouldInvalidateMethodProfilingOf(int methodId) {
    	return isRetransformingInstrumentation() && !isRecursiveRetransformation() && (isInvalidatedMethod(methodId));	//methods to be invalidated are immediately in the isInvalidatedMethod vector 
    }

	public void cleanupRetranformationInfo() {
		retransformingMethodId = 0;
		method0 = 0;
		recursiveRetransformation = false;
	}
	
	public int getVersion() {
		return classVersion;
	}
	
	public boolean isProfilingRun() {
		return profilingRun;
	}
	
	public boolean shouldTransformWaits() {
		return transformWaits;
	}
	
	public boolean usingAdaptiveProfiling() {
		return adaptiveProfiling;
	}
	
	public boolean usingExternalInstrumentation() {
		return externalInstrumentation;
	}
	
	public boolean trappingInteractionPoints() {
		return trapInteractionPoints;
	}
		
	/*
	 * Check if the method should be profiled according to the options parsed
	 */
	public boolean shouldMethodBeProfiled(String className, String methodName) {
		return methodName.equals("main") || ( !methodName.startsWith("access$") && (profiledMethodNames.size() == 0 || profiledMethodNames.contains(className+methodName)));
	}
	
	/*
	 * Check if a file is used to determine the acceleration factors of methods
	 */	
	public boolean usingAdaptedMethodsFile() {
		return (adaptedMethodNames.size() > 0);
	}
	
	/*
	 * Method used to select methods that should be profiled according to FQN found in the argument file
	 */
	public void loadMethodFiltering(String filename) {
		File file = new File(filename);

		BufferedReader bufRdr = null;
		try {
			bufRdr = new BufferedReader(new FileReader(file));

			String line = null;
			String className = null;
			String methodName = null;
			int lastSlash;
			// Reading threads ids - thread names
			debugLog("Method filtering file parsing:" + filename);


			while((line = bufRdr.readLine()) != null) {
				if (line.equals("") || line.charAt(0) == '#') {
					continue;
				}
				//if ((lastSlash = line.lastIndexOf('/')) != -1) {
				lastSlash = line.indexOf(' ');
				className = line.substring(0, lastSlash);

				methodName = line.substring(lastSlash+1);
				lastSlash = methodName.indexOf(' ');
				if (lastSlash != -1) {
					methodName = methodName.substring(0, lastSlash);
				}
				profiledMethodNames.add(className + methodName);
			}
	
			int length = profiledMethodNames.size();
			if (length != 0) {
				for (int i = 0 ;i <length; i++) {
					debugLog("profiling method: " + profiledMethodNames.elementAt(i));
				}
			}		
		} catch (IOException e1) {
			e1.printStackTrace();
		}

	}
	
	
	/*
	 * Method used to adapt profiled methods according to FQNs and VTF factors found in the argument file
	 */
	public void loadMethodAdaptation(String filename) {
		File file = new File(filename);

		BufferedReader bufRdr = null;
		try {
			bufRdr = new BufferedReader(new FileReader(file));

			String line = null;
			String className = null;
			String methodName = null;
			String factorValue = null;
			int lastSlash;
			// Reading threads ids - thread names
			debugLog("Method adaptation file parsing:" + filename);

			while((line = bufRdr.readLine()) != null) {
				if (line.equals("") || line.charAt(0) == '#') {
					continue;
				}
				String originalLine = line;
				try {
					// classname
					lastSlash = line.indexOf(' ');
					className = line.substring(0, lastSlash);
					line = line.substring(lastSlash+1);

					// methodname
					lastSlash = line.indexOf(' ');
					methodName = line.substring(0, lastSlash);

					// factor
					factorValue = line.substring(lastSlash+1);
					
					try {
						double factor = Double.parseDouble(factorValue);
						adaptedMethodNames.add(new MethodAdaptationInfo(className + methodName, factor));
						
					} catch (NumberFormatException nfe) {
						
						line = line.substring(lastSlash+1);
						lastSlash = line.indexOf(' ');
						String modelName = line.substring(0, lastSlash);
						boolean replaceBody = Boolean.parseBoolean(line.substring(lastSlash+1));						
						adaptedMethodNames.add(new MethodAdaptationInfo(className + methodName, modelName, replaceBody));

					}
				} catch (Exception se) {
					System.err.println("Parsing of line: " + originalLine + " failed\nFormat is <classname> <methodname> [<scaling>|<model-file> <shouldreplacebody>]");
					se.printStackTrace(System.err);
					
				} 

			}
			
			int length = adaptedMethodNames.size();
			if (length != 0) {
				for (int i = 0 ;i <length; i++) {
					debugLog("adapting time of method: " + adaptedMethodNames.elementAt(i).getMethod() + " - " + adaptedMethodNames.elementAt(i).getFactor());
				}
			}
			
		} catch (IOException e1) {
			e1.printStackTrace();
		}

	}
	
	synchronized public void addMethodToBeInvalidated(int methodId) {
		invalidatedMethodIds.add(methodId);
	}
	
	synchronized public void addRecursiveMethodToBeInvalidated(int methodId) {
		recursiveMethodIds.add(methodId);
	}
	 
	synchronized public boolean isInvalidatedMethod(int methodId) {
		return invalidatedMethodIds.contains(methodId);
	}
	
	synchronized public boolean isRecursiveMethod(int methodId) {
		return recursiveMethodIds.contains(methodId);
	}
	
	synchronized public boolean methodShouldBeAdapted(int methodId) {
		return (!isInvalidatedMethod(methodId) && !isRecursiveMethod(methodId));
	}
	
	public boolean exceptionHandlingEverywhere() {
		return instrumentExceptionHandlingCodeEverywhere;
	}
	
	synchronized public void printAdaptedMethods() {
		System.out.print("Invalidated methods: ");
		Iterator<Integer> it = invalidatedMethodIds.iterator();
		while (it.hasNext()) {
			System.out.print(it.next() + " ");
		}
		System.out.println();
		
		System.out.print("Recursive methods: ");
		it = recursiveMethodIds.iterator();
		while (it.hasNext()) {
			System.out.print(it.next() + " ");
		}
		System.out.println();

//		
//		for (int i =0; i<invalidatedMethodIds.size(); ++i) {
//			System.out.print(invalidatedMethodIds.elementAt(i) + " ");	
//		}
//		
//		
//		
//		for (int i =0; i<recursiveMethodIds.size(); ++i) {
//			System.out.print(recursiveMethodIds.elementAt(i) + " ");
//		}
	}
	
	/*
	 * Utility vector parsing function
	 */
	public MethodAdaptationInfo getMethodAdaptationInfoOf(String fullname) {
		
		for (MethodAdaptationInfo info : adaptedMethodNames) {
			if (info.getMethod().equals(fullname)) {
				return info;
			}
		}
		return null;
	}
	

	public double getModelDescribingMethod(String fullname) {
		int adaptedMethodSize = adaptedMethodNames.size();
		MethodAdaptationInfo info;
		
		for (int i = 0; i < adaptedMethodSize; i++) {
			info = adaptedMethodNames.elementAt(i);
			if (info.getMethod().equals(fullname)) {
				double factor = info.getFactor();
				adaptedMethodNames.remove(i);
				adaptedMethodSize = adaptedMethodNames.size();
				return factor;
			}
		}
		return 1.0;
	}


	public MethodAdaptationInfo registerExternallyProvidedMethodPerformanceFactors(int methodId, String fullName) {
		
		if (usingAdaptedMethodsFile()) {
			MethodAdaptationInfo methodAdaptationInfo = getMethodAdaptationInfoOf(fullName);
			if (methodAdaptationInfo != null) {
				double factor = methodAdaptationInfo.getFactor();
				if (factor != 1.0) {
					EventNotifier.registerMethodVirtualTime(methodId, factor);		
				} else {
					String modelFileName = methodAdaptationInfo.getModelFilename();   
					EventNotifier.registerMethodPerformanceModel(methodId, modelFileName, 0, "");		
				}
				
			}
			return methodAdaptationInfo;
		}
		
		return null;
	}
	

	/*
	 * Print debugging messages
	 */
	private static void debugLog(String s) {
		if(debug) {
			System.out.println(s);
		}
	}

	public void enableOptimizer() {
		isOptimizerEnabled = true;		
	}

	public boolean shouldBeWrappedWithVexOptimizerLoop(String mName) {
		if (isOptimizerEnabled && !vexOptimizerSet && mName.equals("main")) {
			vexOptimizerSet = true;
			return true;
		}
		return false;
	}
	
	public void limitProfiling(String string) {
		if (string.equals("none")) {
			profiledMethodNames.add(" ");	// dirty hack			
		} 
//		else if ("limited") {
//			//TODO: profile only if method fills some criteria of size
//		}
		
	}
}

