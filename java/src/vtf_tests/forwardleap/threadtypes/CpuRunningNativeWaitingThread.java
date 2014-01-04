package vtf_tests.forwardleap.threadtypes;

import vtf_tests.forwardleap.utils.NativeWaitingSimulation;
import vtf_tests.forwardleap.utils.TimeStampPrinter;

public class CpuRunningNativeWaitingThread extends CpuRunningThread {
	static {
		System.loadLibrary("nativesim");
	}
	
	public CpuRunningNativeWaitingThread(int iterations) {
		super(iterations);
	}
	
	@Override
	public void run() {
		NativeWaitingSimulation.wait200ms();
		super.run();
	}
}
