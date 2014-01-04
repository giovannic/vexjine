package customadapter;

public class MethodAdaptationInfo {
	private String methodFQN;
	private double speedup;
	
	MethodAdaptationInfo(String _methodFQN, double _speedup) {
		methodFQN = _methodFQN;
		speedup   = _speedup;
	}
	
	public double getFactor() {
		return speedup;
	}
	
	public String getMethod() {
		return methodFQN;
	}
	
}
