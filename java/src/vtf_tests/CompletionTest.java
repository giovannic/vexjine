package vtf_tests;

public class CompletionTest {

	//private final static double result = 28096.41467190507; 
	private final static double result = 2.447549998844879E19;
	public static void main(String[] args) {
		
		double iterations = 2100000000;
		//double iterations = Integer.parseInt(args[0]);
		
		long start = System.nanoTime();
		double myresult = 1.0;
//		boolean flag = true;
		for (double i = 1; i<iterations; i++) {
//			if (flag) {
				myresult += 11.1*i;			
//			} else {
//				myresult += i;
//			}
//			flag ^= true;
			if (myresult == 0) {
				System.out.println("Failed at iteration " + i);
				break;
			}
		}

		long end = System.nanoTime();
		
		if (result == myresult) {
			System.out.println("Score: " + (end-start)/1e9 + " sec");
		} else {
			System.out.println("Score: NOT valid result " + myresult + " after " + (end-start)/1e9 + " sec");
		}
			
	}
}
