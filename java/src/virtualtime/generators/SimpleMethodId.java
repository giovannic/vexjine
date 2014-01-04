package virtualtime.generators;

public class SimpleMethodId extends MethodId {

	public SimpleMethodId() {
		methodId = 110;
	}
	
	@Override
	public int getNextMethodId() {
		return methodId++;
	}

	@Override
	public void rollbackToMethodId(int method0) {
		previousMethodId = methodId;
		methodId = method0;
	}

	@Override
	public void returnToCurrentMethodId() {
		methodId = previousMethodId;
	}

	@Override
	public int getNextMethodId(String className, String methodName,	String methodDesc) {
		return getNextMethodId();
//		return 0;
	}
	
}
