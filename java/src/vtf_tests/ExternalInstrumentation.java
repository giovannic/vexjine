package vtf_tests;

public class ExternalInstrumentation {
	private int i;
	public static long startTime;
	public static long endTime;
	public static long diffTime;

	static {
		startTime = System.nanoTime();
	}
	
//	ExternalInstrumentation() {
//		i = 0;
//	}
	
	ExternalInstrumentation(int l) {
		i = l;
	}
	
//	long addTime() {
//		return endTime - startTime + i;
//	}
//	
//	long reduceTime() {
//		return endTime - startTime - i;
//	}
//	
//	synchronized long getTime() {
//		return endTime - startTime;
//	}
//	
//	long myFactorial(long x) {
//		if (x <= 1) {
//			return 1;
//		} else {
//			return myFactorial(x - 1) * x;
//		}
//	}
//	
	public static void main(String[] args) {
				
		long sum = 0;
		for (int i = 0; i<100000; i++) {
			ExternalInstrumentation ext = new ExternalInstrumentation(i);
//			sum += ext.addTime();
		}
		System.out.println("crazy sum: " + sum);
//		System.out.println("5! = " + new ExternalInstrumentation().myFactorial(5));
//		System.out.println("10! = " + new ExternalInstrumentation().myFactorial(10));
		
	}
	
	static {
		endTime = System.nanoTime();
		diffTime = endTime - startTime;
	}
	
}
