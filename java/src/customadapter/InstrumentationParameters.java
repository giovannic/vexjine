package customadapter;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.Vector;

/*
 * Class used to encapsulate the parameters of JINE instrumentation
 */
public class InstrumentationParameters {
	private static final boolean debug = false;
	
	private boolean transformWaits;
	private boolean profilingRun;
	private boolean adaptiveProfiling;
	private boolean externalInstrumentation; 	// a method is wrapped externally with the profiling instruments (so that JIT is not affected) 
	private boolean trapInteractionPoints; 
	
//	private boolean stackTraceMode;
	
	private Vector<String> profiledMethodNames;
	private Vector<MethodAdaptationInfo> adaptedMethodNames;
	
	private Vector<Integer> invalidatedMethodIds;
	private Vector<Integer> recursiveMethodIds;
	
	private int classVersion;
	private int method0;
	
	private int retransformingMethodId;
	private boolean recursiveRetransformation;
	private boolean instrumentExceptionHandlingCodeEverywhere;
	private boolean modifyTimeCalls;
	
	private String retransformingMethodName;
	private String retransformingMethodDesc;
	
	// Default options
	public InstrumentationParameters() {
		transformWaits		 	= true;
		profilingRun 			= false;
		adaptiveProfiling 		= false;
		externalInstrumentation = false;
		trapInteractionPoints 	= false;
//		stackTraceMode 			= false;
		
		adaptedMethodNames  = new Vector<MethodAdaptationInfo>();
		profiledMethodNames = new Vector<String>();
	
		invalidatedMethodIds = new Vector<Integer>(); 
		recursiveMethodIds   = new Vector<Integer>(); 
		
		classVersion = 1;
		method0 = 0;
		
		modifyTimeCalls = false;	
		recursiveRetransformation = false;
		retransformingMethodId = 0;
		
		instrumentExceptionHandlingCodeEverywhere = false;
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
	
	
	public void enableTimeCallsModification() {
		modifyTimeCalls = true;
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
		return methodName.equals("main") || (!(methodName.startsWith("access$")) && (profiledMethodNames.size() == 0 || profiledMethodNames.contains(className+methodName)));
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
					adaptedMethodNames.add(new MethodAdaptationInfo(className + methodName, Double.parseDouble(factorValue)));

				} catch (Exception se) {

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
	
	public void addInvalidatedMethodId(int methodId) {
		invalidatedMethodIds.add(methodId);
	}
	
	public void addRecursiveMethodId(int methodId) {
		recursiveMethodIds.add(methodId);
	}
	 
	public boolean isInvalidatedMethod(int methodId) {
		return invalidatedMethodIds.contains(methodId);
	}
	
	public boolean isRecursiveMethod(int methodId) {
		return recursiveMethodIds.contains(methodId);
	}
	
	public boolean methodNotBeingAdapted(int methodId) {
		return (!isInvalidatedMethod(methodId) && !isRecursiveMethod(methodId));
	}
	
	public boolean exceptionHandlingEverywhere() {
		return instrumentExceptionHandlingCodeEverywhere;
	}
	
	public void printAdaptedMethods() {
		System.out.print("Invalidated methods: ");
		for (int i =0; i<invalidatedMethodIds.size(); ++i) {
			System.out.print(invalidatedMethodIds.elementAt(i) + " ");	
		}
		System.out.println();
		
		System.out.print("Recursive methods: ");
		for (int i =0; i<recursiveMethodIds.size(); ++i) {
			System.out.print(recursiveMethodIds.elementAt(i) + " ");
		}
		System.out.println();
	}
	
	/*
	 * Utility vector parsing function
	 */
	public double getFactorOf(String fullname) {
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
	
	/*
	 * Print debugging messages
	 */
	private static void debugLog(String s) {
		if(debug) {
			System.out.println(s);
		}
	}
}
