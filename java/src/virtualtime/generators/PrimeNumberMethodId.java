package virtualtime.generators;

public class PrimeNumberMethodId extends MethodId {

	private PrimeNumberGenerator primeNumberGenerator;
	
	public PrimeNumberMethodId() {
		primeNumberGenerator = new PrimeNumberGenerator();
		methodId = primeNumberGenerator.getCurrentPrime();
	}
	
	@Override
	public int getNextMethodId() {
		int method = methodId;
		methodId = primeNumberGenerator.getNext();
		return method;
	}

	@Override
	public void rollbackToMethodId(int method0) {
		previousMethodId = methodId;
		primeNumberGenerator.setCurrentPointerTo(method0);
		methodId = primeNumberGenerator.getCurrentPrime();
	}

	@Override
	public void returnToCurrentMethodId() {
		primeNumberGenerator.clearCurrentPointer();
		methodId = primeNumberGenerator.getCurrentPrime();
	}
	
	@Override
	public int getNextMethodId(String className, String methodName,	String methodDesc) {
		getNextMethodId();
		return 0;
	}
	
}
