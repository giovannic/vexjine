package staticinstrument;


import java.io.File;
import java.io.FileOutputStream;

import java.io.IOException;
import java.lang.instrument.ClassFileTransformer;
import java.lang.ClassLoader;
import java.lang.instrument.IllegalClassFormatException;
import java.lang.instrument.Instrumentation;
import java.lang.instrument.UnmodifiableClassException;
import java.util.Properties;

import org.objectweb.asm.ClassAdapter;
import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassWriter;


import staticinstrument.asm.*;

public class StaticClassesToBePreloadedTransformer implements ClassFileTransformer {

	private final static boolean DEBUG = false;

	// Used for static class instrumentation prior to JINE execution
	private static int ioClassToStaticallyInstrument = -1;	// determines which system class will be statically instrumented 
	
	private static String[] allStaticallyInstrumentedClasses = {
		"java.lang.Thread",	
		"java.net.ServerSocket", "java.net.Socket", "java.net.SocketInputStream", "java.net.SocketOutputStream",
		"java.io.FileOutputStream", "java.io.FileInputStream", "java.io.RandomAccessFile", "java.io.UnixFileSystem",
	};
	
	private static String[] staticallyInstrumentedIoClasses = {
		"java/net/SocketInputStream", "java/net/SocketOutputStream", 
		"java/io/FileOutputStream", "java/io/FileInputStream", "java/io/RandomAccessFile", "java/io/UnixFileSystem",
	};

	
	/*
	 * Constructor 
	 */
	private StaticClassesToBePreloadedTransformer() {
		debugLog("ClassTransformer enabled!");
	}
	
	/*
	 * Static instrumentation of the i-th such Java system class (internal)	
	 */
	public static void parseArgs(String options) {
		
		try {
			ioClassToStaticallyInstrument=Integer.parseInt(options);
			
			if (ioClassToStaticallyInstrument >= allStaticallyInstrumentedClasses.length) {
				throw new NumberFormatException();
			}
		} catch (NumberFormatException ne) {
			System.out.println("Please select the id of the Java class to instrument from: 0 - " + (allStaticallyInstrumentedClasses.length - 1) + ", e.g. javaagent:staticinstrument.jar=2");
			System.exit(-1);
		}
	}
	
	
	/*
	 * The first method that is called when the agent is loaded
	 */
	public static void premain(String options, Instrumentation ins) {
		debugLog("ClassTransformer premain starting..." + Thread.currentThread().getName());
				
		if (options != null) {
			parseArgs(options);
		} 
			
		staticClassInstrumentation(ins, new StaticClassesToBePreloadedTransformer());
		
	}
	
	/*
	 * Method used to retransform system classes and store them into modified bytecode files
	 * Static instrumentation prior to actual simulation.
	 */
	@SuppressWarnings("unchecked")
	private static void staticClassInstrumentation(Instrumentation ins, StaticClassesToBePreloadedTransformer vtfClassTransformer) {
		// loaded IO library creation	

		// re-transformation capable - used to create VTF IO classes 
		ins.addTransformer(vtfClassTransformer, true);   

		Class[] list = ins.getAllLoadedClasses();

		String className;
		//"java.lang.System",

		try {
			ClassLoader.getSystemClassLoader().loadClass(allStaticallyInstrumentedClasses[ioClassToStaticallyInstrument]);
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
							if (className.equals(allStaticallyInstrumentedClasses[ioClassToStaticallyInstrument])) {
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
	 * The main transformation method called for every loaded class
	 * The isProfiled flag decides the classes to be instrumented
	 * and the instrumentClass instruments them.
	 */
	public byte[] transform(ClassLoader loader, String className, Class<?> cBR, java.security.ProtectionDomain pD, byte[] classfile) throws IllegalClassFormatException {
		
		if (!isInstrumentable(className)) { 
			return null; // Don't instrument.className: 
		}

		byte[] newBytes = null;
		try {
			newBytes = instrumentClass(className, classfile, true);

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
	
	

	/**
	 * Check if the loaded class should be statically instrumented
	 */
	private static boolean classShouldBeStaticallyInstrumented(String className) {
		for (String staticallyInstrumentedClass : allStaticallyInstrumentedClasses) {
			if (staticallyInstrumentedClass.replace('.','/').equals(className)) {
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

		// We need to include the java/io package for the vtfIO.jar creation
		if (classShouldBeStaticallyInstrumented(className)) {
			return false;
		} else {
			return true;
		}
	}
	
	
	/**
	 * Methods to check if a class should be instrumented
	 * 
	 * @param className The (fully-qualified) name of the class *
	 * @return True if the class is an instrumentation class.
	 */
	public static boolean isInstrumentClass(String className) {
		return className.equals("Transformer") || className.startsWith("org/apache/bcel/") || (className.startsWith("virtualtime/"));
	}

	public static boolean isInstrumentable(String className) {
		className = className.replace('.', '/');
		return !(isSystemClass(className) || isInstrumentClass(className));
	}

	private static boolean isIoClass(String cname) {
		for (String ioClass : staticallyInstrumentedIoClasses) {
			if (ioClass.equals(cname)) {
				return true;
			}
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
	
			if (className.startsWith("java")) {
				if (className.equals("java/lang/Thread")) {
					adapt = new ThreadAdapter(writer);
				} else if (isIoClass(className)) {
					adapt = new IoAdapter(writer);
				} else if (className.equals("java/net/Socket") || className.equals("java/net/ServerSocket")) {
					adapt = new ExplicitWaitingAdapter(writer);
//				} else if (className.equals("java/lang/System")) {
//					adapt = new SystemAdapter(writer);
				}
				reader.accept(adapt, 0);
			}
			
			return writer.toByteArray();
		} else {
			return null;
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
	 * Used to instrument classes and store them into an output class for inspection
	 */
	public static void main(String[] argv) {
	
	}

	

}
