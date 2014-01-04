package vtf_tests;

public class TestUtils {
	
	public static boolean checkRatio(double d1, double d2, double limit) {
		double error_ratio = (double)Math.abs(d1-d2) / (double)d1;
		if (error_ratio < limit) {
			return true;
		} else {
			//System.out.print("... " + Double.toString(d1).substring(0,4) + " " + Double.toString(d2).substring(0,4) + "->" + Double.toString(error_ratio).substring(0,5) + "...");
			System.out.println("... " + Double.toString(d1) + " " + Double.toString(d2)+ "->" + Double.toString(error_ratio) + "...");
			return false;
		}
	}
	
	public static boolean checkRatio(long d1, long d2, double limit) {
		double error_ratio = (double)Math.abs(d1-d2) / (double)d1;
		if (error_ratio < limit) {
			return true;
		} else {
			//System.out.print("... " + Double.toString(d1).substring(0,4) + " " + Double.toString(d2).substring(0,4) + "->" + Double.toString(error_ratio).substring(0,5) + "...");
			System.out.println("... " + Double.toString(d1) + " " + Double.toString(d2)+ "->" + Double.toString(error_ratio) + "...");
			return false;
		}
	}
}
