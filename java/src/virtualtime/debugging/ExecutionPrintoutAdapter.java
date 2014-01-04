package virtualtime.debugging;

import static org.objectweb.asm.Opcodes.INVOKESTATIC;

import java.util.Vector;

import org.objectweb.asm.ClassAdapter;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.MethodAdapter;
import org.objectweb.asm.MethodVisitor;

public class ExecutionPrintoutAdapter extends ClassAdapter {
	
	protected static final boolean debug = false; // debugging enabling flag
	
	protected String cname = null;
	protected Vector<String> methodsToAddInstruments = null;
	public ExecutionPrintoutAdapter(ClassVisitor arg0, String className, Vector<String> methodsToAddInstruments) {
		super(arg0);
		this.methodsToAddInstruments = methodsToAddInstruments;
		cname = className;
	}
	
	public MethodVisitor visitMethod(int access, String name, String desc, String signature, String[] exceptions) {
		
		MethodVisitor mv = cv.visitMethod(access, name, desc, signature, exceptions);
		if (methodsToAddInstruments == null || methodsToAddInstruments.contains(name)) {
			return new PrintOutMessageAdapter(mv, cname, name);	
		} else {
			return mv;
		}
		
	}	
	
}


class PrintOutMessageAdapter extends MethodAdapter {

	private String cName;
	private String mName;
	
	public PrintOutMessageAdapter(MethodVisitor arg0, String cName, String mName) {
		super(arg0);
		this.cName = cName;
		this.mName = mName;
	}
	
	
	/***
	 * Visitor of method code
	 * 
	 * Sets the afterMethodEntry - beforeMethodExit hooks to the VTF If the
	 * method is synchronised it also wraps the method with monitorEnter -
	 * monitorExit
	 */
	@Override
	public void visitCode() {
		mv.visitCode();
		mv.visitLdcInsn(cName);
		mv.visitLdcInsn(mName);
		mv.visitMethodInsn(INVOKESTATIC, "virtualtime/debugging/MessagePrinter", "print", "(Ljava/lang/String;Ljava/lang/String;)V");
	}
}