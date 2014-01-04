package vtf_tests.forwardleap.threadtypes;

import vtf_tests.forwardleap.utils.NativeWaitingSimulation;
import vtf_tests.forwardleap.utils.TimeStampPrinter;

public class NativeWaitingThread implements Runnable {
	static {
		System.loadLibrary("nativesim");
	}
	
	public NativeWaitingThread() {
		
	}
	
	@Override
	public void run() {
		NativeWaitingSimulation.waitOnceForLong();
		TimeStampPrinter.printAbsoluteFinishingTime("NW");
	}
}
