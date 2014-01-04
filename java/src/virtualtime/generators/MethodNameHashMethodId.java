package virtualtime.generators;

public class MethodNameHashMethodId extends MethodId {

	public MethodNameHashMethodId() {
		methodId = 110;
	}
	
	@Override
	public int getNextMethodId() {
		return 0;
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
		String s = className+methodName+methodDesc;
//		System.out.println(s + "<<<< TO >>>>" + s.hashCode());
		return Math.abs(s.hashCode());
	}
}
