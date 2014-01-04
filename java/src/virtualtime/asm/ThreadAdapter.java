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



/*
 * The adapter class for java.lang.Thread
 * Used to define the extra Thread fields required by JINE
 * and instrument the interrupt method to occur in Virtual Time.
 */
public class ThreadAdapter extends ClassAdapter {

	ClassWriter myVisitor;
	boolean profiling = true;

	private String cname; // the instrumented class name


	private static final boolean debug = false; // debugging enabling flag

	public ThreadAdapter(ClassVisitor arg0) {
		super(arg0);
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
		createThreadFields();
	}


	/*
	 * Used to statically instrument Thread classes to wake-up threads when they call interrupt
	 */
	private MethodVisitor staticThreadInstrumentation(int access, String name, String desc, String signature, String[] exceptions) {
		if (name.equals("interrupt") || name.equals("start") || name.equals("join")) { 
			// Create the wrapper method
			myVisitor.newMethod(cname, name, desc, false);        	

			MethodVisitor mv = myVisitor.visitMethod(access, name, desc, signature, exceptions);
			if (debug) {
				System.out.println(cname + " "+ name +" " +desc);
			}
			return new JavaThreadMethodAdapter(mv, cname, name, desc);
	
		// NOTE: This does not work, because the Thread constructor invokes a registerNativeMethods method, 
		// that requires that sleep(J)V is native. We cannot replace the original method. We would like to do
		// that to avoid handling subclasses of Thread calling directly the sleep method
//		} else if (name.equals("sleep") && ((access & ACC_NATIVE) != 0)) {
//			
//			MethodVisitor mv = myVisitor.visitMethod(access ^ ACC_NATIVE, name, desc, signature, exceptions);
//			mv.visitCode();
//			mv.visitVarInsn(LLOAD, 0);
//			mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "sleepInVirtualTime", "(J)V");
//			mv.visitInsn(RETURN);
//			mv.visitMaxs(0, 0);
//			return mv;	
		
		} else {
			return cv.visitMethod(access, name, desc, signature, exceptions);
			
		}
	}
	
	/*
	 * Visitor to a method
	 * 
	 * Distinguish between different methods from different classes and call the
	 * corresponding methodAdapter
	 */
	@Override
    public MethodVisitor visitMethod(int access, String name, String desc, String signature, String[] exceptions) {
   		return staticThreadInstrumentation(access, name,desc,signature,exceptions);
    }	
	
	/*
	 * Utility instrumentation methods
	 */
	@SuppressWarnings("unchecked")
	private void createSetter(String propertyName, String type, Class c) {
		String methodName = "set" + propertyName.substring(0, 1).toUpperCase() + propertyName.substring(1);
		MethodVisitor mv = myVisitor.visitMethod(ACC_PUBLIC, methodName, "(" + type + ")V", null, null);
		mv.visitVarInsn(ALOAD, 0);
		mv.visitVarInsn(ILOAD, 1);
		mv.visitFieldInsn(PUTFIELD, cname, propertyName, type);
		mv.visitInsn(RETURN);
		mv.visitMaxs(0, 0);
	}

	@SuppressWarnings("unchecked")
	private void createGetter(String propertyName, String returnType, Class c) {
		String methodName = "get" + propertyName.substring(0, 1).toUpperCase() + propertyName.substring(1);
		MethodVisitor mv = myVisitor.visitMethod(ACC_PUBLIC, methodName, "()" + returnType, null, null);
		mv.visitVarInsn(ALOAD, 0);
		mv.visitFieldInsn(GETFIELD, cname, propertyName, returnType);
		mv.visitInsn(IRETURN);
		mv.visitMaxs(0, 0);
	}

	public static void insertPrintMsg(MethodVisitor mv, String message) {
 		mv.visitFieldInsn(GETSTATIC, "java/lang/System", "out", "Ljava/io/PrintStream;");
		mv.visitLdcInsn(message);
		mv.visitMethodInsn(INVOKEVIRTUAL, "java/io/PrintStream", "println", "(Ljava/lang/String;)V");
	}


	/*
	 * Method used to generate the fields in the java.lang.Thread class needed for Jine
	 */
	private void createThreadFields() {
		// Add vtfIoInvocationPoint
		myVisitor.visitField(ACC_PRIVATE, "vtfIoInvocationPoint", "I",
				null, 0);
		createGetter("vtfIoInvocationPoint", "I", Integer.class);
		createSetter("vtfIoInvocationPoint", "I", Integer.class);

		// Add vtfStackDepth
		myVisitor.visitField(ACC_PRIVATE, "vtfStackDepth", "I", null, 0);
		createGetter("vtfStackDepth", "I", Integer.class);
		createSetter("vtfStackDepth", "I", Integer.class);

		// Add vtfProfiled
		myVisitor.visitField(ACC_PRIVATE, "vtfProfiled", "Z", null, 0);
		createGetter("vtfProfiled", "Z", Boolean.class);
		createSetter("vtfProfiled", "Z", Boolean.class);

		// Add methods to handle these fields
		MethodVisitor mv = myVisitor.visitMethod(ACC_PUBLIC, "incVtfStackDepth", "()I", null, null);
		mv.visitVarInsn(ALOAD, 0);
		mv.visitFieldInsn(GETFIELD, cname, "vtfStackDepth", "I");
		mv.visitInsn(ICONST_1);
		mv.visitInsn(IADD);
		mv.visitVarInsn(ISTORE, 1);
		mv.visitVarInsn(ALOAD, 0);
		mv.visitVarInsn(ILOAD, 1);
		mv.visitFieldInsn(PUTFIELD, cname, "vtfStackDepth", "I");
		mv.visitVarInsn(ALOAD, 0);
		mv.visitFieldInsn(GETFIELD, cname, "vtfStackDepth", "I");
		mv.visitInsn(IRETURN);
		mv.visitMaxs(0, 0);

		mv = myVisitor.visitMethod(ACC_PUBLIC, "decVtfStackDepth", "()V", null, null);
		mv.visitVarInsn(ALOAD, 0);
		mv.visitFieldInsn(GETFIELD, cname, "vtfStackDepth", "I");
		mv.visitInsn(ICONST_1);
		mv.visitInsn(ISUB);
		mv.visitVarInsn(ISTORE, 1);
		mv.visitVarInsn(ALOAD, 0);
		mv.visitVarInsn(ILOAD, 1);
		mv.visitFieldInsn(PUTFIELD, cname, "vtfStackDepth", "I");
		mv.visitInsn(RETURN);
		mv.visitMaxs(0, 0);
	}
	
}


/*****
 * 
 * MethodAdapter class JavaThreadMethodAdapter
 * 
 * JavaThreadMethodAdapter is used to wrap the Thread.interrupt method so that
 * can be used to wakeup VTF blocked threds from any thread of the application
 */
class JavaThreadMethodAdapter extends MethodAdapter {

	private String mName;
	
	public JavaThreadMethodAdapter(MethodVisitor arg0, String className, String methodName, String _desc) {

		super(arg0);
		mName = methodName;
		
		if (methodName.equals("interrupt")) {
			mv.visitVarInsn(ALOAD, 0);
			mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "beforeThreadInterrupt", "(Ljava/lang/Thread;)I");

			Label skipActualInterrupt = new Label();
			mv.visitJumpInsn(IFEQ, skipActualInterrupt);
			mv.visitInsn(RETURN);
			mv.visitLabel(skipActualInterrupt);
		}
	}

	@Override
	public void visitCode() {
		mv.visitCode();
		if (mName.equals("start")) {
			mv.visitVarInsn(ALOAD, 0);
			mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "beforeThreadStart", "(Ljava/lang/Thread;)V");
		} 
	
	}

	@Override
	public void visitMethodInsn(int opcode, String owner, String name, String desc) {
		if (mName.equals("start")) {
			if (opcode == INVOKESPECIAL && name.equals("start0")) {
				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "getTimeBeforeCreatingThread", "()V");
				mv.visitMethodInsn(opcode, owner, name, desc);
				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "getTimeAfterCreatingThread", "()V");
			} else {
				mv.visitMethodInsn(opcode, owner, name, desc);
			}
		} else if (mName.equals("join") && name.equals("wait") && owner.equals("java/lang/Object")) {
			// Log the id of the joining thread before starting to wait, so that it wakes you up when it does so 
			// (before the JVM scheduler actually does) 
			mv.visitVarInsn(ALOAD, 0);
			mv.visitMethodInsn(INVOKEVIRTUAL, "java/lang/Thread", "getId", "()J");
			mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "logJoiningThread", "(J)V");
			mv.visitMethodInsn(opcode, owner, name, desc);
		} else {
			mv.visitMethodInsn(opcode, owner, name, desc);
		}
	}

	@Override
	public void visitInsn(int opcode) {
		mv.visitInsn(opcode);
	}

}
