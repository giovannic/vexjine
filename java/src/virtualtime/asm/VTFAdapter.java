package virtualtime.asm;

import static org.objectweb.asm.Opcodes.ACC_INTERFACE;
import static org.objectweb.asm.Opcodes.ACC_STATIC;
import static org.objectweb.asm.Opcodes.ACC_SYNCHRONIZED;
import static org.objectweb.asm.Opcodes.ALOAD;
import static org.objectweb.asm.Opcodes.ATHROW;
import static org.objectweb.asm.Opcodes.DUP;
import static org.objectweb.asm.Opcodes.GETSTATIC;
import static org.objectweb.asm.Opcodes.INVOKESPECIAL;
import static org.objectweb.asm.Opcodes.INVOKESTATIC;
import static org.objectweb.asm.Opcodes.INVOKEVIRTUAL;
import static org.objectweb.asm.Opcodes.MONITORENTER;
import static org.objectweb.asm.Opcodes.SIPUSH;

import org.objectweb.asm.ClassAdapter;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.ClassWriter;
import org.objectweb.asm.Label;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.GeneratorAdapter;

import virtualtime.EventNotifier;
import virtualtime.InstrumentationParameters;
import virtualtime.MethodAdaptationInfo;
import virtualtime.generators.MethodId;

public class VTFAdapter extends ClassAdapter {
	
	protected static final boolean debug = false; // debugging enabling flag
	
	protected static MethodId methodIdGenerator = null;	// used for stack trace mode
	protected static int methodIdBeforeRetransformation = 0;
	
	protected ClassWriter myVisitor;
	protected String cname; 						// the instrumented class name
	protected boolean isInterface;
	protected boolean profiling;
	
	protected boolean isSuperclassThread;			// small hack to identify sleep methods when the direct superclass is java.lang.Thread - we avoid the entire tree to avoid synchronization points on a common data structure 
	
	protected InstrumentationParameters iparams;
	protected MethodAdaptationInfo adaptationInfo; 	// used for methods that are substituted by models
	
	protected final static String vtfMethodEntryCallback = "_afterMethodEntry";
	protected final static String vtfMethodExitCallback = "_beforeMethodExit";
	protected final static String performanceModelMethodIdentifier = "UsingPerformanceModel";
	
	public VTFAdapter(ClassVisitor arg0) {
		super(arg0);
		isSuperclassThread = false;
	}

	
	public static void initialize(MethodId _methodIdGenerator) {
		methodIdGenerator = _methodIdGenerator; 
	}
	
	
	public static void onRetransformationStart(int method0) {
		methodIdGenerator.rollbackToMethodId(method0);
	}
	
	public static void onRetransformationEnd() {
		methodIdGenerator.returnToCurrentMethodId();
	}
	
	
	/*
	 * Get class name for future use and check if this is an interface
	 */
	@Override
	public void visit(int version, int access, String name, String signature, String superName, String[] interfaces) {
		this.cname = name;
		// NOTE: this is not effective as the wrongly sleeping class (new SubclassOfThread().sleep instead of Thread.sleep)
		// might not be loaded, when we make the check - this is quite hopeless - TODO: fix it even for wrong handling 
		//iparams.addPossibleSubclassOfJavaLangThread(name, superName);
		
		iparams.setWhetherJavaLangThreadIsSuperclass(superName);
		cv.visit(version, access, name, signature, superName, interfaces);
		isInterface = (access & ACC_INTERFACE) != 0;
	}


	/*
	 * Used to wrap a synchronised method with another non synchronized one in order to include an interaction point before
	 * calling the sync-method.
	 */
	protected MethodVisitor wrapSynchronizedMethod(int access, String name, String desc, String signature, String[] exceptions, boolean profileMethod) {
		
		boolean isStatic = (access & ACC_STATIC) != 0;
		int newAccess = access ^ ACC_SYNCHRONIZED;
		
		int methodId = methodIdGenerator.getCurrentMethodId();
		if (profileMethod && !iparams.isRetransformingInstrumentation()) {
			EventNotifier.registerMethod(cname + " " + name + " " + desc, methodId, iparams.getMethod0());
		}
		
		myVisitor.newMethod(cname, name, desc, false);
		
		
		MethodVisitor mv2 = myVisitor.visitMethod(newAccess, name, desc, signature, exceptions);
		
		boolean instrumentExceptionHandlingCode = exceptions != null && exceptions.length > 0;
		Label startExceptionCatchingBlockLabel = null;
		// Denote the start of the method - try {
		if (instrumentExceptionHandlingCode) {
			startExceptionCatchingBlockLabel = new Label();
			mv2.visitLabel(startExceptionCatchingBlockLabel);
		}		
		
		GeneratorAdapter mg = new GeneratorAdapter(mv2, newAccess, name, desc);

		if (iparams.onlyLimitedJvmtiUsage()) {
			if (isStatic) {
				mv2.visitLdcInsn(Type.getType("L" + cname + ";"));
			} else {
				mv2.visitVarInsn(ALOAD, 0);
			}
//			mv2.visitMethodInsn(INVOKEVIRTUAL, "java/lang/Object", "hashCode", "()I");
			mv2.visitMethodInsn(INVOKESTATIC, "java/lang/System", "identityHashCode", "(Ljava/lang/Object;)I");

		}
		
		if (iparams.trappingInteractionPoints()) {
			mv2.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "_interactionPoint", "()V");
		}
		
		if (iparams.onlyLimitedJvmtiUsage()) {
			mv2.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier","_beforeAcquiringMonitor","(I)V");
		}		
		
		int invocationType = INVOKESPECIAL;
		if (isStatic) {		
			invocationType = INVOKESTATIC;
		} else {
			mv2.visitVarInsn(ALOAD, 0);	
		}

		mg.loadArgs();
		if (profileMethod) {
			insertMethodInstrument(mv2, methodId, vtfMethodEntryCallback);
		}
		
		mv2.visitMethodInsn(invocationType, cname, "_vtfsynced_"+name, desc);
		if (profileMethod) {
			insertMethodInstrument(mv2, methodId, vtfMethodExitCallback);
		}
		
		if (iparams.trappingInteractionPoints()) {
			mv2.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "_interactionPoint", "()V");
		}
		
//		if (iparams.onlyLimitedJvmtiUsage()) {
//			if (isStatic) {
//				mv2.visitLdcInsn(Type.getType("L" + cname + ";"));
//			} else {
//				mv2.visitVarInsn(ALOAD, 0);
//			}
		// mv2.visitMethodInsn(INVOKEVIRTUAL, "java/lang/Object", "hashCode", "()I");
//			mv2.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier","_beforeReleasingMonitor","(I)V");
//		}
		
		mg.returnValue();
		
		// Instrument the exception handler for VTF exiting invocation, only if
		// this method may throw exceptions
		Label endExceptionCatchingBlockLabel = null;
		if (instrumentExceptionHandlingCode) {
			endExceptionCatchingBlockLabel = new Label();
			mv2.visitLabel(endExceptionCatchingBlockLabel);

			// Used to print all intermediate exceptions
			// mv.visitInsn(DUP);
			// mv.visitIntInsn(SIPUSH, methodId);
			// mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier","printException","(Ljava/lang/Throwable;I)V");

			if (profileMethod) {
				insertMethodInstrument(mv2, methodId, vtfMethodExitCallback);
			}

//			if (iparams.onlyLimitedJvmtiUsage()) {
//				if (isStatic) {
//					mv2.visitLdcInsn(Type.getType("L" + cname + ";"));
//				} else {
//					mv2.visitVarInsn(ALOAD, 0);
//				}
//				mv2.visitMethodInsn(INVOKEVIRTUAL, "java/lang/Object", "hashCode", "()I");
//				mv2.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier","_beforeReleasingMonitor","(I)V");
//			}
			
			mv2.visitTryCatchBlock(startExceptionCatchingBlockLabel, endExceptionCatchingBlockLabel, endExceptionCatchingBlockLabel, null);
			mv2.visitInsn(ATHROW);
		}		
		
		mv2.visitMaxs(0,0);

		if (!iparams.shouldRemoveMonitors()) {
			return cv.visitMethod(access, "_vtfsynced_"+name, desc, signature, exceptions);
		} else {
			return cv.visitMethod(access ^ ACC_SYNCHRONIZED, "_vtfsynced_"+name, desc, signature, exceptions);
		}
	}

	
	/*
	 * Basic instrumentation method 
	 */
	protected MethodVisitor callBasicBlockAdapter(MethodVisitor mv, int access, String name, String desc, String signature, String[] exceptions) {
    
		if (!isInterface) {

			int method=0; 
			if (profiling) {			// determines whether the method will be profiled
				method = methodIdGenerator.getNextMethodId(cname, name, desc);
			}

			// Get externally provided adaptation info
			adaptationInfo = iparams.registerExternallyProvidedMethodPerformanceFactors(method, cname + name);
			
			// Instrument all - profile only selected methods
			mv = new BasicBlockMethodAdapter(mv, method, profiling, access, cname, name, desc, signature, exceptions,  iparams, adaptationInfo);
		}

		return mv;
	}

	
	/*
	 * Generic methods for method related instrument insertion
	 */
	protected static void insertMethodInstrument(MethodVisitor mv, int thisMethodId, String eventNotifierMethod) { 
		setProperMethodIdLoadInstruction(mv, thisMethodId);
		mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", eventNotifierMethod, "(I)V");		
	}
	
	/*
	 * Set proper id: short if < 32767 or integer if >= 32767
	 */
	private static void setProperMethodIdLoadInstruction(MethodVisitor mv, int thisMethodId) { 
		if (thisMethodId < 32767) {
			mv.visitIntInsn(SIPUSH, thisMethodId);
		} else {
			mv.visitLdcInsn(thisMethodId);
		}		
	}
	
	
	/*
	 * Utility instrumentation method to add checkpoints
	 */
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

	
}
