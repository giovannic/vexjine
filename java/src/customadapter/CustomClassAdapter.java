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
package customadapter;

import static org.objectweb.asm.Opcodes.*;

import org.objectweb.asm.AnnotationVisitor;
import org.objectweb.asm.ClassAdapter;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.ClassWriter;
import org.objectweb.asm.Label;
import org.objectweb.asm.MethodAdapter;
import org.objectweb.asm.MethodVisitor;

/*
 * The adapter class for java.lang.Thread
 * Used to define the extra Thread fields required by JINE
 * and instrument the interrupt method to occur in Virtual Time.
 */
public class CustomClassAdapter extends ClassAdapter {

	ClassWriter myVisitor;
	boolean profiling = true;

	private String cname; // the instrumented class name
	private InstrumentationParameters iparams = null;
	
	public CustomClassAdapter(ClassVisitor arg0, InstrumentationParameters _instrumentationParameters) {
		super(arg0);
		iparams = _instrumentationParameters;
		myVisitor = (ClassWriter) arg0;
	}


	/*
	 * Visitor to a class - used to augment class with new fields and methods
	 * 
	 * VTF3.1: Instruments java.lang.Thread to have a vtfIoInvocationPoint field
	 * that identified I/O invocations from each other before they are called.
	 * This means that before an I/O operation the thread's vtfIoInvocationPoint
	 * will be set to a particular id. When VTF agent is contacted this field
	 * will be read to identify the method and use the prediction scheme for
	 * that point.
	 */
	@Override
	public void visit(int version, int access, String name, String signature, String superName, String[] interfaces) {
		cname = name;
		cv.visit(version, access, name, signature, superName, interfaces);
	}


	/*
	 * Visitor to a method
	 * 
	 * Distinguish between different methods from different classes and call the
	 * corresponding methodAdapter
	 */
	@Override
    public MethodVisitor visitMethod(int access, String name, String desc, String signature, String[] exceptions) {
		
		if (cname.equals("org/eclipse/core/internal/jobs/Semaphore")) {
//			System.out.println("Instrumenting " + cname + " " + name + " " + desc);
			MethodVisitor mv = cv.visitMethod(access, name, desc, signature, exceptions);
			return new CustomMethodAdapter(mv, access, cname, name, desc, signature, exceptions, iparams);
			 
		}
		if (cname.equals("com/A/A/E/K")) {
			System.out.println(cname + " " + name + " " + desc);
			if (name.equals("H") && desc.equals("()Z")) {					
	        	MethodVisitor mv2 = myVisitor.visitMethod(access, name, desc, signature, exceptions);
	        	mv2.visitInsn(ICONST_1);
	        	mv2.visitInsn(IRETURN);
	        	mv2.visitMaxs(0,0);		
	        	return null;
			}
		} 
		return cv.visitMethod(access, name, desc, signature, exceptions);

	}	
	
	
	public static void insertPrintMsg(MethodVisitor mv, String message) {
 		mv.visitFieldInsn(GETSTATIC, "java/lang/System", "out", "Ljava/io/PrintStream;");
		mv.visitLdcInsn(message);
		mv.visitMethodInsn(INVOKEVIRTUAL, "java/io/PrintStream", "println", "(Ljava/lang/String;)V");
	}
	
	public static void insertPrintMsgWithThread(MethodVisitor mv, String message) {

 		mv.visitFieldInsn(GETSTATIC, "java/lang/System", "out", "Ljava/io/PrintStream;");
 		mv.visitMethodInsn(INVOKESTATIC, "java/lang/Thread", "currentThread", "()Ljava/lang/Thread;");
 		mv.visitMethodInsn(INVOKEVIRTUAL, "java/lang/Thread", "getName", "()Ljava/lang/String;");
		mv.visitMethodInsn(INVOKEVIRTUAL, "java/io/PrintStream", "print", "(Ljava/lang/String;)V");
 		mv.visitFieldInsn(GETSTATIC, "java/lang/System", "out", "Ljava/io/PrintStream;");
 		mv.visitLdcInsn(" ");
		mv.visitMethodInsn(INVOKEVIRTUAL, "java/io/PrintStream", "print", "(Ljava/lang/String;)V");
		mv.visitFieldInsn(GETSTATIC, "java/lang/System", "out", "Ljava/io/PrintStream;");
		mv.visitLdcInsn(message);
		mv.visitMethodInsn(INVOKEVIRTUAL, "java/io/PrintStream", "println", "(Ljava/lang/String;)V");
	}


	public static void checkPointMessage(int checkPoint, String name) { 
		System.out.println(Thread.currentThread() + " in " + name + " and checkpoint " + checkPoint);
		
	}
}







/*****
 * 
 * MethodAdapter class BasicBlockMethodAdapter
 * 
 * BasicBlockMethodAdapter is used for dynamic byte-code instrumentation of Java
 * methods that should be profiled from the loaded application classes. The
 * selection of the methods is made by the calling BasicBlockAdapter class
 */
class CustomMethodAdapter extends MethodAdapter {

	private boolean DEBUG = false;

	private boolean isSynchronised;
	private boolean profile;

	private boolean instrumentExceptionHandlingCode; // only deal with abrupt termination, if exceptions can be thrown

	private int methodId;
	private String className;
	private String methodName;
	
	Label startExceptionCatchingBlockLabel;
	Label finalSyncLabel;

	private InstrumentationParameters iparams;

	/***
	 * Constructor setting the parameters of the method
	 * @param arg0							the method visitor object to write the new code
	 * @param _methodId						the unique JINE id of the method
	 * @param isProfiled					should we add instrumentation code to profile the method
	 * @param access						access rules of the method
	 * @param cName							class name
	 * @param mName							method name
	 * @param desc							arguments description
	 * @param signature						signature
	 * @param exceptions					array of strings with the possibly thrown exception classes 
	 * @param _iparams						instrumentation parameters as selected by the user options
	 */
	public CustomMethodAdapter(MethodVisitor arg0, int access, String cName, String mName, String desc, String signature, String[] exceptions, InstrumentationParameters _iparams) {

		super(arg0);

		iparams = _iparams;

		
		className = cName;
		methodName = mName;
		
		isSynchronised = (access & ACC_SYNCHRONIZED) != 0;
		
		// Some methods
		if (iparams.exceptionHandlingEverywhere()) {
			instrumentExceptionHandlingCode = true;
		} else{
			instrumentExceptionHandlingCode = false;
		}
				
		if (DEBUG) {
			System.out.println("Instrumenting  " + cName + " -> " + mName + "(" + methodId + ")" + desc);
		}

		checkPoint = 1;
	}

	private int checkPoint;
	

	
	private void myInstrumentation() {
//		mv.visitIntInsn(SIPUSH, (checkPoint++));
//		mv.visitLdcInsn(className + " " + methodName);
//		mv.visitMethodInsn(INVOKESTATIC, "customadapter/CustomClassAdapter", "checkPointMessage", "(ILjava/lang/String;)V");
		CustomClassAdapter.insertPrintMsgWithThread(mv, " " + className + " " + methodName + " at " + (checkPoint++));
//		CustomClassAdapter.insertPrintMsg(mv, className + " " + methodName + " at " + (checkPoint++));
	}
	
	/***
	 * Visitor of the method-calling byte-code instructions called by the method
	 * 
	 * This allows us to instrument the method-calling byte-code instructions of a
	 * profiled method. These methods can be divided into the ones that: 
	 * - denote thread start
	 * - denote thread yield
	 * - denote thread wait
	 * - trigger synchronisation points 
	 * - should register I/O points
	 */
	@Override
	public void visitMethodInsn(int opcode, String owner, String name, String desc) {

		if (DEBUG) {
			System.out.println(owner + " --> " + name + ": " + desc + " (" + opcode + ")");
		}

		myInstrumentation();
		mv.visitMethodInsn(opcode, owner, name, desc);

	}

	@Override
	public void visitVarInsn(int opcode, int arg1) {
		mv.visitVarInsn(opcode, arg1);
		if (className.equals("org/eclipse/core/internal/jobs/Semaphore") && methodName.equals("acquire")) {
			System.out.println("ACQUIRE BC: " + opcode + " " + arg1 + " "  + checkPoint);
			if (opcode == LLOAD && arg1 == 5) {
				System.out.println("TO VRIKA");
		
				mv.visitFieldInsn(GETSTATIC, "java/lang/System", "out", "Ljava/io/PrintStream;");
				mv.visitVarInsn(LLOAD, 5);
				mv.visitMethodInsn(INVOKEVIRTUAL, "java/io/PrintStream", "println", "(J)V");
			}
		}

	}
	
	/***
	 * Visit any byte-code instruction
	 * 
	 * This allows us to instrument methods an the instruction level.
	 * Instructions that are instrumented this way regard exiting and
	 * synchronising byte-code instructions
	 */
	@Override
	public void visitInsn(int opcode) {

		myInstrumentation();
		
		// These are all instructions which terminate a method
		if ((opcode >= IRETURN && opcode <= RETURN) || (!instrumentExceptionHandlingCode && opcode == ATHROW)) {
			mv.visitInsn(opcode);
			return;

		}

		// Monitor enter instrumentation - suspend thread 
		mv.visitInsn(opcode);

	}
	
}

