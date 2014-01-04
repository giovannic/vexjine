package virtualtime;


import java.io.File;

import java.io.FileInputStream;
import java.io.FileOutputStream;

import java.io.BufferedInputStream;

import java.io.IOException;
import java.io.InputStream;
import java.lang.instrument.ClassFileTransformer;
import java.lang.ClassLoader;
import java.lang.instrument.IllegalClassFormatException;
import java.lang.instrument.Instrumentation;
import java.lang.instrument.UnmodifiableClassException;
import java.util.Enumeration;
import java.util.Properties;
import java.util.StringTokenizer;

import org.objectweb.asm.ClassAdapter;
import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassWriter;

import virtualtime.asm.BasicBlockAdapter;
import virtualtime.asm.ExternalWrappingAdapter;
import virtualtime.asm.IoAdapter;
import virtualtime.asm.ExplicitWaitingAdapter;
import virtualtime.asm.VTFAdapter;
import virtualtime.debugging.*;

import virtualtime.generators.*;
import virtualtime.statistics.InstrumentationRecordFactory;

public class ClassTransformer implements ClassFileTransformer {

	private final static boolean DEBUG = false;

	// Various JINE parameters

	// Used for package, method and method adaptation selections
	private static Properties packages = null;

	// Used to output instrumented class files for inspection
	private static boolean outputInstrumentedClasses = false;
	private static String outputDirForInstrumentedClasses = null;
	private static ExecutionPrintoutForMethods executionPrintoutForMethods = null;
	private InstrumentationParameters instrumentationParameters = null;
	private static Instrumentation globalInstrumentation = null;
	private static InstrumentationParameters globalInstrumentationParameters = null;

	private static String profilingHostApproach = "min";

	// Classes that belong to the following packages are automatically considered too low level
	// and are not instrumented by JINE 
	private static String[] excludedPackages = {
		"com/sun", "java/", "javax/", "sun/", "sunw/" , "com/ibm",
		"org/ietf", "org/omg" , "org/w3c", "org/xml", "org/objectweb/asm",
		// Apache harmony excluded for IBM JVM
		"org/apache/harmony/",
		// these are excluded for the DaCapo benchmarks to work
		"org/apache/derby/exe" , 
		"net/sourceforge/pmd/ast/",
		"org/apache/jasper/compiler/"
	};
	
	// Exceptional system classes belong to the excluded packages, but are still instrumented 
	private static String[] exceptionalSystemClasses = {
//		"sun/nio/ch/ServerSocketChannelImpl",
//		"sun/nio/ch/EPollArrayWrapper",
//		"sun/nio/ch/",
		"sun/nio/ch/Net",
		"sun/nio/ch/ServerSocketChannelImpl",
		"sun/nio/ch/EPollArrayWrapper",
		"sun/nio/ch/FileDispatcher",
		"java/util/concurrent/locks/LockSupport"
	};


	
	//tomcat
//	"org/apache/commons",
	// tradebeans
//		"org/apache/axis/transport/http/"
	
	

	/*
	 * Constructor 
	 */
	private ClassTransformer(InstrumentationParameters _instrumentationParameters) {
		instrumentationParameters = _instrumentationParameters;
		if (globalInstrumentationParameters == null) {
			globalInstrumentationParameters = _instrumentationParameters;
		}

//		try {
//			globalInstrumentation.appendToSystemClassLoaderSearch(new JarFile("/homes/nb605/VTF/java/bin/vtf_tests/AdaptiveProfilingTest.class", false));
//		} catch (IOException e) {
//			e.printStackTrace();
//		}
		
		debugLog("ClassTransformer enabled!");
	}

	void printAllProperties() {
		// Enumerate all system properties
		Properties props = System.getProperties();
		Enumeration<?> enumer = props.propertyNames();
		for (; enumer.hasMoreElements(); ) {
		    // Get property name
		    String propName = (String)enumer.nextElement();
	
		    // Get property value
		    String propValue = (String)props.get(propName);
		    System.out.println(propName + " => " + propValue );
		}
	}
	
	/*
	 * Stop instrumenting a method
	 *
	 * methodId:	the methodId of the method to be stopped instrumented
	 * className:	the name of the class that should be loaded
	 * method0:		the methodId of the first method of the class of that method
	 **/
	public static void invalidateMethodProfiling(String fqn, int methodId, int method0) {

		// The control takes place now on the lower-JINE level
		globalInstrumentationParameters.addMethodToBeInvalidated(methodId);
//		System.out.println("Thread " + Thread.currentThread().getName() + " invalidating code of " + methodId + " "+ fqn +" " +method0 + " by " + Thread.currentThread().getName());
		reAdaptMethod(fqn, methodId, method0, false);

	}

	/*
	 * Change the instrumentation code of a recursive method
	 *
	 * methodId:	the methodId of the recursive method
	 * className:	the name of the class that should be loaded
	 * method0:		the methodId of the first method of the class
	 **/
	public static void adaptRecursiveMethod(String fqn, int methodId, int method0) {
		// The control takes place now on the lower-JINE level
		globalInstrumentationParameters.addRecursiveMethodToBeInvalidated(methodId);
		//		System.out.println("Re-instrumenting code of " + methodId + " "+ fqn +" " +method0 + " by " + Thread.currentThread().getName());
		reAdaptMethod(fqn, methodId, method0, true);	

	}

	private static void reAdaptMethod(String fqn, int methodId, int method0, boolean isRecursiveAdaptation) {
		try {
			String[] fqnParts = fqn.split(" ");
			String cname = fqnParts[0].replace('/', '.');

						
			// TODO make this work
//			if (globalInstrumentation.isRetransformClassesSupported()) {
//				Class<?> classOfMethodToBeAdapted = Class.forName(cname);
//				
//				if (globalInstrumentation.isModifiableClass(classOfMethodToBeAdapted)) {
//					synchronized(ClassTransformer.class) {
//						globalInstrumentationParameters.setRetranformationInfo(methodId, isRecursiveAdaptation, method0, fqn);		
//						VTFAdapter.onRetransformationStart(method0);
//						
//						// This fails, because the class path of the classOfMethodToBeAdapted is not found.
//						// Excplicitly appending all jars of the classpath helps, but I cannot make it work in a generic way 
//						// for instance when libraries are loaded via reflection.
//						globalInstrumentation.retransformClasses(classOfMethodToBeAdapted);
//						VTFAdapter.onRetransformationEnd(method0);
//						globalInstrumentationParameters.cleanupRetranformationInfo();
//					}
//				}
//			}
			
			Class<?>[] list = globalInstrumentation.getAllLoadedClasses();
			// Check in loaded classes to reload the class and adapt it 			
			for (int i =0 ; i<list.length; i++) {
				if (list[i] != null) {
					String className = list[i].getName();

					if (className != null) {
						if (className.equals(cname)) {
							if (globalInstrumentation.isRetransformClassesSupported()) {
								synchronized(ClassTransformer.class) {
									globalInstrumentationParameters.setRetranformationInfo(methodId, isRecursiveAdaptation, method0, fqn);
									//globalInstrumentationParameters.printAdaptedMethods();

									// Relying on the fact that only one thread is running.... Low-level vex locks are held for multicore
									// so that only a single thread can re-adapt a method
									VTFAdapter.onRetransformationStart(method0);
									globalInstrumentation.retransformClasses(list[i]);
									VTFAdapter.onRetransformationEnd();	// revert to previous method0
									globalInstrumentationParameters.cleanupRetranformationInfo();
								}
							}
						}

					}
				}
			}

		} catch (ClassCastException ex) {
			ex.printStackTrace(System.err);
		} catch (UnmodifiableClassException e) {
			e.printStackTrace(System.err);
		} catch (Exception e) {
			e.printStackTrace(System.err);
		}

	}


	/*
	 * Parse the arguments passed to the java agent like -javaagent:xxxxx.jar=option1=value1,option2,option3,option4=value4
	 * Set the instrumentation parameters instance to correspond to the options selected
	 * Options:
	 * 
	 * adaptedMethods		:	a file containing the methods that should be adapted together with the corresponding virtual factor
	 * adaptiveProfiling	:	a number defining the samples that should be acquired for each method before profiling is turned off
	 * methodFile			:	a file containing the methods that should be instrumented
	 * noWait				:	do not instrument the Object.wait calls
	 * packages				:	a file containing the packages that should be instrumented
	 * profilingRun			:	do not instrument the synchronized keywords
	 * 
	 * -vtfIOcreate	: 	enable the static instrumentation of the i-th such Java system class (internal)	
	 */
	public static void parseArgs(String options, InstrumentationParameters iparams) throws IllegalArgumentException {

		String[] args = options.split(",");

		int arguments = args.length;
		for (int i=0; i<arguments; ++i) {          
			if (args[i].equals("noWait")) {
				iparams.setTransformWaits(false);

			} else if (args[i].equals("profilingRun")) {
				iparams.setProfilingRun(true);

			} else if (args[i].equals("externalInstrumentation")) {
				iparams.setExternalInstrumentation(true);
				EventNotifier.enableLineLevelStackTrace();
				
			} else if (args[i].equals("trapIPs")) {	
				iparams.setInteractionPointsTrapping(true);

//			} else if (args[i].equals("stackTraceMode")) {
//				iparams.setStackTraceMode(true);
//				EventNotifier.enableStackTraceMode();

			} else if (args[i].equals("exceptionHandlingAll")) {
				iparams.setInstrumentExceptionHandlingEverywhere();

			} else if (args[i].equals("noModifyTimeCalls")) {
				iparams.disableTimeCallsModification();

			} else if (args[i].equals("removeMonitors")) {
				iparams.removeMonitors();
				
			} else if (args[i].equals("limitJvmti")) {
				iparams.limitJvmtiUsage();

			} else if (args[i].equals("enableOptimizer")) {
				iparams.enableOptimizer();

			} else {
				String[] argWithParameter = args[i].split("=");

				if (argWithParameter[0].equals(args[i]) || argWithParameter[1] == null) {
					printHelp();
					throw new IllegalArgumentException("The VEX-Jine argument \""+ args[i] +"\" is either not defined, or requires two arguments");
				}
				
				if (argWithParameter[0].equals("methodFile") &&  argWithParameter[1] != null) {
					iparams.loadMethodFiltering(argWithParameter[1]);
				} else if (argWithParameter[0].equals("printoutMethodsFile") &&  argWithParameter[1] != null) {
					executionPrintoutForMethods = new ExecutionPrintoutForMethods(argWithParameter[1]);
					
				} else if (argWithParameter[0].equals("profileHost") &&  argWithParameter[1] != null) {
					unProfiled = true;
					profilingHostApproach = new String(argWithParameter[1]);
					
				} else if (argWithParameter[0].equals("packages") && argWithParameter[1] != null) {
					packages = loadProfilingPackages(argWithParameter[1]);
					
				} else if (argWithParameter[0].equals("stats") && argWithParameter[1] != null) {
					InstrumentationRecordFactory.setStatistics(argWithParameter[1]);
					
				} else if (argWithParameter[0].equals("adaptiveProfiling") && argWithParameter[1] != null) {

					EventNotifier.registerProfilingInvalidationPoilcy(argWithParameter[1]);
					iparams.setAdaptiveProfiling(true);

				} else if (argWithParameter[0].equals("outputInstrClasses") && argWithParameter[1] != null) {
					outputInstrumentedClasses = true;
					outputDirForInstrumentedClasses = argWithParameter[1];
					
				} else if (argWithParameter[0].equals("limitProfiling") && argWithParameter[1] != null) {
					
					iparams.limitProfiling(argWithParameter[1]);
					
				} else if (argWithParameter[0].equals("adaptedMethods") && argWithParameter[1] != null) {
					iparams.loadMethodAdaptation(argWithParameter[1]);

				} else {
					printHelp();
					System.exit(0);
				}
			}			
		}

		// Link with no_scheduling parameter of VEX
		if (EventNotifier.forcingNoWaitBecauseNotUsingScheduling()) {
			iparams.setTransformWaits(false);
		}
		if (EventNotifier.notRelyingOnJvmtiForMonitorHandling()) {
			iparams.limitJvmtiUsage();
		}		
		
	}


	/*
	 * The first method that is called when the agent is loaded
	 */
	public static void premain(String options, Instrumentation ins) {
		debugLog("ClassTransformer premain starting..." + Thread.currentThread().getName());

		InstrumentationParameters iparams = new InstrumentationParameters();

		if (options != null) {
			parseArgs(options, iparams);
		} 

		// Notify VEX that this is an interpreted mode run (to use different delays)
//		if(jvmInInterpretedMode() && !iparams.isProfilingRun()) {
//			EventNotifier.registerInterpretedMode();
//		}			

		
		EventNotifier.registerExcludedJvmSystemThreads();	
		
	//	VTFAdapter.initialize(new MethodNameHashMethodId());//SimpleMethodId());	// invokes hashCode() that might change JIT compilation order
		VTFAdapter.initialize(new SimpleMethodId());	//
		ins.addTransformer(new ClassTransformer(iparams), true);
		globalInstrumentation = ins;
		
		
//		String classpath = System.getProperty("java.class.path");
//		String[] locations = classpath.split(System.getProperty("path.separator"));
//		for (String s : locations ){ 
//			if (s.endsWith("jar") && !s.endsWith("instrument.jar")) {
//				try {
//					System.out.println("adding jar: " + s);
//					
//					globalInstrumentation.appendToBootstrapClassLoaderSearch(new JarFile(s));
//					globalInstrumentation.appendToSystemClassLoaderSearch(new JarFile(s));
//					
//				} catch (IOException e) {
//					e.printStackTrace();
//				}
//			}
//			System.out.println("classpath: " + s);
//		}
				
	}

	/*
	 * Method to load packages to be instrumented
	 */
	private static Properties loadProfilingPackages(String filename) {
		debugLog("ClassTransformer loadProfilingPackages");

		try {
			InputStream str = new BufferedInputStream(new FileInputStream(filename));
			Properties prop = new Properties();

			debugLog("loading package: " + str);
			prop.load(str);
			return prop;

		} catch (IOException e) {
			e.printStackTrace();
		}
		return null;
	}

	static private boolean unProfiled = false;
	/*
	 * The main transformation method called for every loaded class
	 * The isProfiled flag decides the classes to be instrumented
	 * and the instrumentClass instruments them.
	 */
	public byte[] transform(ClassLoader loader, String className, Class<?> cBR, java.security.ProtectionDomain pD, byte[] classfile) throws IllegalClassFormatException {

		if (unProfiled) {
			synchronized(ClassTransformer.class) {
				if (unProfiled && Thread.currentThread().getName().equals("main")) {
					EventNotifier.profileInstrumentationCode(profilingHostApproach);
					//			EventNotifier.profileThreadYield(jvmInInterpretedMode());
					unProfiled = false;
				}
			}
		}

		//		long instrumentationStart;
		//		try {
		//			instrumentationStart = EventNotifier.getThreadVirtualTime();
		//		} catch (UnsatisfiedLinkError e) {
		//			instrumentationStart = 0;
		//		}

		if (!isInstrumentable(className)) {
			//			EventNotifier.addInstrumentationTime(instrumentationStart); 
			return null; // Don't instrument.className: 
		}

		//	EventNotifier.onInstrumentationStart();
		boolean isProfiled = isProfiled(className);
		byte[] newBytes = null;
		try {
			newBytes = instrumentClass(className, classfile, isProfiled);

		} catch (Throwable exc) {
			System.err.println(exc + " thrown while transforming class " + className + ": " + exc.getMessage());
			exc.printStackTrace(System.out);
		}

		//	EventNotifier.onInstrumentationEnd();
		return newBytes;
	}

	public static boolean isInstrumentable(String className) {
		className = className.replace('.', '/');
		return !(isSystemClass(className) || isInstrumentClass(className));
	}
	
	private static boolean isExceptionalSystemClass(String className) {
		for (String exceptionalSystemClass : exceptionalSystemClasses) {
			if (className.startsWith(exceptionalSystemClass)) {
				return true;
			}
		}
		return false;
	}

	private static boolean classBelongsToExcludedPackage(String className) {
		for (String excludedPackage : excludedPackages) {
			if (className.startsWith(excludedPackage)) {
				return true;
			}
		}
		return false;	
	}

	/**
	 * Check if a class is a system class, based on the package name. *
	 * 
	 * @param className The (fully-qualified) name of the class *
	 * @return True 	if the class is a system class - got these by examining rt.jar from the JRE libraries.
	 */
	public static boolean isSystemClass(String className) {

		if (isExceptionalSystemClass(className)) {
			return false;
		}
		return classBelongsToExcludedPackage(className);
	}
	



	/**
	 * Methods to check if a class should be instrumented
	 * 
	 * @param className The (fully-qualified) name of the class *
	 * @return True if the class is an instrumentation class.
	 */
	public static boolean isInstrumentClass(String className) {
		return className.equals("Transformer") || className.startsWith("org/apache/bcel/")
		|| (className.startsWith("virtualtime/"));
	}

	public boolean isProfiled(String className) {
		if (packages == null) {
			return true;
		}

		StringTokenizer tokens = new StringTokenizer(className, "/");

		String packge = tokens.nextToken();
		while (tokens.hasMoreTokens()) {
			if (packages.containsKey(packge)) {
				System.out.println("instrumenting " + className);
				return true;
			}
			packge = packge + "/" + tokens.nextToken();
		}

		return false;
	}


	/**
	 * * Instruments a given class file; * this method's interface is suitable
	 * for use with java.lang.instrument *
	 * 
	 * @param className
	 *            The .class file name *
	 * @param jc
	 *            The JavaClass object corresponding to this class *
	 * @return An array of bytes corresponding to the new class definition, * or
	 *         null if there are actually no changes.
	 */
	public byte[] instrumentClass(String className, byte[] classFile, boolean profile) {

		if (classFile != null) {
			debugLog("Instrumenting class " +className);

			ClassReader reader = new ClassReader(classFile);
			ClassWriter writer = new ClassWriter(ClassWriter.COMPUTE_MAXS);
			ClassAdapter adapt = null;

			if (isExceptionalSystemClass(className)) {
				if (className.equals("sun/nio/ch/FileDispatcher")) {
					adapt = new IoAdapter(writer);
				} else if (instrumentationParameters.shouldTransformWaits()){
					adapt = new ExplicitWaitingAdapter(writer);
				} else {
					return classFile;
				}
			} else {
				if (instrumentationParameters.usingExternalInstrumentation()) {
					adapt = new ExternalWrappingAdapter(writer, profile, instrumentationParameters);
				} else {
					adapt = new BasicBlockAdapter(writer, profile, instrumentationParameters);
				}
			}

			reader.accept(adapt, 0);

					
			if (executionPrintoutForMethods != null && executionPrintoutForMethods.instrumentsClass(className)) {
				ClassReader executionPrintoutsReader = new ClassReader(writer.toByteArray());
				ClassWriter executionPrintoutsWriter = new ClassWriter(ClassWriter.COMPUTE_MAXS);
				ClassAdapter executionPrintoutsAdapt = new ExecutionPrintoutAdapter(executionPrintoutsWriter, className, executionPrintoutForMethods.getMethodsOfClass(className));
				
				executionPrintoutsReader.accept(executionPrintoutsAdapt, 0);
				
				if (outputInstrumentedClasses) {
					outputInstrumentedClass(outputDirForInstrumentedClasses, className, executionPrintoutsWriter.toByteArray());
				}

				return executionPrintoutsWriter.toByteArray();
			} else {
				
				if (outputInstrumentedClasses) {
					outputInstrumentedClass(outputDirForInstrumentedClasses, className, writer.toByteArray());
				}
				return writer.toByteArray();	
			}

			
		} else {
			return null;
		}

	}



	/*
	 * Method to output the instrumented classes into bytecode files
	 */
	private static void outputInstrumentedClass(String outputDirectory, String className, byte[] newBytecode) {
		try {
			int delimiterPos = className.lastIndexOf("/");

			if (delimiterPos != -1) {
				String dirsname = outputDirectory + "/" + className.substring(0, delimiterPos);
				new File(dirsname).mkdirs();
			} 

			FileOutputStream fos = new FileOutputStream(outputDirectory + "/" + className + ".class");
			fos.write(newBytecode);
			fos.close();
		} catch (IOException ei) {
			System.out.println(ei.getMessage());
		}
	}


	/*
	 * Print debugging messages
	 */
	private static void debugLog(String s) {
		if(DEBUG) {
			System.out.println(s);
		}
	}

	/*
	 * Find out if the JVM is run in interpreted mode only to adjust the times that will
	 * be decreased as framework overhead
	 */
	private static boolean jvmInInterpretedMode() {
		return System.getProperty("java.vm.info").equals("interpreted mode") || System.getProperty("java.vm.info").contains("JIT disabled");
	}

	/*
	 * Printing the options of JINE
	 */
	private static void printHelp() {
		System.out.println("\nJINE options:\n" +
				"- adaptedMethods=<file>         a file containing the methods that should be adapted together (time scaling factor/describing performance model)\n" +
				"- adaptiveProfiling<value>      set invalidation policy. value can be \"=samples\" (/method), \":int absolute ms\" (/method), \":float relative time\" (/method)\n" +
				"- exceptionHandlingAll          all methods will be wrapped inside exception handlers\n" +
				"- externalInstrumentation       put profiling instruments externally of each profiled method to avoid changes in JIT-compiler\n" +
				"- enableOptimizer               enable the method automatic optimization mode\n" + 
				"- help                          print this menu and exit\n" +
				"- limitJvmti                    do not use JVMTI to trap MonitorWait(ed) and MonitorContendedEnter(ed) events - explicitly instrument the calls\n" +
				"- limitProfiling=<value>        limit the adding of profiling instruments. Value=\"none\"\n" +
				"- lineLevelStackTrace           I/O invocation points differentiate according to the calling methods lines\n" + 
				"- methodFile=<file>             a file containing the methods that should be instrumented\n" +
				"- noModifyTimeCalls             do not modify calls System.nanoTime() and System.currentTimeMillis() to return simulation timestamps\n" +
				"- noWait                        do not instrument Object.wait calls\n" +
				"- outputInstrClasses=<dir>      output the instrumented classes bytecode under the specified directory\n" + 
				"- package=<file>                a file containing the packages that should be instrumented\n" +
				"- printoutMethodsFile=<file>    a file containing classes and methods that should print out a message when executed (for dbg reasons)\n" +
				"- profileHost=option            profile the instrumentation delays on this host and store them to .vex_delays or file of \"delays_file\" VEX parameter\n" +
				"                                option can be min|avg|median\n" +	
				"- profilingRun                  do not instrument the synchronized keywords\n" +
				"- removeMonitors                (only effective with limitJmvti): replace monitors by low-level locks\n" +
				"- stats=<value>                 output instrumentation statistics to /data/jine_instrumentation_statistics.csv. Value=global|class|method\n" +
				"- trapIPs                       trap Interaction Points\n");
//		"- stackTraceMode                group results by stack traces (not just method names)\n" +
	}


	/*
	 * Used to instrument classes and store them into an output class for inspection
	 */
	public static void main(String[] argv) {
		debugLog("Explicit class instrumentation with JINE");
		if (argv.length == 2 && argv[0].endsWith(".class")) {
			try {

				FileInputStream fis = new FileInputStream(argv[0]);
				ClassReader reader = new ClassReader(fis);	

				ClassWriter writer = new ClassWriter(ClassWriter.COMPUTE_MAXS);
				BasicBlockAdapter adapt = new BasicBlockAdapter(writer, true, new InstrumentationParameters());
				reader.accept(adapt, 0);

				ClassReader reader2 = new ClassReader(writer.toByteArray());	
				ClassWriter writer2 = new ClassWriter(ClassWriter.COMPUTE_MAXS);
				reader2.accept(writer2, 0);

				FileOutputStream fos = new FileOutputStream(argv[1]);
				fos.write(writer2.toByteArray());
				fos.close();

			} catch (IOException ex) {
				ex.printStackTrace(System.err);
			}

		} else {
			System.out.println("Usage: ClassTransformer class-file output");
		}
	}
}
