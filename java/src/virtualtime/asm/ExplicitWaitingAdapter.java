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
 * The explicit waiting adapter class is used to 
 * identify non-JVMTI captured waiting points
 */
public class ExplicitWaitingAdapter extends ClassAdapter {

	private String cname; // the instrumented class name
	ClassWriter myVisitor;
	
	public ExplicitWaitingAdapter(ClassVisitor arg0) {
		super(arg0);
		myVisitor = (ClassWriter) arg0;	
	}

	/*
	 * Visitor to the socket related class
	 */
	@Override
	public void visit(int version, int access, String name, String signature,
			String superName, String[] interfaces) {
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
		if (((cname.equals("java/net/ServerSocket")	|| cname.equals("sun/nio/ch/ServerSocketChannelImpl")) && name.equals("accept"))  
			|| cname.equals("java/util/concurrent/locks/LockSupport")
			|| (cname.equals("sun/nio/ch/EPollArrayWrapper") 	&& name.equals("poll"))
			|| (cname.equals("sun/nio/ch/EPollArrayWrapper") 	&& name.equals("interrupt") && desc.equals("()V"))
		) {
			
			//System.out.println("meas ston epollarraywrapper " + cname + " " + name + " " + desc);

			MethodVisitor mv = cv.visitMethod(access, name, desc, signature, exceptions);	            
			return new ExplicitWaitingMethodAdapter(mv, cname, name, desc);
			
//		} else if (cname.equals("sun/nio/ch/Net") && name.equals("connect")) {
//
//			// Create the wrapper method
//			myVisitor.newMethod(cname, name, desc, false);        	
//			myVisitor.visitMethod(access, "__vtf_native_prefix_" + name, desc, signature, exceptions);
//			MethodVisitor mv = myVisitor.visitMethod(access ^ ACC_NATIVE, name, desc, signature, exceptions);
//			
//			mv.visitVarInsn(ALOAD, 0);
//			mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "beforeSocketConnectOnFd", "(Ljava/lang/Object;)V");
//
//			GeneratorAdapter mg = new GeneratorAdapter(mv, access, name, desc);
//			// We know that the call is static
//			mg.loadArgs();
//			mv.visitMethodInsn(INVOKESTATIC, cname, "__vtf_native_prefix_" + name, desc);
//			
//	    	mg.returnValue();
//			mv.visitMaxs(1, 1);
//			return mv;
			
		} else if (cname.equals("sun/misc/Unsafe") && name.endsWith("park")) {	// park and unpark native methods instrumentation
			// Create the wrapper method
//			myVisitor.newMethod(cname, name, desc, false);        	
//			myVisitor.visitMethod(access, "__vtf_native_prefix_" + name, desc, signature, exceptions);
			MethodVisitor mv = myVisitor.visitMethod(access ^ ACC_NATIVE, name, desc, signature, exceptions);
			
//			mv.visitVarInsn(ALOAD, 0);
			GeneratorAdapter mg = new GeneratorAdapter(mv, access, name, desc);
			mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", name, desc);

			
			// We know that the call is static
//			mg.loadArgs();
//			mv.visitMethodInsn(INVOKESTATIC, cname, "__vtf_native_prefix_" + name, desc);
			
	    	mg.returnValue();
			mv.visitMaxs(1, 1);
			return mv;	
	

		} else {
			return cv.visitMethod(access, name, desc, signature, exceptions);
		}
		
    }	
			
}

/* ****
 * 
 * MethodAdapter class ExplicitWaitingMethodAdapter
 * 
 * ExplicitWaitingMethodAdapter is used to instrument 
 * java classes to wrap invocations to (usually native) methods that are 
 * essentially waiting points, but are not trapped by JVMTMI
 * 
 */
class ExplicitWaitingMethodAdapter extends MethodAdapter {

	private String className;
	private String methodName;
	private String methodDesc;

	private static boolean debug = false;

	public ExplicitWaitingMethodAdapter(MethodVisitor arg0, String owner, String name, String desc) {

		super(arg0);

		className = owner;
		methodName = name;
		methodDesc = desc;
	}
	
	
//	private Label startExceptionCatchingBlockLabel ;
//	public void visitMaxs(int maxStack, int maxLocals) {
//		if (className.equals("sun/nio/ch/EPollArrayWrapper") && methodName.equals("poll")) {
//	
//			Label endExceptionCatchingBlockLabel = new Label();
//			mv.visitLabel(endExceptionCatchingBlockLabel);
//			mv.visitTryCatchBlock(startExceptionCatchingBlockLabel, endExceptionCatchingBlockLabel, endExceptionCatchingBlockLabel, null);
//			mv.visitInsn(ICONST_4);
//			mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "resumedSynchronizeOnFdInVirtualTime", "(I)V");
//			mv.visitInsn(ATHROW);
//			
//			mv.visitMaxs(maxStack, maxLocals);
//		} else {	
//			mv.visitMaxs(maxStack, maxLocals);
//		}
//	}

	@Override
	public void visitMethodInsn(int opcode, String owner, String name, String desc) {
		
		// ------------------------- SERVER BIND ------------------------ //
		if (className.equals("java/net/ServerSocket") && methodName.equals("accept") && methodDesc.equals("()Ljava/net/Socket;")) {

			// Hack: Instrumentation of accept to be treated as a waiting
			// operation (since it calls native socket.accept() and does not
			// perform any other operation so that the VTF understands that this is a blocking
			// operation)
			if (debug) {
				System.out.println("serverSocket.accept(): owner " + owner + " name " + name + " desc " + desc);
			}

			if (opcode == INVOKEVIRTUAL && owner.equals("java/net/ServerSocket") && name.equals("implAccept") && desc.equals("(Ljava/net/Socket;)V")) {
				
				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "beforeSocketAccept", "()V");
				mv.visitMethodInsn(opcode, owner, name, desc);
				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "afterSocketAccept", "()V");
				
				return;
			}
				
		} else if (className.equals("sun/nio/ch/ServerSocketChannelImpl") && methodName.equals("accept")) {
			if (debug) {
				System.out.println("ServerSocketChannelImpl.accept(): owner " + owner + " name " + name + " desc " + desc);
			}
			
			if (opcode == INVOKESPECIAL && name.equals("accept0")) {
//				mv.visitVarInsn(ALOAD, 0);
//				mv.visitFieldInsn(GETFIELD, "sun/nio/ch/ServerSocketChannelImpl", "fd" , "Ljava/io/FileDescriptor;");
				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "beforeSocketAccept", "()V");
				mv.visitMethodInsn(opcode, owner, name, desc);
				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "afterSocketAccept", "()V");
				return;
			}
				
		} else if (className.equals("sun/nio/ch/EPollArrayWrapper") && methodName.equals("poll")) {
			
			if (opcode == INVOKESPECIAL && owner.equals("sun/nio/ch/EPollArrayWrapper") && name.equals("epollWait")) {
				
//				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "epollWaiting", "(JIJI)I");
				
				mv.visitInsn(DUP);
				mv.visitVarInsn(LLOAD, 1);
//				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "synchronizeOnFdInVirtualTime", "(IJ)V");
//				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "beforeWrappedTimeoutBlockingCallOn", "(IJ)V");
				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "beforeEpollWaitingOn", "(IJ)V");
				
//				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "setWaiting", "()V");
//				startExceptionCatchingBlockLabel = new Label();
//				mv.visitLabel(startExceptionCatchingBlockLabel);
				mv.visitMethodInsn(opcode, owner, name, desc);
				
//				mv.visitInsn(DUP);
//				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "resumedSynchronizeOnFdInVirtualTime", "(I)V");
//				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "afterAnyWrappedBlockingCall", "()V");
				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "afterEpollWaiting", "()V");
				
//				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "unsetWaiting", "()V");
				return;
			}
		} else if (className.equals("sun/nio/ch/EPollArrayWrapper") && methodName.equals("interrupt") && desc.equals("(I)V")) {	
			if (opcode == INVOKESTATIC) {
//				VTFAdapter.insertPrintMsgWithThread(mv, "interrupting a poll waiting thread");	
			    mv.visitInsn(DUP);
				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "_beforeInterruptingFd", "(I)V");
			}
		} else if (className.equals("java/util/concurrent/locks/LockSupport") && owner.equals("sun/misc/Unsafe") && name.equals("park")) {
			mv.visitInsn(DUP2);					// for the Long timeout value
			if (methodName.equals("parkUntil")) {
				mv.visitInsn(ICONST_1);
			} else {
				mv.visitInsn(ICONST_0);
			}
			mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "vexpark", "(JZ)V");
			mv.visitMethodInsn(opcode, owner, name, desc);
			mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "parked", "()V");
			return;
		} else if (className.equals("java/util/concurrent/locks/LockSupport") && owner.equals("sun/misc/Unsafe") && name.equals("unpark")) {
			
			mv.visitInsn(DUP);				
			mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "vexunpark", desc);
			mv.visitMethodInsn(opcode, owner, name, desc);			
			return;
		}
//		}	else if (className.equals("java/util/concurrent/locks/LockSupport") && owner.equals("sun/misc/Unsafe") && name.endsWith("park")) {
//
//			if (name.equals("park")) {
//				mv.visitInsn(DUP2);					// for the Long timeout value
//				if (methodName.equals("parkUntil")) {
//					mv.visitInsn(ICONST_1);
//				} else {
//					mv.visitInsn(ICONST_0);
//				}
//				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "vexpark", "(JZ)V");
//			} else {
//				mv.visitInsn(DUP);			
//				mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "vexunpark", desc);
//			}
//			mv.visitMethodInsn(opcode, owner, name, desc);
//			return;
//		}	
//		} else if (className.equals("java/util/concurrent/locks/LockSupport") && owner.equals("sun/misc/Unsafe") && name.endsWith("park")) {
//			mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", "vex" + name, desc);
//			return;
//
//		// REPLACEMENT OF PARK AND UNPARK
////		} else if (className.equals("java/util/concurrent/locks/LockSupport") && owner.equals("sun/misc/Unsafe") && name.endsWith("park")) {
////			// park and unpark instrumentation
////			// replacement of park is quite dangerous
////			mv.visitMethodInsn(INVOKESTATIC, "virtualtime/EventNotifier", name, desc);
////			return;
//		} else if (className.equals("java/util/concurrent/locks/LockSupport") && owner.equals("sun/misc/Unsafe") && name.endsWith("putObject")) {	// park and unpark native methods instrumentation
//			// remove invocation
//			return ;
//		}

		mv.visitMethodInsn(opcode, owner, name, desc);
	}

}
