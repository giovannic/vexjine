package virtualtime;

public class PerformanceModelStub {
	
	public PerformanceModelStub() {
		this.usePerformanceModel = false;
		this.removeMethodBody = false;
		this.returnValue = null;
	}

	private boolean usePerformanceModel;
	private boolean removeMethodBody;
	private Object returnValue;
	
	public boolean usingPerformanceModel() {
		return usePerformanceModel;
	}

	public void usePerformanceModel() {
		this.usePerformanceModel = true;
	}
	
	public Object getReturnValue() {
		return returnValue;
	}
	
	public void setReturnValue(Object value) {
		returnValue = value;
	}
	
	public boolean removingMethodBody() {
		return removeMethodBody;
	}

	public void removeMethodBody() {
		this.removeMethodBody = true;
	}
}
