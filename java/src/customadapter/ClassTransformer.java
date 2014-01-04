package customadapter;


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
import java.util.Properties;
import java.util.StringTokenizer;

import org.objectweb.asm.ClassAdapter;
import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassWriter;

public class ClassTransformer implements ClassFileTransformer {

	private final static boolean DEBUG = false;

	// Various JINE parameters
	private static boolean creatingVtfIoFiles = false;
	
	// Used for package, method and method adaptation selections
	private static Properties packages = null;
	
	// Used for static class instrumentation prior to JINE execution
	private static int ioClassToStaticallyInstrument = -1;
	
	// Used to output instrumented class files for inspection
	private static boolean outputInstrumentedClasses = false;
	private static String outputDirForInstrumentedClasses = null;
		
	private InstrumentationParameters instrumentationParameters = null;
	private static Instrumentation globalInstrumentation = null;
	private static InstrumentationParameters globalInstrumentationParameters = null;

	private static boolean unProfiled = true;
	/*
	 * Constructor 
	 */
	private ClassTransformer(InstrumentationParameters _instrumentationParameters) {
		instrumentationParameters = _instrumentationParameters;
		if (globalInstrumentationParameters == null) {
			globalInstrumentationParameters = _instrumentationParameters;
		}
		debugLog("ClassTransformer enabled!");
	}

	
	/*
	 * Stop instrumenting a method
	 *
	 * methodId:	the methodId of the method to be stopped instrumented
	 * className:	the name of the class that should be loaded
	 * method0:		the methodId of the first method of the class of that method
	 **/
	public static void invalidateMethodProfiling(String fqn, int methodId, int method0) {
		debugLog("Invalidating code of " + methodId + " "+ fqn +" " +method0 + " by " + Thread.currentThread().getName());
		globalInstrumentationParameters.addInvalidatedMethodId(methodId);
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
		debugLog("Re-instrumenting code of " + methodId + " "+ fqn +" " +method0 + " by " + Thread.currentThread().getName());
		globalInstrumentationParameters.addRecursiveMethodId(methodId);
		reAdaptMethod(fqn, methodId, method0, true);	
	}
	
	private static void reAdaptMethod(String fqn, int methodId, int method0, boolean isRecursiveAdaptation) {
		try {
			String[] fqnParts = fqn.split(" ");
			String cname = fqnParts[0].replace('/', '.');
	
			// TODO make this work
	//		globalInstrumentationParameters.setRetranformationInfo(methodId, true, method0, fqn);
	//		VTFAdapter.onRetransformationStart(method0);
	//		globalInstrumentation.retransformClasses(Class.forName(cname));
	//		VTFAdapter.onRetransformationEnd(method0);
	//		globalInstrumentationParameters.cleanupRetranformationInfo();

			//className.indexOf(" ")
			Class<?>[] list = globalInstrumentation.getAllLoadedClasses();
	
			String className;
			
			// Check in loaded classes to reload the class and adapt it 			
			for (int i =0 ; i<list.length; i++) {
				if (list[i] != null) {
					className = list[i].getName();
					
					if (className != null) {
						if (className.equals(cname)) {
							if (globalInstrumentation.isRetransformClassesSupported()) {
								synchronized(ClassTransformer.class) {
									globalInstrumentationParameters.setRetranformationInfo(methodId, isRecursiveAdaptation, method0, fqn);
//									globalInstrumentationParameters.printAdaptedMethods();
//									VTFAdapter.onRetransformationStart(method0);
									globalInstrumentation.retransformClasses(list[i]);
//									VTFAdapter.onRetransformationEnd(method0);
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
	public static void parseArgs(String options, InstrumentationParameters iparams) {
		
		if (options.startsWith("-vtfIOcreate")) {
			options = options.substring(12);	//strlen "-vtfIOcreate"
			ioClassToStaticallyInstrument=Integer.parseInt(options);
			creatingVtfIoFiles = true;
			
		} else {

			String[] args = options.split(",");
			
			int arguments = args.length;
			for (int i=0; i<arguments; ++i) {          
				if (args[i].equals("noWait")) {
					iparams.setTransformWaits(false);
					
				} else if (args[i].equals("profilingRun")) {
					iparams.setProfilingRun(true);
				} else if (args[i].equals("noProfiling")) {
					unProfiled = false;
					
				} else if (args[i].equals("externalInstrumentation")) {
					iparams.setExternalInstrumentation(true);

				} else if (args[i].equals("trapIPs")) {	
					iparams.setInteractionPointsTrapping(true);
					
//				} else if (args[i].equals("stackTraceMode")) {
//					iparams.setStackTraceMode(true);
					
				} else if (args[i].equals("exceptionHandlingAll")) {
					iparams.setInstrumentExceptionHandlingEverywhere();
				
				} else if (args[i].equals("modifyTimeCalls")) {
					iparams.enableTimeCallsModification();
					
				} else {
					String[] argWithParameter = args[i].split("=");

					if (argWithParameter[0].equals("methodFile") &&  argWithParameter[1] != null) {
						iparams.loadMethodFiltering(argWithParameter[1]);
						
					} else if (argWithParameter[0].equals("packages") && argWithParameter[1] != null) {
						packages = loadProfilingPackages(argWithParameter[1]);
						
					} else if (argWithParameter[0].equals("adaptiveProfiling") && argWithParameter[1] != null) {
						iparams.setAdaptiveProfiling(true);
					
					} else if (argWithParameter[0].equals("outputInstrClasses") && argWithParameter[1] != null) {
						outputInstrumentedClasses = true;
						outputDirForInstrumentedClasses = argWithParameter[1];
						
					} else if (argWithParameter[0].equals("adaptedMethods") && argWithParameter[1] != null) {
						iparams.loadMethodAdaptation(argWithParameter[1]);
							
					} else {
						printHelp();
						System.exit(0);
					}
				}			
			}
		}		
		
	}
	
	
	/*
	 * The first method that is called when the agent is loaded
	 */
	public static void premain(String options, Instrumentation ins) {
		debugLog("ClassTransformer premain starting...");
		
		InstrumentationParameters iparams = new InstrumentationParameters();
				
		if (options != null) {
			parseArgs(options, iparams);
		} 
		
				
		// Default case
		if (ioClassToStaticallyInstrument == -1) {
			ins.addTransformer(new ClassTransformer(iparams), true);
			globalInstrumentation = ins;
		} else {
			staticClassInstrumentation(ins, new ClassTransformer(iparams));
		}
	}
	
	/*
	 * Method used to retransform system classes and store them into modified bytecode files
	 * Static instrumentation prior to actual simulation.
	 */
	@SuppressWarnings("unchecked")
	private static void staticClassInstrumentation(Instrumentation ins, ClassTransformer vtfClassTransformer) {
		// loaded IO library creation	

		// re-transformation capable - used to create VTF IO classes 
		ins.addTransformer(vtfClassTransformer, true);   

		Class[] list = ins.getAllLoadedClasses();

		String className;
		String[] allIOclasses = {
				"java.lang.Thread",  
				"java.net.ServerSocket", "java.net.Socket", "java.net.SocketInputStream", "java.net.SocketOutputStream", 
				"java.io.FileOutputStream", "java.io.FileInputStream", "java.io.RandomAccessFile", "java.io.UnixFileSystem"
		};

		try {
			ClassLoader.getSystemClassLoader().loadClass(allIOclasses[ioClassToStaticallyInstrument]);
		} catch (Exception io) {
			System.out.println(io.getMessage());
		}

		
		// Check in loaded classes to reload the system class and transform it 
		try {
			for (int i =0 ; i<list.length; i++) {
				if (list[i] != null) {
					className = list[i].getCanonicalName();
					if (className != null) {
						if (ioClassToStaticallyInstrument != -1) {
							if (className.equals(allIOclasses[ioClassToStaticallyInstrument])) {
								if (ins.isRetransformClassesSupported()) {
									ins.retransformClasses(list[i]);
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
		}
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



	
	/*
	 * The main transformation method called for every loaded class
	 * The isProfiled flag decides the classes to be instrumented
	 * and the instrumentClass instruments them.
	 */
	public byte[] transform(ClassLoader loader, String className, Class<?> cBR, java.security.ProtectionDomain pD, byte[] classfile) throws IllegalClassFormatException {
		
		synchronized(ClassTransformer.class) {
			if (unProfiled && ioClassToStaticallyInstrument == -1) {
				unProfiled = false;	
			}
		}
		
		if (!isInstrumentable(className)) {
			return null; // Don't instrument.className: 
		}

		boolean isProfiled = isProfiled(className);
		byte[] newBytes = null;
		try {
			newBytes = instrumentClass(className, classfile, isProfiled);

		} catch (Throwable exc) {
			System.err.println(exc + " thrown while transforming class " + className + ": " + exc.getMessage());
			exc.printStackTrace(System.out);
		}

		
		if (className.startsWith("java/")) {
			System.out.print("Statically instrumenting:" + className);
			new File("java").mkdirs();		// all statically instrumented
			String cn = className.replace(".","/");
			outputInstrumentedClass("./", cn, newBytes);

			debugLog("New bytecode of " + newBytes.length + " bytes output into ./" + cn + ".class");
			System.out.println(" exiting..............");
			System.exit(0);
		}

		return newBytes;
	}

	/**
	 * Check if a class is a system class, based on the package name. *
	 * 
	 * @param className The (fully-qualified) name of the class *
	 * @return True 	if the class is a system class - got these by examining rt.jar from the JRE libraries.
	 */
	public static boolean isSystemClass(String className) {

		// We need to include the java/io package for the vtfIO.jar creation
		if (creatingVtfIoFiles && (className.startsWith("java/io/")	
				||	className.equals("java/lang/Thread")
				||	className.equals("java/net/Socket")
				||	className.equals("java/net/ServerSocket")
				||	className.equals("java/net/SocketInputStream") 
				||  className.equals("java/net/SocketOutputStream"))) {  
			return false;

		// Dynamic allocation of socket i/o classes	
//		} else if (className.equals("java/net/SocketInputStream") ||  className.equals("java/net/SocketOutputStream")) {
//			return false;
		}
		
		return className.startsWith("com/sun") || className.startsWith("java/")
		|| className.startsWith("javax/") || className.startsWith("org/ietf")
		|| className.startsWith("org/omg") || className.startsWith("org/w3c")
		|| className.startsWith("org/xml") || className.startsWith("sun/")
		|| className.startsWith("sunw/") || className.startsWith("com/ibm");
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
				return true;
			}
			packge = packge + "/" + tokens.nextToken();
		}

		return false;
	}

	public static boolean isInstrumentable(String className) {
		className = className.replace('.', '/');
		return !(isSystemClass(className) || isInstrumentClass(className));
	}

	private static boolean isIoClass(String cname) {
		if (cname.equals("java/io/FileInputStream") || cname.equals("java/io/FileOutputStream") || cname.equals("java/io/RandomAccessFile")  || cname.equals("java/io/UnixFileSystem")
			|| cname.equals("java/net/SocketInputStream") || cname.equals("java/net/SocketOutputStream") ) {
			return true;
		} else {
			return false;
		}
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
	
//			if (className.startsWith("java") && creatingVtfIoFiles) {
//				if (className.equals("java/lang/Thread")) {
//					adapt = new ThreadAdapter(writer);
//				} else if (isIoClass(className)) {
//					adapt = new IoAdapter(writer);
//				} else if (className.equals("java/net/Socket") || className.equals("java/net/ServerSocket")) {
//					adapt = new SocketAdapter(writer);
//				}
//			} else {
//				if (profile) {
	
				adapt = new CustomClassAdapter(writer, instrumentationParameters);
					
//				}
//			}
		
			reader.accept(adapt, 0);

			if (outputInstrumentedClasses) {
				outputInstrumentedClass(outputDirForInstrumentedClasses, className, writer.toByteArray());
			}
			
			return writer.toByteArray();
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
	 * Printing the options of JINE
	 */
	private static void printHelp() {
		System.out.println("\nJINE options:\n" +
				"- adaptedMethods=<filename>     a file containing the methods that should be adapted together with the corresponding virtual factor\n" +
				"- adaptiveProfiling=<number>    a number defining the samples that should be acquired for each method before profiling is turned off\n" +
				"- exceptionHandlingAll          all methods will be wrapped inside exception handlers\n" +
				"- externalInstrumentation       put profiling instruments externally of each profiled method to avoid changes in JIT-compiler\n" +				
				"- lineLevelStackTrace           I/O invocation points differentiate according to the calling methods lines\n" +
				"- help                          print this menu and exit\n" + 
				"- methodFile=<filename>         a file containing the methods that should be instrumented\n" +
				"- noWait                        do not instrument Object.wait calls\n" +
				"- outputInstrClasses=<dir>      output the instrumented classes bytecode under the specified directory\n" + 
				"- package=<filename>            a file containing the packages that should be instrumented\n" +
				"- profilingRun                  do not instrument the synchronized keywords\n" +
				"- stackTraceMode                group results by stack traces (not just method names)\n" +
				"- trapIPs                       trap Interaction Points");
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
				ClassAdapter adapt = new CustomClassAdapter(writer, new InstrumentationParameters());
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
