/*

 * Created on May 28, 2008
 *
 * Changelog for Java instrumentation library
 * 
 * BasicBlockMethodAdapter & AccelerateAnnotationVisitor for basic VTF instrumentation (from prev versions)
 * VTF2.0: NativeIOMethodAdapter for static I/O instrumentation
 * VTF2.8: SocketMethodAdapter for internal socket instrumentation
 * VTF3.0: 
 * - added shouldTransformWaits flag in BasicBlockMethodAdapter to denote, whether waits should be replaced (used for real time Visualizer)
 * - Static instrumentation of java/lang/Thread class
 * 		-> adding vtfInvocation points to filter only points of interest
 * 		-> setting all join methods to synchronized... (required but?)    
 * 		-> identification with ids
 */
package virtualtime.asm;

import static org.objectweb.asm.Opcodes.*;

import org.objectweb.asm.ClassAdapter;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.ClassWriter;
import org.objectweb.asm.Label;
import org.objectweb.asm.MethodAdapter;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.commons.GeneratorAdapter;


/*
 * The class responsible for I/O instrumentation.
 * The native I/O methods of FileInputStream/FileOutputStream etc are statically instrumented
 * to notify VTF about I/O operations. Static instrumentation means that the corresponding
 * system classes will be instrumented before any profiling execution into a jar library. This
 * will then be preloaded (-Xbootclasspath/p:libraryName.jar) so that the instrumented classes
 * are used instead of the normal system classes.
 */
public class IoAdapter extends ClassAdapter {
	private static int invocationPointId = 0; 			// the id of an I/O method invocation point

	static int ioMethodId;
	static int fileOutputStreamMethodIdPtr = 1;	// output I/O method ids start from that position in ioIds 
	static int fileInputStreamMethodIdPtr = 4; 	
	static int randomAccessMethodIdPtr = 9;	
	static int socketInputStreamMethodIdPtr = 13;
	static int socketOutputStreamMethodIdPtr = 14;
	static int unixIOMethodIdPtr = 15;			
	static int sunChFileDispatcherIOMethodIdPtr = 30;

	//static int[] ioIds = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 39, 41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107};
	
	ClassWriter myVisitor;
	
	private String cname; // the instrumented class name

	private static final boolean debug = false; // debugging enabling flag

	public IoAdapter(ClassVisitor arg0) {
		super(arg0);
		myVisitor = (ClassWriter) arg0;
	}


	@Override
	public void visit(int version, int access, String name, String signature, String superName, String[] interfaces) {
		this.cname = name;
		cv.visit(version, access, name, signature, superName, interfaces);
	}


	/*
	 * Visitor to the I/O classes - selection only of the methods that should be wrapped
	 */
	@Override
	public MethodVisitor visitMethod(int access, String name, String desc, String signature, String[] exceptions) {


		if (isIoClassMethod(name, desc, access)) {

			if (debug) {
				System.out.println("Instrumenting method: '" + name+ "' '" + desc + "' '" + signature + "' ");
			}	
			return staticIoInstrumentation(access, name, desc, signature, exceptions);

			// other system classes that were decided to be instrumented
		} else {	
			return cv.visitMethod(access, name, desc, signature, exceptions);
		}
	}	



	private boolean isIoClassMethod(String name, String desc, int access) {
		if (cname.equals("java/io/FileInputStream") || cname.equals("java/io/FileOutputStream") || cname.equals("java/io/RandomAccessFile")) {  
			if (name.equals("open") || name.equals("openAppend") || (!cname.startsWith("java/net") && name.equals("available")) || 
					name.equals("skip") || ((name.equals("writeBytes") || name.equals("write")) && desc.equals("(I)V")) ||
					name.equals("readBytes") || (name.equals("read") && desc.equals("()I"))) {
				return true;
			}
		} else if (cname.equals("java/net/SocketInputStream") || cname.equals("java/net/SocketOutputStream") ) {    		
			if (name.equals("socketRead0") || name.equals("socketWrite0")) {
				return true;	
			}
		} else if (cname.equals("sun/nio/ch/FileDispatcher")) {
			if (name.equals("read0") || name.equals("pread0") || name.equals("readv0")
					|| name.equals("write0")|| name.equals("pwrite0")|| name.equals("writev0")) {
				return true;
			}			
		} else if (cname.equals("java/io/UnixFileSystem")) {
			if ((access & ACC_NATIVE) != 0) {
				return true;
			}
		}
		return false;
	}


	/*
	 * Used to statically instrument I/O classes to let the I/O module handle them in Virtual Time
	 */
	private MethodVisitor staticIoInstrumentation(int access, String name, String desc, String signature, String[] exceptions) {

		boolean isNative = (access & ACC_NATIVE) != 0;

		if (isNative) {
			// Create the wrapper method
			myVisitor.newMethod(cname, name, desc, false);        	
			myVisitor.visitMethod(access, "__vtf_native_prefix_" + name, desc, signature, exceptions);

			MethodVisitor mv = myVisitor.visitMethod(access ^ ACC_NATIVE, name, desc, signature, exceptions);
			if (cname.equals("java/io/FileInputStream")) {
				ioMethodId = fileInputStreamMethodIdPtr++;
			} else if (cname.equals("java/io/FileOutputStream")) {
				ioMethodId = fileOutputStreamMethodIdPtr++;
			} else if (cname.equals("java/io/RandomAccessFile")) {
				ioMethodId = randomAccessMethodIdPtr++;
			} else if (cname.equals("java/net/SocketInputStream")) {
				ioMethodId = socketInputStreamMethodIdPtr++;
			} else if (cname.equals("java/net/SocketOutputStream")) {
				ioMethodId = socketOutputStreamMethodIdPtr++;
			} else if (cname.equals("sun/nio/ch/FileDispatcher")) {
				ioMethodId = sunChFileDispatcherIOMethodIdPtr++;
			} else {
				ioMethodId = unixIOMethodIdPtr++;
			}

			if (debug) {
				System.out.println(cname + " "+ name +" " +desc + " - " + ioMethodId);
			}
			return new NativeIOMethodAdapter(mv, ioMethodId, access, cname, name, desc);
		} else {
			return cv.visitMethod(access, name, desc, signature, exceptions);
		}
	}

	public static int getNextInvocationPointId() {
		return ++invocationPointId;
	}
}

/* ****
 * 
 * MethodAdapter class NativeIOMethodAdapter
 * 
 * NativeIOMethodAdapter is used to wrap native IO methods for - I/O classes:
 * FileInputStream/FileOutputStream/RandomAccessStream - socket I/O:
 * socketRead/socketWrite methods from the java.net.Socket
 */
class NativeIOMethodAdapter extends MethodAdapter {
	
	private int countArgs(String desc) {
		String innerDesc = desc.substring(1, desc.lastIndexOf(')'));	// removing ()
		int totalArguments = 0;
		while (!innerDesc.equals("")) {
		
			if (innerDesc.startsWith("L")) {
				innerDesc = innerDesc.substring(innerDesc.indexOf(';')+1);
			} else if (innerDesc.startsWith("[")) {
				innerDesc = innerDesc.substring(2);
			} else {
				innerDesc = innerDesc.substring(1);
			}
			
			++totalArguments;
		}
		return totalArguments;
		
	}
	
	final static String[] blockingIoMethods = {"read0", "pread0", "readv0"};
	private boolean isBlockingIoMethod(String methodName) {
		for (String s : blockingIoMethods) {
			if (s.equals(methodName)) {
				return true;
			}
		}
		return false;
	}
	
	final static String[] notifyingIoMethods = {"write0", "pwrite0", "writev0"};
	private boolean isNotifyingIoMethod(String methodName) {
		for (String s : notifyingIoMethods) {
			if (s.equals(methodName)) {
				return true;
			}
		}
		return false;
	}
	
	public NativeIOMethodAdapter(MethodVisitor arg0, int methodId,
			int access, String className, String methodName, String desc) {

		super(arg0);

		// Define start of try block
		Label startFinally = new Label();
		mv.visitLabel(startFinally);

		// Do this to cache Thread.currentThread() throughout all instrumented calls
		int currentThreadLocalStorage = countArgs(desc) + 2;	// Note: the "expected" +1 had problems overwriting a Long (J) in the second position of the stack
		mv.visitMethodInsn(INVOKESTATIC, "java/lang/Thread", "currentThread", "()Ljava/lang/Thread;");
		mv.visitVarInsn(ASTORE, currentThreadLocalStorage);
		mv.visitVarInsn(ALOAD, currentThreadLocalStorage);

		// Step 1: Prepend the calling to the original I/O method by code to
		// contact the VTF agent with the corresponding id
		mv.visitIntInsn(SIPUSH, methodId); // the VTF-registered I/O method id

		boolean timeOutBasedIoOperation = false;
		if (methodName.equals("socketRead0")) {
			
			mv.visitVarInsn(ALOAD, 1); // the fd of the socket to check whether it is registered as internal
			mv.visitVarInsn(ILOAD, 4); // the bytes to read or write - I
			mv.visitVarInsn(ILOAD, 5); // the timeout
			
			mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "beforeSocketRead", "(Ljava/lang/Thread;ILjava/lang/Object;II)V");
			timeOutBasedIoOperation = true;
			
			// Replacing the timeout with -1 so that it will never timeout in real time - only in virtual time by the VTF scheduler
			mv.visitVarInsn(ALOAD, 0);mv.visitVarInsn(ALOAD, 1);mv.visitVarInsn(ALOAD, 2);mv.visitVarInsn(ILOAD, 3);mv.visitVarInsn(ILOAD, 4);
//			mv.visitInsn(ICONST_0);
			mv.visitVarInsn(ILOAD,5);
			
			mv.visitMethodInsn(INVOKESPECIAL, className, "__vtf_native_prefix_" + methodName, desc);


		} else if (isNotifyingIoMethod(methodName)) {
//			timeOutBasedIoOperation = true;
			// The methods are static
			mv.visitVarInsn(ALOAD, 0); // the fd of the socket to check whether it is registered as internal
			mv.visitVarInsn(ILOAD, 3); // the bytes to write 

			mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "afterWritingIoMethodEntry", "(Ljava/lang/Thread;ILjava/lang/Object;I)V");
			
		} else {
			mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "afterIoMethodEntry", "(Ljava/lang/Thread;I)V");
		}
		
		
		GeneratorAdapter mg = new GeneratorAdapter(mv, access, methodName, desc);
		if (!timeOutBasedIoOperation) {
			if ((access & ACC_STATIC) != 0) {
				mg.loadArgs();
				mv.visitMethodInsn(INVOKESTATIC, className, "__vtf_native_prefix_" + methodName, desc);				
			} else {
				mv.visitVarInsn(ALOAD, 0);
				mg.loadArgs();
				mv.visitMethodInsn(INVOKESPECIAL, className, "__vtf_native_prefix_" + methodName, desc);
			}
		}
		
		mv.visitVarInsn(ALOAD, currentThreadLocalStorage);
		mv.visitIntInsn(SIPUSH, methodId);
		mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "beforeIoMethodExit", "(Ljava/lang/Thread;I)V"); // beforeIoMethodExit is void -
    	
    	mg.returnValue();

		// Define end of try-block
		Label endFinally = new Label();

		// syntax: start of try, end of try block, exc handler type of thrown exceptions
		mv.visitTryCatchBlock(startFinally, endFinally, endFinally, "java/io/IOException");
		mv.visitLabel(endFinally);

		// Step 4: Set the correct arguments and call beforeIoMethodExit
		mv.visitMethodInsn(INVOKESTATIC, "java/lang/Thread", "currentThread", "()Ljava/lang/Thread;");
		mv.visitIntInsn(SIPUSH, methodId);
		mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "beforeIoMethodExit", "(Ljava/lang/Thread;I)V"); // beforeIoMethodExit is void -

		// the stack doesn't change
		mv.visitInsn(ATHROW);

		mv.visitMaxs(1, 1);

	}

	@Override
	public void visitCode() {
		mv.visitCode();
	}

	@Override
	public void visitMethodInsn(int opcode, String owner, String name,
			String desc) {
		mv.visitMethodInsn(opcode, owner, name, desc);
	}

	@Override
	public void visitInsn(int opcode) {
		mv.visitInsn(opcode);
	}

}
