package vtf_tests.forwardleap.utils;

public class TimeStampPrinter {
	public static final long startingTime;
	static {
		startingTime = System.nanoTime();	
	}
	
	public static final String myMessageBody = " finished at ";
	public static void printAbsoluteFinishingTime() {
		System.out.println("Operation" + myMessageBody + System.nanoTime()/1e9);
	}
	
	public static void printAbsoluteFinishingTime(String operation) {
		System.out.println(operation + myMessageBody + System.nanoTime()/1e9);
	}

	public static void printAbsoluteFinishingTime(double result) {
		System.out.println(result + myMessageBody + System.nanoTime()/1e9);
	}
	
	public static void printRelativeFinishingTime() {
		System.out.println("Operation" + myMessageBody + (System.nanoTime() - startingTime)/1e9);
	}
	
	public static void printRelativeFinishingTime(String operation) {
		System.out.println(operation + myMessageBody + (System.nanoTime() - startingTime)/1e9);
	}

	public static void printRelativeFinishingTime(double result) {
		System.out.println(result + myMessageBody + (System.nanoTime() - startingTime)/1e9);
	}
}
