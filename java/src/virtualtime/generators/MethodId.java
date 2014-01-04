package virtualtime.generators;

public abstract class MethodId {
	
	protected int methodId;
	protected int previousMethodId;
	
	public abstract int getNextMethodId();
	public abstract int getNextMethodId(String className, String methodName, String methodDesc);
	public abstract void rollbackToMethodId(int method0);
	public abstract void returnToCurrentMethodId();
	
	public int getCurrentMethodId() {
		return methodId;
	}
}
