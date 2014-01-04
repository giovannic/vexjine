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
 * 		-> setting all join methods to synchronised... (required but?)    
 * 		-> identification with ids
 */
package virtualtime.asm;

import java.util.Vector;

import virtualtime.EventNotifier;
import virtualtime.InstrumentationParameters;
import virtualtime.MethodAdaptationInfo;
import virtualtime.PerformanceModelStub;
import virtualtime.statistics.InstrumentationRecordFactory;
import virtualtime.statistics.InstrumentationStatsRecord;

import static org.objectweb.asm.Opcodes.*;

import org.objectweb.asm.AnnotationVisitor;
import org.objectweb.asm.Attribute;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.ClassWriter;
import org.objectweb.asm.Label;
import org.objectweb.asm.MethodAdapter;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Type;


/*
 * The basic adapter class
 * 
 * Identifies the kind of instrumentation (dynamic on runtime or static for system classes) for each method and
 * creates an instance of the appropriate MethodAdapter class.
 * 
 * Constructor params:
 * @param: ClassVisitor arg0 					-> the classVisitor instance for this class
 * @param: boolean profile						-> used for debugging to disable all dynamic profiling
 * @param: Vector<String> _profiledMethodNames	-> the FQN of the methods that should be profiled from this class (enabling selective profiling)
 * @param: boolean _shouldTransformWaits		-> used for debugging to instrument classes but disable Object.wait special handling
 * @param: boolean _isProfilingRun 				-> used to opt-out the synchronised substitution when we are performing a profiling run
 */
public class BasicBlockAdapter extends VTFAdapter {
	
	public BasicBlockAdapter(ClassVisitor arg0, boolean profile, InstrumentationParameters _iparams) {
		super(arg0);
		myVisitor = (ClassWriter) arg0;
		profiling = profile;
		iparams = _iparams;
		
		if (!iparams.isRetransformingInstrumentation()) {
			iparams.setMethod0(methodIdGenerator.getCurrentMethodId());	// define the first methodId of a method of the class that will be instrumented	
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
		
		if (debug) {
			System.out.println("Visiting method: " + cname + " '" + name+ "' '" + desc + "' '" + signature + "' ");
		}	
		
		MethodVisitor mv = null;
		boolean profilingMethod = profiling && iparams.shouldMethodBeProfiled(cname, name);
		if (!profilingMethod) {
			mv = cv.visitMethod(access, name, desc, signature, exceptions);
			return new BasicBlockMethodAdapter(mv, -1, false, access, cname, name, desc, signature, exceptions,  iparams, adaptationInfo);
		}
//		if (!profilingMethod) {
//			return cv.visitMethod(access, name, desc, signature, exceptions);
//		}
		
		if ((access & ACC_SYNCHRONIZED) != 0 && (iparams.trappingInteractionPoints() || iparams.onlyLimitedJvmtiUsage())) {
			mv = wrapSynchronizedMethod(access, name, desc, signature, exceptions, profilingMethod && !iparams.shouldInvalidateMethodProfilingOf(methodIdGenerator.getCurrentMethodId()));        	
		} else {
			mv = cv.visitMethod(access, name, desc, signature, exceptions);
		}
		return callBasicBlockAdapter(mv, access, name, desc, signature, exceptions);
	}	



}



/*****
 * 
 * MethodAdapter class BasicBlockMethodAdapter
 * 
 * BasicBlockMethodAdapter is used for dynamic byte-code instrumentation of Java
 * methods that should be profiled from the loaded application classes. The
 * selection of the methods is made by the calling BasicBlockAdapter class
 * 
 * The rules from http://asm.ow2.org/asm33/javadoc/user/org/objectweb/asm/MethodVisitor.html
 * The methods of this interface must be called in the following order: 
 * 
 * [ visitAnnotationDefault ] 
 * ( visitAnnotation | visitParameterAnnotation | visitAttribute )* 
 * [ visitCode 
 * ( visitFrame | visitXInsn | visitLabel | visitTryCatchBlock | visitLocalVariable | visitLineNumber)* 
 *   visitMaxs ] 
 * [ visitEnd  ]
 * 
 * In addition, the visitXInsn and visitLabel methods must be called in the sequential order of the bytecode instructions of the visited code, visitTryCatchBlock must be called before the labels passed as arguments have been visited, and the visitLocalVariable and visitLineNumber methods must be called after the labels passed as arguments have been visited. 
 */
class BasicBlockMethodAdapter extends MethodAdapter {

	private boolean DEBUG = false;

	private boolean isSynchronised;
	private boolean shouldBeWrappedWithVexOptimizerLoop;
	private boolean profile;

	private boolean instrumentExceptionHandlingCode; // only deal with abrupt termination, if exceptions can be thrown
	private VirtualTime_CritialSection critialSection = null;

	private int methodId;
	private String className;
	private String methodName;
	private String methodDesc;
	
	Label startExceptionCatchingBlockLabel;
	Label finalSyncLabel;
	
	Label startMainWrappedWithVexOptimizer;
	Label endMainWrappedWithVexOptimizer;

	Vector<Label> allStartingTryCatchingBlockLabels;
	Vector<Label> allEndingTryCatchingBlockLabels;
	int tryCatchDepth;	// used to determine whether the ATHROW opcodes should be preceded by an "onMethodExit" call or whether they are handled internally in the method 
	
	private InstrumentationParameters iparams;
	private MethodAdaptationInfo methodAdaptationInfo;
	private boolean isStatic;
	
	private PerformanceModelStub performanceModelParameters;
	
	private InstrumentationStatsRecord instrumentationStatsRecord;
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
	public BasicBlockMethodAdapter(MethodVisitor arg0, int _methodId, boolean isProfiled, int access, 
			String cName, String mName, String desc, String signature, String[] exceptions, 
			InstrumentationParameters _iparams, MethodAdaptationInfo _methodAdaptationInfo) {

		super(arg0);

		instrumentationStatsRecord = InstrumentationRecordFactory.getRecord();
		
		allStartingTryCatchingBlockLabels = null;
		allEndingTryCatchingBlockLabels = null;
		tryCatchDepth = 0;
		
		iparams = _iparams;
		methodAdaptationInfo = _methodAdaptationInfo;
		
		methodId = _methodId;
		
		className = cName;
		methodName = mName;
		methodDesc = desc;
		
		isSynchronised = (access & ACC_SYNCHRONIZED) != 0;
		isStatic = (access & ACC_STATIC) != 0;
		
		profile = isProfiled && !(isSynchronised && (iparams.trappingInteractionPoints() || (iparams.usingExternalInstrumentation())));	// sync methods are profiled externally when IPs are trapped 

		// Some methods
		if (iparams.exceptionHandlingEverywhere()) {
			instrumentExceptionHandlingCode = true;
		} else{
			instrumentExceptionHandlingCode = false;
		}
		
		// Register the methodId to the VTF
		if (profile) {
			instrumentationStatsRecord.setProfiled();
			
			if (!iparams.isRetransformingInstrumentation()) {
				EventNotifier.registerMethod(cName + " " + mName + " " + desc, methodId, _iparams.getMethod0());
			} else {
				instrumentationStatsRecord.setRetransformed();
			}
			
			shouldBeWrappedWithVexOptimizerLoop = iparams.shouldBeWrappedWithVexOptimizerLoop(mName); 	// wrap the first appearance of main
			if (shouldBeWrappedWithVexOptimizerLoop) {
				System.out.println("will wrap vex optimizer " + cName + " " + mName + " " + desc);				
			}
			
			
			if (iparams.methodShouldBeAdapted(methodId)) {
				if (exceptions != null && exceptions.length > 0) {
					instrumentExceptionHandlingCode = true;
				}
			}
		} 
		
		
		if (DEBUG) {
			System.out.println("Instrumenting  " + cName + " -> " + mName + "(" + methodId + ")" + desc);
		}

		performanceModelParameters = null;
	}
	
		

	/*
	 * Method declaring whether a method is described by a performance model
	 */
	private boolean isPerformanceDescribedByExternallyDefinedModel() {
		return methodAdaptationInfo != null && methodAdaptationInfo.isPerformanceDescribedByExternallyDefinedModel();
	}
	private boolean shouldMethodBodyBeRemovedAccordingToExternallyDefinedModel() {
		return methodAdaptationInfo != null && methodAdaptationInfo.shouldMethodBodyBeReplaced();
	}
	
	/***
	 * Parse the method annotations to register a virtual time specification
	 */
	@Override
	public AnnotationVisitor visitAnnotation(String desc, boolean visible) {
		if (DEBUG) {
			System.out.println("visitAnnotation " + "(" + methodName + " - " + methodId + ") " + desc );
		}

		// check for virtual time annotations..
		if (desc.equals("Lvirtualtime/Accelerate;")) {
			AccelerateAnnotationVisitor visitor = new AccelerateAnnotationVisitor(className, methodId);
			critialSection = visitor;
			return visitor;
		} else if (desc.equals("Lvirtualtime/ModelPerformance;")) {
			performanceModelParameters = new PerformanceModelStub();
			ModelPerformanceAnnotationVisitor visitor = new ModelPerformanceAnnotationVisitor(className, methodId, performanceModelParameters);
			critialSection = visitor;
			return visitor;
		}

		return super.visitAnnotation(desc, visible);
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
		if (DEBUG) {
			System.out.println("visitCode " + methodId);
		}

		mv.visitCode();
		
		if (critialSection != null) {
			critialSection.register();
		}
		
		if (shouldBeWrappedWithVexOptimizerLoop) {
			startMainWrappedWithVexOptimizer = new Label();
			mv.visitLabel(startMainWrappedWithVexOptimizer);
			mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "_shouldMainIterationContinue", "()Z");
			endMainWrappedWithVexOptimizer = new Label();
			mv.visitJumpInsn(IFEQ, endMainWrappedWithVexOptimizer);
		}
		   
		// Denote the start of the method - try {
		if (instrumentExceptionHandlingCode) {
			startExceptionCatchingBlockLabel = new Label();
			visitLabel(startExceptionCatchingBlockLabel);
		}

		if (profile) {
			onMethodEntryInstrumentation();
		}		
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
		// called at the beginning of a method
		if (removingMethodBody()) {
			return;
		}
		
		instrumentationStatsRecord.addInstruction();
		
		// Thread.yield - cannot be instrumented in java.lang.Thread, because it's a native method
		if (opcode == INVOKESTATIC && owner.equals("java/lang/Thread") && name.equals("yield") && desc.equals("()V")) {
			instrumentationStatsRecord.addSyncPrimitive();
			mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "_yield", "()V");
			return;
		}

		
		// Virtualizing waiting calls
		if (iparams.shouldTransformWaits()) {
				
			// In the limited JVMTI case we do not rely on JVMTI to trap Object.wait(), so we trap both indefinite and timed-waiting events
			if (iparams.onlyLimitedJvmtiUsage()) {
				if (opcode == INVOKEVIRTUAL && owner.equals("java/lang/Object") && name.equals("wait")) {
					instrumentationStatsRecord.addSyncPrimitive();
					
					if (iparams.shouldRemoveMonitors()) {
						mv.visitLdcInsn(ICONST_0);
					} else {
						mv.visitLdcInsn(ICONST_1);
					}
					
					if (desc.equals("()V")) {
						mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "waitInVirtualTimeWithoutJvmti", "(Ljava/lang/Object;Z)V");	
				
					} else if (desc.equals("(J)V")) {
						mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "waitInVirtualTimeWithoutJvmti", "(Ljava/lang/Object;JZ)V");	
						
					} else {
						mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "waitInVirtualTimeWithoutJvmti", "(Ljava/lang/Object;JIZ)V");				
						
					}
					return;
				}

			} else {
			// JVMTI traps Object.wait(), so we only trap timed-waiting events
				if (opcode == INVOKEVIRTUAL && owner.equals("java/lang/Object") && name.equals("wait") && desc.equals("(J)V")) {
					instrumentationStatsRecord.addSyncPrimitive();
					mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "waitInVirtualTime", "(Ljava/lang/Object;J)V");	
					return;
				}
	
				if (opcode == INVOKEVIRTUAL && owner.equals("java/lang/Object") && name.equals("wait") && desc.equals("(JI)V")) {
					instrumentationStatsRecord.addSyncPrimitive();
					// Object.wait(timeout, nanos)
					mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "waitInVirtualTime", "(Ljava/lang/Object;JI)V");
					return;
				}
			}
			
			
			// Hack to include subclasses of Thread that override sleep - naive onLoad check does not work
			if (name.equals("sleep") && (desc.equals("(J)V") || desc.equals("(JI)V")) && (!owner.equals(className) || iparams.isSubClassOfJavaLangThread())) { // && owner.equals("java/lang/Thread")) {	// || iparams.isExtendindJavaLangThread(owner)
				instrumentationStatsRecord.addSyncPrimitive();
//				System.out.println("changing sleep for " + owner + " "  + name + " " + desc + " " + className + " " + methodName);
				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "sleepInVirtualTime", desc);
				return;
			} else if (name.equals("sleep")) {
//				System.out.println("*NOT* changing sleep for " + owner + " "  + name + " " + desc + " " + className + " " + methodName);
			}
		}

		
		if (iparams.shouldTransformWaits()) {
			if (!iparams.shouldRemoveMonitors()) {
				
				// Object.notifyAll():
				if (opcode == INVOKEVIRTUAL && owner.equals("java/lang/Object") && name.equals("notifyAll") && desc.equals("()V")) {
					instrumentationStatsRecord.addSyncPrimitive();
					mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "beforeNotifyAll", "(Ljava/lang/Object;)V");
					return;
				}
		
				// Object.notify():
				if (opcode == INVOKEVIRTUAL && owner.equals("java/lang/Object") && name.equals("notify") && desc.equals("()V")) {
					instrumentationStatsRecord.addSyncPrimitive();
					mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "beforeNotify", "(Ljava/lang/Object;)V");
					return;
				}
			} else {
				// WITHOUT MONITORS
				// Object.notifyAll():
				if (opcode == INVOKEVIRTUAL && owner.equals("java/lang/Object") && name.equals("notifyAll") && desc.equals("()V")) {
					instrumentationStatsRecord.addSyncPrimitive();
					mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "beforeNotifyAllWithoutMonitors", "(Ljava/lang/Object;)V");
					return;
				}
		
				// Object.notify():
				if (opcode == INVOKEVIRTUAL && owner.equals("java/lang/Object") && name.equals("notify") && desc.equals("()V")) {
					instrumentationStatsRecord.addSyncPrimitive();
					mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "beforeNotifyWithoutMonitors", "(Ljava/lang/Object;)V");
					return;
				}
			}
		}
		
		// Any I/O method: store invocation id before every I/O call of a profiled method
		if (owner.startsWith("java/io/") || (owner.startsWith("java/net/") && (name.equals("write") || name.equals("read")))) {
			instrumentationStatsRecord.addIoPoint();
			mv.visitMethodInsn(INVOKESTATIC, "java/lang/Thread", "currentThread", "()Ljava/lang/Thread;");
			mv.visitIntInsn(SIPUSH, IoAdapter.getNextInvocationPointId());
			mv.visitMethodInsn(INVOKEVIRTUAL, "java/lang/Thread", "setVtfIoInvocationPoint", "(I)V");

		}

		// Adapting recursive call to self 
		if (iparams.isRecursiveRetransformation() && iparams.usingExternalInstrumentation()) {
			if (owner.equals(className) && methodName.equals(iparams.getRetransformingMethodName()) && desc.equals(iparams.getRetransformingMethodDesc())) {
				if (DEBUG) {
					System.out.println("Found recursive call " + owner + " --> " + name + ": " + desc + " (" + opcode + ")");
				}				
				
				name = "_vtfmethod_"+ name;
//				if (!name.endsWith("init>")) {
//					name = "_vtfmethod_"+ name;
//				} else {
//					if (name.equals("<init>")) {
//						name = "_vtfmethod_constructor";
//					} else {
//		        		name = "_vtfmethod_class_initialization";
//					}
//				}
			}
		}
				
		if (iparams.shouldModifyTimeCalls()) {
			if ((opcode == INVOKESTATIC && owner.equals("java/lang/System") && name.equals("currentTimeMillis") && desc.equals("()J")) ||
					(opcode == INVOKESTATIC && owner.equals("java/lang/System") && name.equals("nanoTime") && desc.equals("()J"))) {
				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", name, desc);
				return;
			} 
		}
		mv.visitMethodInsn(opcode, owner, name, desc);

	}

	/***
	 * Parse the local variables
	 */
	@Override
	public void visitLocalVariable(String name, String desc, String signature, Label start, Label end, int index) {
		if (removingMethodBody()) {
			return;
		}
		instrumentationStatsRecord.addInstruction();
		super.visitLocalVariable(name, desc, signature, start, end, index);
	}
	
	@Override
	public void visitVarInsn(int opcode, int var) {
		if (removingMethodBody()) {
			return;
		}
		instrumentationStatsRecord.addInstruction();
		super.visitVarInsn(opcode, var);
	}

	@Override
	public void visitIincInsn(int var, int increment) {
		if (removingMethodBody()) {
			return;
		}
		instrumentationStatsRecord.addInstruction();
		super.visitIincInsn(var, increment);
	}

	@Override
	public void visitAttribute(Attribute attr) {
		if (removingMethodBody()) {
			return;
		}
		instrumentationStatsRecord.addInstruction();
		super.visitAttribute(attr);
	}
	
	@Override
	public void visitFieldInsn(int opcode, String owner, String name, String desc) {
		if (removingMethodBody()) {
			return;
		}
		instrumentationStatsRecord.addInstruction();
		super.visitFieldInsn(opcode, owner, name, desc);	
	}
	

	@Override
	public void visitIntInsn(int arg0, int arg1) {
		if (removingMethodBody()) {
			return;
		}
		instrumentationStatsRecord.addInstruction();
		super.visitIntInsn(arg0, arg1);
	}

	@Override
	public void visitJumpInsn(int arg0, Label arg1) {
		if (removingMethodBody()) {
			return;
		}
		instrumentationStatsRecord.addInstruction();
		super.visitJumpInsn(arg0, arg1);
	}

	@Override
	public void visitLabel(Label arg0) {
		if (removingMethodBody()) {
			return;
		}
		
		if (allStartingTryCatchingBlockLabels != null && allStartingTryCatchingBlockLabels.contains(arg0)) {
			++tryCatchDepth;
		} else if (allEndingTryCatchingBlockLabels != null && allEndingTryCatchingBlockLabels.contains(arg0)) {
			--tryCatchDepth;
		}		
		super.visitLabel(arg0);
	}

	@Override
	public void visitLdcInsn(Object arg0) {
		if (removingMethodBody()) {
			return;
		}
		instrumentationStatsRecord.addInstruction();
		super.visitLdcInsn(arg0);
	}

	@Override
	public void visitMultiANewArrayInsn(String arg0, int arg1) {
		if (removingMethodBody()) {
			return;
		}
		instrumentationStatsRecord.addInstruction();
		super.visitMultiANewArrayInsn(arg0, arg1);
		
	}

	@Override
	public void visitTableSwitchInsn(int arg0, int arg1, Label arg2, Label[] arg3) {
		if (removingMethodBody()) {
			return;
		}
		instrumentationStatsRecord.addInstruction();
		super.visitTableSwitchInsn(arg0, arg1, arg2, arg3);
		
	}

	@Override
	public void visitTypeInsn(int arg0, String arg1) {
		if (removingMethodBody()) {
			return;
		}
		instrumentationStatsRecord.addInstruction();
		super.visitTypeInsn(arg0, arg1);
		
	}

	@Override
	public void visitFrame(int arg0, int arg1, Object[] arg2, int arg3, Object[] arg4) {
		if (removingMethodBody()) {
			return;
		}
		super.visitFrame(arg0, arg1, arg2, arg3, arg4);
		
	}
	
	
	private boolean removingMethodBody() {
		return performanceModelParameters != null && performanceModelParameters.removingMethodBody();
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

		instrumentationStatsRecord.addInstruction();
		
		// if (opcode != ATHROW) {
		if (performanceModelParameters != null) {
			if(performanceModelParameters.removingMethodBody()) {
				if ((opcode >= IRETURN && opcode <= RETURN) || (!instrumentExceptionHandlingCode && opcode == ATHROW)) {
					if (profile) {
						onMethodExitInstrumentation();
					}
					mv.visitInsn(opcode);
				}
				return;
			}
			
		} 

		// These are all instructions which terminate a method
		if ((opcode >= IRETURN && opcode <= RETURN) || (!instrumentExceptionHandlingCode && opcode == ATHROW && tryCatchDepth == 0)) {

			if (profile) {
				onMethodExitInstrumentation();
			}

			if (iparams.onlyLimitedJvmtiUsage() && isSynchronised) {
				if (isStatic) {
					mv.visitLdcInsn(Type.getType("L" + className + ";"));
				} else {
					mv.visitVarInsn(ALOAD, 0);
				}
//				mv.visitMethodInsn(INVOKEVIRTUAL, "java/lang/Object", "hashCode", "()I");
				mv.visitMethodInsn(INVOKESTATIC, "java/lang/System", "identityHashCode", "(Ljava/lang/Object;)I");
				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier","_beforeReleasingMonitor","(I)V");
			}
			
			
			if (shouldBeWrappedWithVexOptimizerLoop) {
				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "_exportAndProcessMainIterationResults", "()V");				
				mv.visitJumpInsn(GOTO, startMainWrappedWithVexOptimizer);
				mv.visitLabel(endMainWrappedWithVexOptimizer);
			}
			mv.visitInsn(opcode);
			return;

		}

		
		// Monitor enter instrumentation - suspend thread
		if (opcode == MONITORENTER || opcode == MONITOREXIT) {
			instrumentationStatsRecord.addSyncPrimitive();
			
			if (iparams.onlyLimitedJvmtiUsage()) {
				
				if (!iparams.shouldRemoveMonitors()) {
					mv.visitInsn(DUP);				
				}
				if (opcode == MONITORENTER) {
//					mv.visitMethodInsn(INVOKEVIRTUAL, "java/lang/Object", "hashCode", "()I");
					mv.visitMethodInsn(INVOKESTATIC, "java/lang/System", "identityHashCode", "(Ljava/lang/Object;)I");
					onInteractionPointEncounter();
					mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier","_beforeAcquiringMonitor","(I)V");
					
				} else {
					onInteractionPointEncounter();
					//mv.visitMethodInsn(INVOKEVIRTUAL, "java/lang/Object", "hashCode", "()I");
					mv.visitMethodInsn(INVOKESTATIC, "java/lang/System", "identityHashCode", "(Ljava/lang/Object;)I");
					
					mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier","_beforeReleasingMonitor","(I)V");
					
				}
				
				if (iparams.shouldRemoveMonitors()) {
					return;
				}
				
			}
			
		}
		mv.visitInsn(opcode);
			
	}

	
	private void addTryCatchingBlockLabel(Label label, boolean isStartLabel) {
		if (isStartLabel) {
			if (allStartingTryCatchingBlockLabels == null) {
				allStartingTryCatchingBlockLabels = new Vector<Label>();
			}
			allStartingTryCatchingBlockLabels.add(label);
		} else {
			if (allEndingTryCatchingBlockLabels == null) {
				allEndingTryCatchingBlockLabels = new Vector<Label>();
			}
			allEndingTryCatchingBlockLabels.add(label);		
		}
	}
	
	@Override
	public void visitTryCatchBlock(Label start, Label end, Label handler, String type) {

		if (performanceModelParameters == null || !performanceModelParameters.removingMethodBody()) {
			addTryCatchingBlockLabel(start, true);	// used to identify whether athrow should be instrumented or not based on whether the exception will be caught inside the method or not
			addTryCatchingBlockLabel(end, false);
			
//			System.out.println("visitTryCatchBlock " + type);// + " to " + end.getOffset() + " with handler " + handler.getOffset());	
			mv.visitTryCatchBlock(start, end, handler, type);
		} 		
	}
	

	/***
	 * Before updating maximum stack counter, after instrumentation
	 */
	@Override
	public void visitMaxs(int maxStack, int maxLocals) {

		// Instrument the exception handler for VTF exiting invocation, only if
		// this method may throw exceptions
		Label endExceptionCatchingBlockLabel = null;
		if (instrumentExceptionHandlingCode && (performanceModelParameters == null || !performanceModelParameters.removingMethodBody())) {
			endExceptionCatchingBlockLabel = new Label();
			mv.visitLabel(endExceptionCatchingBlockLabel);

			// Used to print all intermediate exceptions
			// mv.visitInsn(DUP);
			// mv.visitIntInsn(SIPUSH, methodId);
			// mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier","printException","(Ljava/lang/Throwable;I)V");

			onMethodExitInstrumentation();

			mv.visitTryCatchBlock(startExceptionCatchingBlockLabel, endExceptionCatchingBlockLabel, endExceptionCatchingBlockLabel, null);
			mv.visitInsn(ATHROW);

		}
	
		InstrumentationRecordFactory.addRecord(className, methodName, methodDesc, instrumentationStatsRecord);
		mv.visitMaxs(maxStack, maxLocals);		
	}


	/***
	 * Code to be instrumented when a method is entered.
	 */
	private void onMethodEntryInstrumentation() {		
		if (!iparams.usingExternalInstrumentation() || methodName.endsWith("init>")) { 
			if (iparams.methodShouldBeAdapted(methodId)) {
				if (isMethodDescribedByPerformanceModel()) {
					if (!shouldMethodBodyBeRemoved()) {
						// The method body will be executed as normal, but the performance impact will only depend on the provided model
						// This is used to simulate the behaviour, but still get the correct behaviour
						BasicBlockAdapter.insertMethodInstrument(mv, methodId, "_enterPerfModel");
					} else {						
						// No method behaviour, just performance cost added by the model
						BasicBlockAdapter.insertMethodInstrument(mv, methodId, "_enterPerfModel");
					}
				} else {
					BasicBlockAdapter.insertMethodInstrument(mv, methodId, BasicBlockAdapter.vtfMethodEntryCallback);
				}
			}
		} 
	}

	/*
	 * A model describing the performance of a method can either be described through annotation or externally 
	 */
	private boolean isMethodDescribedByPerformanceModel() {
		return (performanceModelParameters != null && performanceModelParameters.usingPerformanceModel()) || isPerformanceDescribedByExternallyDefinedModel();
	}
	private boolean shouldMethodBodyBeRemoved() {
		return (performanceModelParameters != null && performanceModelParameters.removingMethodBody()) || shouldMethodBodyBeRemovedAccordingToExternallyDefinedModel();
	}
	
	
	/***
	 * Code to be instrumented when a method is exited.
	 */
	private void onMethodExitInstrumentation() {
		if (!iparams.usingExternalInstrumentation() || methodName.endsWith("init>")) {
			if (iparams.methodShouldBeAdapted(methodId)) {
				if (isMethodDescribedByPerformanceModel()) {
					if (!shouldMethodBodyBeRemoved()) {
						// Only the case that real code is executed under performance model time will call this
						BasicBlockAdapter.insertMethodInstrument(mv, methodId, "_exitPerfModel");
					} else {
						BasicBlockAdapter.insertMethodInstrument(mv, methodId, "_exitPerfModel");
					}
				} else {
					BasicBlockAdapter.insertMethodInstrument(mv, methodId, BasicBlockAdapter.vtfMethodExitCallback);
					
				}
			}
		}
	}
	
	
	/***
	 * Code to be instrumented when an interaction point is encountered.
	 * 
	 */
	private void onInteractionPointEncounter() {
		if (iparams.trappingInteractionPoints()) { 
			mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "_interactionPoint", "()V");
		}
	}
	
}
