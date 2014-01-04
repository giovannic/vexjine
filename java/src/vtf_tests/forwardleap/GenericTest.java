package vtf_tests.forwardleap;

import vtf_tests.forwardleap.threadtypes.BlockingIoThread;
import vtf_tests.forwardleap.threadtypes.CpuRunningThread;
import vtf_tests.forwardleap.threadtypes.GcTriggeringThread;
import vtf_tests.forwardleap.threadtypes.IoThread;
import vtf_tests.forwardleap.threadtypes.NativeWaitingThread;
import vtf_tests.forwardleap.threadtypes.RealTimeWaitingThread;
import vtf_tests.forwardleap.threadtypes.SleepingThread;
import vtf_tests.forwardleap.threadtypes.TimedNativeWaitingThread;
import vtf_tests.forwardleap.threadtypes.UserInputIoThread;
import vtf_tests.forwardleap.threadtypes.WaitingThread;

public class GenericTest {

	public static void genericTest(String[] threadTypes) {
		int threads = 0;
		for (int i = 0; i<threadTypes.length; i++) {
			try {
				threads += Integer.parseInt(threadTypes[i]);
			} catch (NumberFormatException ne) {
				threads += 0;
			}
		}
		
		if (threads <= 0) {
			System.out.println("Generic test order: <Short Waiting> <Long Waiting> <Short Sleeping> <Long Sleeping> <Cpu> <Io> <GC> <BlockIo> <RT waiting> <UserInput> <NW> <Timed-NW>");
			System.out.println("                                                                             w W s S C I G B R U N n");
			return;
		}
		
		Thread[] testThreads = new Thread[threads];
		int count = 0;
		for (int i = 0; i<threadTypes.length; i++) {
			try {
				int threadsOfNextCategory = Integer.parseInt(threadTypes[i]);
				for (int k = 0; k<threadsOfNextCategory; k++) {
					switch (i) {
						case 0:
							testThreads[count] = new Thread(new WaitingThread(), "WaitingThread" + k);
							break;
						case 1:
							testThreads[count] = new Thread(new WaitingThread(10000, 1), "WaitingThread" + k);
							break;
						case 2:
							testThreads[count] = new Thread(new SleepingThread(), "SleepingThread" + k);
							break;
						case 3:
							testThreads[count] = new Thread(new SleepingThread(10000, 1), "SleepingThread" + k);
							break;
						case 4:
							testThreads[count] = new Thread(new CpuRunningThread(), "CpuThread" + k);
							break;
						case 5:
							testThreads[count] = new Thread(new IoThread(), "IoThread" + k);
							break;							
						case 6:
							testThreads[count] = new Thread(new GcTriggeringThread(), "GcTriggeringThread" + k);
							break;
						case 7:
							testThreads[count] = new Thread(new BlockingIoThread(), "BlockIoThread" + k);
							testThreads[count].setDaemon(true);
							break;
						case 8:
							testThreads[count] = new Thread(new RealTimeWaitingThread(), "RealTimeWaitingThread" + k);
							testThreads[count].setDaemon(true);
							break;
						case 9:
							testThreads[count] = new Thread(new UserInputIoThread(), "UserInputIoThread" + k);
							testThreads[count].setDaemon(true);
							break;
						case 10:
							testThreads[count] = new Thread(new NativeWaitingThread(), "NativeWaitingThread" + k);
							break;
						case 11:
							testThreads[count] = new Thread(new TimedNativeWaitingThread(), "TimedNativeWaitingThread" + k);
							break;
						default:
							break;
					}
					++count; 
				}
				
			} catch (NumberFormatException ne) {
				ne.printStackTrace();
			}
		}
		
		
		long startTime = System.nanoTime();
		for (int i = 0; i<threads; i++) {
			testThreads[i].start();
		}
		
		for (int i = 0; i<threads; i++) {
			if (!testThreads[i].isDaemon()) {
				try {
					testThreads[i].join();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
		long totalTime = System.nanoTime() - startTime;
		System.out.println((double)totalTime/1e9);
	}
	
	
	public static void main(String[] args) {
		genericTest(args);
	}
}
