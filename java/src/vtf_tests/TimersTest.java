package vtf_tests;

public class TimersTest {
	public static void main(String[] args) {	
		double temp = 0;
		int iterations = Integer.parseInt(args[0]); 
		long startn = System.nanoTime();
		long start = System.currentTimeMillis();
		for (int i =0 ; i <iterations; i++) {
			temp += i;
		}
		long endn = System.nanoTime();
		long end = System.currentTimeMillis();
		System.out.println(temp + ": " + start + " " + end + " = " + (end-start) + " (" + (endn-startn)/1000000.0 + ")");
	}
}
