package virtualtime.asm;

import org.objectweb.asm.AnnotationVisitor;
import virtualtime.EventNotifier;

/*
 * AccelerateAnnotationVisitor class
 * 
 * Called when a method's annotation denotes a virtual time specification This
 * specification (speedup factor) is then registered for this methodId to the
 * VTF agent
 */
public class AccelerateAnnotationVisitor implements AnnotationVisitor,
VirtualTime_CritialSection {

	private int methodId;

	public AccelerateAnnotationVisitor(String _className, int _methodId) {
		methodId = _methodId;
	}

	private double speedup;

	public double getSpeedup() {
		return speedup;
	}

	@Override
	public void visit(String name, Object value) {
		if ("speedup".equals(name)) {
			speedup = (Double) value;
		}
	}

	@Override
	public AnnotationVisitor visitAnnotation(String arg0, String arg1) {
		return null;
	}

	@Override
	public AnnotationVisitor visitArray(String arg0) {
		return null;
	}

	@Override
	public void visitEnd() {
	}

	@Override
	public void visitEnum(String arg0, String arg1, String arg2) {
	}

	@Override
	public void register() {
		EventNotifier.registerMethodVirtualTime(methodId, speedup);
	}

}