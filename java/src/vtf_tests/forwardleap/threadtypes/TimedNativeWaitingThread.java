package vtf_tests.forwardleap.threadtypes;

import vtf_tests.forwardleap.utils.NativeWaitingSimulation;
import vtf_tests.forwardleap.utils.TimeStampPrinter;

public class TimedNativeWaitingThread implements Runnable {
	static {
		System.loadLibrary("nativesim");
	}
	
	public TimedNativeWaitingThread() {
		
	}
	
	@Override
	public void run() {
		NativeWaitingSimulation.waitManyTimes();
		TimeStampPrinter.printAbsoluteFinishingTime("NW-multiple");
	}
}
