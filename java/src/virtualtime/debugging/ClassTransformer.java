package virtualtime.debugging;

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
import java.security.ProtectionDomain;
import java.util.Properties;
import java.util.StringTokenizer;

import org.objectweb.asm.ClassAdapter;
import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassWriter;

import virtualtime.asm.BasicBlockAdapter;
import virtualtime.asm.ExternalWrappingAdapter;
import virtualtime.asm.IoAdapter;
import virtualtime.asm.ExplicitWaitingAdapter;
import virtualtime.asm.ThreadAdapter;
import virtualtime.asm.VTFAdapter;
import virtualtime.debugging.*;

import virtualtime.generators.*;

public class ClassTransformer implements ClassFileTransformer {

	private static ExecutionPrintoutForMethods executionPrintoutForMethods = null;

	private static String[] excludedPackages = {
		"com/sun", "java/", "javax/", "sun/", "sunw/" , "com/ibm",
	};
	
	

	/*
	 * Constructor 
	 */
	private ClassTransformer() {
//		debugLog("ClassTransformer enabled!");
	}



	@Override
	public byte[] transform(ClassLoader loader, String className,
			Class<?> classBeingRedefined, ProtectionDomain protectionDomain,
			byte[] classfileBuffer) throws IllegalClassFormatException {
		// TODO Auto-generated method stub
		return null;
	}



//	/*
//	 * Parse the arguments passed to the java agent like -javaagent:xxxxx.jar=option1=value1,option2,option3,option4=value4
//	 * Set the instrumentation parameters instance to correspond to the options selected
//	 * Options:
//	 * 
//	 * adaptedMethods		:	a file containing the methods that should be adapted together with the corresponding virtual factor
//	 * adaptiveProfiling	:	a number defining the samples that should be acquired for each method before profiling is turned off
//	 * methodFile			:	a file containing the methods that should be instrumented
//	 * noWait				:	do not instrument the Object.wait calls
//	 * packages				:	a file containing the packages that should be instrumented
//	 * profilingRun			:	do not instrument the synchronized keywords
//	 * 
//	 * -vtfIOcreate	: 	enable the static instrumentation of the i-th such Java system class (internal)	
//	 */
//	public static void parseArgs(String options) throws IllegalArgumentException {
//		executionPrintoutForMethods = new ExecutionPrintoutForMethods(options);
//
//	}
//
//
//	/*
//	 * The first method that is called when the agent is loaded
//	 */
//	public static void premain(String options, Instrumentation ins) {
//		debugLog("ClassTransformer premain starting..." + Thread.currentThread().getName());
//
//		if (options != null) {
//			parseArgs(options);
//		} 
//
//		ins.addTransformer(new ClassTransformer(), true);
//
//	/*
//	 * The main transformation method called for every loaded class
//	 * The isProfiled flag decides the classes to be instrumented
//	 * and the instrumentClass instruments them.
//	 */
//	public byte[] transform(ClassLoader loader, String className, Class<?> cBR, java.security.ProtectionDomain pD, byte[] classfile) throws IllegalClassFormatException {
//
//		if (!isInstrumentable(className)) {
//			//			EventNotifier.addInstrumentationTime(instrumentationStart); 
//			return null; // Don't instrument.className: 
//		}
//
//		//	EventNotifier.onInstrumentationStart();
//		boolean isProfiled = isProfiled(className);
//		byte[] newBytes = null;
//		try {
//			newBytes = instrumentClass(className, classfile, isProfiled);
//
//		} catch (Throwable exc) {
//			System.err.println(exc + " thrown while transforming class " + className + ": " + exc.getMessage());
//			exc.printStackTrace(System.out);
//		}
//
//		//	EventNotifier.onInstrumentationEnd();
//		return newBytes;
//	}
//
//	public static boolean isInstrumentable(String className) {
//		className = className.replace('.', '/');
//		return !(isSystemClass(className) || isInstrumentClass(className));
//	}
//	
//	private static boolean isExceptionalSystemClass(String className) {
//		for (String exceptionalSystemClass : exceptionalSystemClasses) {
//			if (exceptionalSystemClass.equals(className)) {
//				return true;
//			}
//		}
//		return false;
//	}
//
//	private static boolean classBelongsToExcludedPackage(String className) {
//		for (String excludedPackage : excludedPackages) {
//			if (className.startsWith(excludedPackage)) {
//				return true;
//			}
//		}
//		return false;	
//	}
//
//	/**
//	 * Check if a class is a system class, based on the package name. *
//	 * 
//	 * @param className The (fully-qualified) name of the class *
//	 * @return True 	if the class is a system class - got these by examining rt.jar from the JRE libraries.
//	 */
//	public static boolean isSystemClass(String className) {
//
//		if (isExceptionalSystemClass(className)) {
//			return false;
//		}
//		return classBelongsToExcludedPackage(className);
//	}
//	
//
//
//
//	/**
//	 * Methods to check if a class should be instrumented
//	 * 
//	 * @param className The (fully-qualified) name of the class *
//	 * @return True if the class is an instrumentation class.
//	 */
//	public static boolean isInstrumentClass(String className) {
//		return className.equals("Transformer") || className.startsWith("org/apache/bcel/")
//		|| (className.startsWith("virtualtime/"));
//	}
//
//	public boolean isProfiled(String className) {
//		if (packages == null) {
//			return true;
//		}
//
//		StringTokenizer tokens = new StringTokenizer(className, "/");
//
//		String packge = tokens.nextToken();
//		while (tokens.hasMoreTokens()) {
//			if (packages.containsKey(packge)) {
//				return true;
//			}
//			packge = packge + "/" + tokens.nextToken();
//		}
//
//		return false;
//	}
//
//
//	/**
//	 * * Instruments a given class file; * this method's interface is suitable
//	 * for use with java.lang.instrument *
//	 * 
//	 * @param className
//	 *            The .class file name *
//	 * @param jc
//	 *            The JavaClass object corresponding to this class *
//	 * @return An array of bytes corresponding to the new class definition, * or
//	 *         null if there are actually no changes.
//	 */
//	public byte[] instrumentClass(String className, byte[] classFile, boolean profile) {
//
//		if (classFile != null) {
//			debugLog("Instrumenting class " +className);
//
//			ClassReader reader = new ClassReader(classFile);
//			ClassWriter writer = new ClassWriter(ClassWriter.COMPUTE_MAXS);
//			ClassAdapter adapt = null;
//
//			if (isExceptionalSystemClass(className)) {
//				adapt = new ExplicitWaitingAdapter(writer);
//			} else {
//				//				if (profile) {
//				if (instrumentationParameters.usingExternalInstrumentation()) {
//					adapt = new ExternalWrappingAdapter(writer, profile, instrumentationParameters);
//				} else {
//					adapt = new BasicBlockAdapter(writer, profile, instrumentationParameters);
//				}
//				//				}
//			}
//
//			reader.accept(adapt, 0);
//
//					
//			if (executionPrintoutForMethods != null && executionPrintoutForMethods.instrumentsClass(className)) {
//				ClassReader executionPrintoutsReader = new ClassReader(writer.toByteArray());
//				ClassWriter executionPrintoutsWriter = new ClassWriter(ClassWriter.COMPUTE_MAXS);
//				ClassAdapter executionPrintoutsAdapt = new ExecutionPrintoutAdapter(executionPrintoutsWriter, className, executionPrintoutForMethods.getMethodsOfClass(className));
//				
//				executionPrintoutsReader.accept(executionPrintoutsAdapt, 0);
//				
//				if (outputInstrumentedClasses) {
//					outputInstrumentedClass(outputDirForInstrumentedClasses, className, executionPrintoutsWriter.toByteArray());
//				}
//
//				return executionPrintoutsWriter.toByteArray();
//			} else {
//				
//				if (outputInstrumentedClasses) {
//					outputInstrumentedClass(outputDirForInstrumentedClasses, className, writer.toByteArray());
//				}
//				return writer.toByteArray();	
//			}
//
//			
//		} else {
//			return null;
//		}
//
//	}
//
//
//
//	/*
//	 * Method to output the instrumented classes into bytecode files
//	 */
//	private static void outputInstrumentedClass(String outputDirectory, String className, byte[] newBytecode) {
//		try {
//			int delimiterPos = className.lastIndexOf("/");
//
//			if (delimiterPos != -1) {
//				String dirsname = outputDirectory + "/" + className.substring(0, delimiterPos);
//				new File(dirsname).mkdirs();
//			} 
//
//			FileOutputStream fos = new FileOutputStream(outputDirectory + "/" + className + ".class");
//			fos.write(newBytecode);
//			fos.close();
//		} catch (IOException ei) {
//			System.out.println(ei.getMessage());
//		}
//	}
//
//
//	/*
//	 * Print debugging messages
//	 */
//	private static void debugLog(String s) {
//		if(DEBUG) {
//			System.out.println(s);
//		}
//	}
//
//	/*
//	 * Find out if the JVM is run in interpreted mode only to adjust the times that will
//	 * be decreased as framework overhead
//	 */
//	private static boolean jvmInInterpretedMode() {
//		return System.getProperty("java.vm.info").equals("interpreted mode") || System.getProperty("java.vm.info").contains("JIT disabled");
//	}
//
//	/*
//	 * Printing the options of JINE
//	 */
//	private static void printHelp() {
//		System.out.println("\nJINE options:\n" +
//				"- adaptedMethods=<filename>     a file containing the methods that should be adapted together (time scaling factor/describing performance model)\n" +
//				"- adaptiveProfiling=<number>    a number defining the samples that should be acquired for each method before profiling is turned off\n" +
//				"- exceptionHandlingAll          all methods will be wrapped inside exception handlers\n" +
//				"- externalInstrumentation       put profiling instruments externally of each profiled method to avoid changes in JIT-compiler\n" +
//				"- enableOptimizer               enable the method automatic optimization mode\n" + 
//				"- help                          print this menu and exit\n" +
//				"- limitJvmti                    do not use JVMTI to trap MonitorWait(ed) and MonitorContendedEnter(ed) events - explicitly instrument the calls\n" + 
//				"- lineLevelStackTrace           I/O invocation points differentiate according to the calling methods lines\n" + 
//				"- methodFile=<filename>         a file containing the methods that should be instrumented\n" +
//				"- noModifyTimeCalls             do not modify calls System.nanoTime() and System.currentTimeMillis() to return simulation timestamps\n" +
//				"- noWait                        do not instrument Object.wait calls\n" +
//				"- outputInstrClasses=<dir>      output the instrumented classes bytecode under the specified directory\n" + 
//				"- package=<filename>            a file containing the packages that should be instrumented\n" +
//				"- printoutMethodsFile=<filenm>  a file containing classes and methods that should print out a message when executed (for dbg reasons)\n" +
//				"- profileHost=option            profile the instrumentation delays on this host and store them to .vex_delays or file of \"delays_file\" VEX parameter\n" +
//				"                                option can be min|avg|median\n" +	
//				"- profilingRun                  do not instrument the synchronized keywords\n" +
//				"- removeMonitors                (only effective with limitJmvti): replace monitors by low-level locks\n" +  
//				"- stackTraceMode                group results by stack traces (not just method names)\n" +
//				"- trapIPs                       trap Interaction Points\n");
//	}
//
//
//	/*
//	 * Used to instrument classes and store them into an output class for inspection
//	 */
//	public static void main(String[] argv) {
//		debugLog("Explicit class instrumentation with JINE");
//		if (argv.length == 2 && argv[0].endsWith(".class")) {
//			try {
//
//				FileInputStream fis = new FileInputStream(argv[0]);
//				ClassReader reader = new ClassReader(fis);	
//
//				ClassWriter writer = new ClassWriter(ClassWriter.COMPUTE_MAXS);
//				BasicBlockAdapter adapt = new BasicBlockAdapter(writer, true, new InstrumentationParameters());
//				reader.accept(adapt, 0);
//
//				ClassReader reader2 = new ClassReader(writer.toByteArray());	
//				ClassWriter writer2 = new ClassWriter(ClassWriter.COMPUTE_MAXS);
//				reader2.accept(writer2, 0);
//
//				FileOutputStream fos = new FileOutputStream(argv[1]);
//				fos.write(writer2.toByteArray());
//				fos.close();
//
//			} catch (IOException ex) {
//				ex.printStackTrace(System.err);
//			}
//
//		} else {
//			System.out.println("Usage: ClassTransformer class-file output");
//		}
//	}



}
