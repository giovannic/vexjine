package virtualtime.asm;

import org.objectweb.asm.AnnotationVisitor
;
import virtualtime.EventNotifier;
import virtualtime.PerformanceModelStub;

/*
 * AccelerateAnnotationVisitor class
 * 
 * Called when a method's annotation denotes a virtual time specification This
 * specification (speedup factor) is then registered for this methodId to the
 * VTF agent
 */
public class ModelPerformanceAnnotationVisitor implements AnnotationVisitor, VirtualTime_CritialSection {

	private int methodId;
	private PerformanceModelStub performanceModelParameters;
	
	public ModelPerformanceAnnotationVisitor(String _className, int _methodId, PerformanceModelStub performanceModelParameters) {
		methodId = _methodId;
		this.performanceModelParameters = performanceModelParameters; 
		sourceNodeLabel = null;
		customerClass = 0;
		jmtModelFilename = null;
	}

	private double speedup;
	private String jmtModelFilename;
	private String sourceNodeLabel;
	private int customerClass;
	
	public double getSpeedup() {
		return speedup;
	}

	@Override
	public void visit(String name, Object value) {
		if ("jmtModelFilename".equals(name)) {
			jmtModelFilename = (String)value;
			performanceModelParameters.usePerformanceModel();
			
		} else if ("replaceMethodBody".equals(name)) {
			if (Boolean.valueOf(value.toString())) {
				performanceModelParameters.removeMethodBody();
			}
		} else if ("returnValue".equals(name)) {
			performanceModelParameters.setReturnValue(value);
			
		} else if ("sourceNodeLabel".equals(name)) {
			sourceNodeLabel = (String)value;
			
		} else if ("customerClass".equals(name)) {
			customerClass = Integer.parseInt(value.toString());
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
		EventNotifier.registerMethodPerformanceModel(methodId, jmtModelFilename, customerClass, sourceNodeLabel);
	}

}