package virtualtime;

public class MethodAdaptationInfo {
	private String methodFQN;
	private double speedup;
	private String modelFilename;
	private boolean replaceBody;
	
	MethodAdaptationInfo(String _methodFQN, double _speedup) {
		methodFQN = _methodFQN;
		speedup   = _speedup;
		modelFilename = null;
		replaceBody = false;
	}

	MethodAdaptationInfo(String _methodFQN, String _modelFilename, boolean _replaceBody) {
		methodFQN = _methodFQN;
		speedup   = 1.0;
		modelFilename = _modelFilename;
		replaceBody = _replaceBody;
	}
	
	public double getFactor() {
		return speedup;
	}
	
	public String getMethod() {
		return methodFQN;
	}
	
	public String getModelFilename() {
		return modelFilename;
	}
	
	public boolean shouldMethodBodyBeReplaced() {
		return replaceBody;
	}
	
	public boolean isPerformanceDescribedByExternallyDefinedModel() {
		return modelFilename != null;
	}
	
}
