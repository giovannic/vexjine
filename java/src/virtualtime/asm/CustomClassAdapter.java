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

	public CustomClassAdapter(ClassVisitor arg0) {
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
	}


	/*
	 * Visitor to a method
	 * 
	 * Distinguish between different methods from different classes and call the
	 * corresponding methodAdapter
	 */
	@Override
    public MethodVisitor visitMethod(int access, String name, String desc, String signature, String[] exceptions) {
		
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


}

