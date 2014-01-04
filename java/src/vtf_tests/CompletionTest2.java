package vtf_tests;

public class CompletionTest2 {

	private final static int result = 1349063937;
	
	public static void main(String[] args) {
		
		long iterations = 15000000000L;
		
		long start = System.nanoTime();
		int myresult = 1;
		for (long i = 1; i<iterations; i++) {
			myresult += i;
		}
		long end = System.nanoTime();
		
		if (result == myresult) {
			System.out.println("Score: " + (end-start)/1e9 + " sec");
		} else {
			System.out.println("Score: NOT valid result " + myresult + " after " + (end-start)/1e9 + " sec");
		}
			
	}
}
