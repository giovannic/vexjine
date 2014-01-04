/*

 * Created on May 28, 2008
 *
 * Changelog for Java instrumentation library
 * 
 * InternalAdapter & AccelerateAnnotationVisitor for basic VTF instrumentation (from prev versions)
 * VTF2.0: NativeIOMethodAdapter for static I/O instrumentation
 * VTF2.8: SocketMethodAdapter for internal socket instrumentation
 * VTF3.0: 
 * - added shouldTransformWaits flag in InternalAdapter to denote, whether waits should be replaced (used for real time Visualizer)
 * - Static instrumentation of java/lang/Thread class
 * 		-> adding vtfInvocation points to filter only points of interest
 * 		-> setting all join methods to synchronized... (required but?)    
 * 		-> identification with ids
 */
package virtualtime.asm;

import virtualtime.EventNotifier;
import virtualtime.InstrumentationParameters;

import static org.objectweb.asm.Opcodes.*;

import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.ClassWriter;
import org.objectweb.asm.Label;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Type;
import org.objectweb.asm.commons.GeneratorAdapter;


/*
 * The basic adapter class
 * 
 * Identifies the kind of instrumentation (dynamic on runtime or static for system classes) for each method and
 * creates an instance of the appropriate MethodAdapter class.
 * 
 * Constructor parameters:
 * @param: ClassVisitor arg0 					-> the classVisitor instance for this class
 * @param: boolean profile						-> used for debugging to disable all dynamic profiling
 * @param: Vector<String> _profiledMethodNames	-> the FQN of the methods that should be profiled from this class (enabling selective profiling)
 * @param: boolean _shouldTransformWaits		-> used for debugging to instrument classes but disable Object.wait special handling
 * @param: boolean _isProfilingRun 				-> used to opt-out the synchronized substitution when we are performing a profiling run
 */
public class ExternalWrappingAdapter extends VTFAdapter {

	public ExternalWrappingAdapter(ClassVisitor arg0, boolean profile, InstrumentationParameters _iparams) {
		super(arg0);
		myVisitor = (ClassWriter) arg0;
		profiling = profile;

		iparams = _iparams;
		
		if (!iparams.isRetransformingInstrumentation()) {
			iparams.setMethod0(methodIdGenerator.getCurrentMethodId());
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
			System.out.println("Visiting method: '" + name+ "' '" + desc + "' '" + signature + "' ");
		}
		
        boolean isSynchronized = (access & ACC_SYNCHRONIZED) != 0;
        boolean isStatic = (access & ACC_STATIC) != 0;
        boolean isNative = (access & ACC_NATIVE) != 0;
        int newAccess = access;
        if (isSynchronized) {
        	newAccess = access ^ ACC_SYNCHRONIZED;
        }
               
		// The remaining methods will be tested inside the BasicBlockMethodAdapter class -- 0000000000000000000000        
		MethodVisitor mv = null;
        boolean isAbstract = (access & ACC_ABSTRACT) != 0;
        if (isNative || isAbstract) {
        	return cv.visitMethod(access, name, desc, signature, exceptions);        	
        }

        int methodId = methodIdGenerator.getNextMethodId(cname, name, desc); //methodIdGenerator.getCurrentMethodId();
		if (iparams.isRetransformingInstrumentation() && (iparams.isInvalidatedMethod(methodId) || iparams.isRecursiveMethod(methodId))) {
						
			if (isSynchronized) {				
				mv = wrapSynchronizedMethod(access, name, desc, signature, exceptions, false);        	

	        } else if (!name.endsWith("init>")) {		// excluding <init> and <clinit>
	        	
	        	// When we are reloading a class to remove the VEX instruments,
	        	// we still have to define the _vtf... methods, because otherwise
	        	// an exception with message "attempted to change the schema (add/remove fields)" is thrown
	        	MethodVisitor mv2 = null;
//	        	if (name.equals("<init>")) {		// overloaded constructors will defer from the "desc"
//	        		myVisitor.newMethod(cname, "_vtfmethod_constructor", desc, false);
//	        		mv2 = myVisitor.visitMethod(access, "_vtfmethod_constructor", desc, signature, exceptions);
//	        	} else if (name.equals("<clinit>")) { // all static{} class initializers are aggregated into a single method	
//	        		myVisitor.newMethod(cname, "_vtfmethod_class_initialization", desc, false);
//	        		mv2 = myVisitor.visitMethod(access, "_vtfmethod_class_initialization", desc, signature, exceptions);
//	        	} else {
	        	
        		myVisitor.newMethod(cname, "_vtfmethod_" + name, desc, false);
        		mv2 = myVisitor.visitMethod(access, "_vtfmethod_" + name, desc, signature, exceptions);
        		
//	        	}
				GeneratorAdapter mg = new GeneratorAdapter(mv2, newAccess, name, desc);
				if (isStatic) {
					mg.loadArgs();
				} else {
					mv2.visitVarInsn(ALOAD, 0);
					mg.loadArgs();
				}
	  
				if (isStatic) {
					mv2.visitMethodInsn(INVOKESTATIC ,cname, name, desc);				
				} else {
					mv2.visitMethodInsn(INVOKESPECIAL ,cname, name, desc);
				}
            	mg.returnValue();
	        	mv2.visitMaxs(0,0);
	        	
	        	// the original uninstrumented method
	        	mv = cv.visitMethod(access, name, desc, signature, exceptions);
	        	
	        } else {
	        	mv = cv.visitMethod(access, name, desc, signature, exceptions);
	        }

			
		} else {
			
			EventNotifier.registerMethod(cname + " " + name + " " + desc, methodId, iparams.getMethod0());
			
	        if (isSynchronized) {
				mv = wrapSynchronizedMethod(access, name, desc, signature, exceptions, true);        	    	
	
	        } else if (!name.endsWith("init>")) {		// excluding <init> and <clinit>
	        	
	        	myVisitor.newMethod(cname, name, desc, false);
	        	MethodVisitor mv2 = myVisitor.visitMethod(newAccess, name, desc, signature, exceptions);
				GeneratorAdapter mg = new GeneratorAdapter(mv2, newAccess, name, desc);
				
				if (isStatic) {
					mg.loadArgs();
				} else {
					mv2.visitVarInsn(ALOAD, 0);
					mg.loadArgs();
				}
				
				
				// Add the monitorFlag by one - to show that we are now in a VTF
				// monitored method
//				mv2.visitIntInsn(SIPUSH, methodId);
	
				Label startExceptionCatchingBlockLabel = null;
				if (exceptions != null && exceptions.length > 0) {
					startExceptionCatchingBlockLabel = new Label();
					mv2.visitLabel(startExceptionCatchingBlockLabel);
				}
			
				VTFAdapter.insertMethodInstrument(mv2, methodId, vtfMethodEntryCallback);
				
//	            if (name.equals("<init>")) {
//	            	mv2.visitMethodInsn(INVOKESPECIAL ,cname, "_vtfmethod_constructor", desc);
//	            } else if (name.equals("<clinit>")) { 	
//	        		mv2.visitMethodInsn(INVOKESPECIAL ,cname,", desc);	
//	            } else {
				
				if (isStatic) {
					mv2.visitMethodInsn(INVOKESTATIC ,cname, "_vtfmethod_"+name, desc);				
				} else {
					mv2.visitMethodInsn(INVOKESPECIAL ,cname, "_vtfmethod_"+name, desc);
				}
//	            }
	      
	
				VTFAdapter.insertMethodInstrument(mv2, methodId, vtfMethodExitCallback);			
				
	        	mg.returnValue();
	        	
	        	if (exceptions != null && exceptions.length > 0) {
		        	Label endExceptionCatchingBlockLabel = new Label();
		    		mv2.visitLabel(endExceptionCatchingBlockLabel);
		     		mv2.visitTryCatchBlock(startExceptionCatchingBlockLabel, endExceptionCatchingBlockLabel, endExceptionCatchingBlockLabel, null);
		    		mv2.visitInsn(ATHROW);  	
	        	}
	        	      	
	        	
	        	mv2.visitMaxs(0,0);
	        				
	//			}
	
//	            if (name.equals("<init>")) {
//	            	mv = cv.visitMethod(access, "_vtfmethod_constructor", desc, signature, exceptions);
//	        	} else if (name.equals("<clinit>")) { 	
//	        		mv = cv.visitMethod(access, "_vtfmethod_class_initialization", desc, signature, exceptions);	            	
//	            } else {
	           	mv = cv.visitMethod(access, "_vtfmethod_"+name, desc, signature, exceptions);
//	            }
	        } else {
	        	
	        	mv = cv.visitMethod(newAccess, name, desc, signature, exceptions);
				int method=0; 
				method = methodIdGenerator.getNextMethodId(cname, name, desc);
				
				// Get externally provided adaptation info
				adaptationInfo = iparams.registerExternallyProvidedMethodPerformanceFactors(method, cname + name);
				
				// Instrument all - profile only selected methods
				mv = new BasicBlockMethodAdapter(mv, method, true, access, cname, name, desc, signature, exceptions,  iparams, adaptationInfo);
				return mv; 
	        }		
					
		}
		
        return callBasicBlockAdapter(mv, access, name, desc, signature, exceptions);
	}	

	
	
	
	/******
	 * Method used to set dummy methods that just return a type - they are not meant to be run - just to fool the verifier
	 */
  	void setDummyMethods2(MethodVisitor mv2, String returnType, boolean isStatic) { 
    	if (returnType.startsWith("L")) {
    		if (isStatic) {
    			mv2.visitMethodInsn(INVOKESTATIC , "virtualtime/EventNotifier", "getRandomObject", "()Ljava/lang/Object;");
    			mv2.visitTypeInsn(CHECKCAST, returnType.substring(1, returnType.length()-1));
    			
    		} else {
    			mv2.visitVarInsn(ALOAD, 0);	
    		}
    		mv2.visitInsn(ARETURN);
    	} else if (returnType.startsWith("[")) {
    		mv2.visitMethodInsn(INVOKESTATIC , "virtualtime/EventNotifier", "getRandomObject", "()Ljava/lang/Object;");
			mv2.visitTypeInsn(CHECKCAST, returnType);
			mv2.visitInsn(ARETURN);
		
    	} else if (returnType.startsWith("D")) {
    		mv2.visitInsn(DCONST_0);
    		mv2.visitInsn(DRETURN);
    		
    	} else if (returnType.startsWith("F")) {
    		mv2.visitInsn(FCONST_0);
    		mv2.visitInsn(FRETURN);
    		
    	} else if (returnType.startsWith("S") || returnType.startsWith("I") || returnType.startsWith("Z") || returnType.startsWith("B") || returnType.startsWith("C")) {
    		mv2.visitInsn(ICONST_0);
    		mv2.visitInsn(IRETURN);
    		
    	} else if (returnType.startsWith("J")) {
    		mv2.visitInsn(LCONST_0);
    		mv2.visitInsn(LRETURN);
    		
    	} else if (returnType.startsWith("V")) {
    		mv2.visitInsn(RETURN);
    	}
	}
	
	
	/******
	 * Method used to set dummy methods that just return a type - they are not meant to be run - just to fool the verifier
	 */
  	void setDummyMethods(MethodVisitor mv2, String returnType, boolean isStatic) { 
    	if (returnType.startsWith("L")) {
    		if (isStatic) {
    			mv2.visitMethodInsn(INVOKESTATIC , "virtualtime/EventNotifier", "getRandomObject", "()Ljava/lang/Object;");
    			mv2.visitTypeInsn(CHECKCAST, returnType.substring(1, returnType.length()-1));
    			
    		} else {
    			mv2.visitVarInsn(ALOAD, 0);	
    		}
    		mv2.visitInsn(ARETURN);
    	} else if (returnType.startsWith("[")) {
    		mv2.visitMethodInsn(INVOKESTATIC , "virtualtime/EventNotifier", "getRandomObject", "()Ljava/lang/Object;");
			mv2.visitTypeInsn(CHECKCAST, returnType);
			mv2.visitInsn(ARETURN);
		
    	} else if (returnType.startsWith("D")) {
    		mv2.visitInsn(DCONST_0);
    		mv2.visitInsn(DRETURN);
    		
    	} else if (returnType.startsWith("F")) {
    		mv2.visitInsn(FCONST_0);
    		mv2.visitInsn(FRETURN);
    		
    	} else if (returnType.startsWith("S") || returnType.startsWith("I") || returnType.startsWith("Z") || returnType.startsWith("B") || returnType.startsWith("C")) {
    		mv2.visitInsn(ICONST_0);
    		mv2.visitInsn(IRETURN);
    		
    	} else if (returnType.startsWith("J")) {
    		mv2.visitInsn(LCONST_0);
    		mv2.visitInsn(LRETURN);
    		
    	} else if (returnType.startsWith("V")) {
    		mv2.visitInsn(RETURN);
    	}
	}
}
