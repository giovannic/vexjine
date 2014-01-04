package vtf_tests.forwardleap.utils;

public class NativeWaitingSimulation {

	public static native void waitOnceForLong();
	private static native void waitShort();
	public static native void wait200ms();
	private static long getTime() {
		return System.nanoTime();
	}
	public static void waitManyTimes() {
		long startTime = getTime();
		long totalTime = 0;
		for (int i = 0 ; i < 6; i++) {
			waitShort();
			long intermediateTime = getTime();
			totalTime += (intermediateTime - startTime);
			startTime = intermediateTime;
		}
		System.out.println("total waiting time " + totalTime/1e9);
	}
	
}
