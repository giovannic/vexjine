package vtf_tests.forwardleap.threadtypes;

import java.util.Random;

import vtf_tests.forwardleap.utils.TimeStampPrinter;

public class GcTriggeringThread  implements Runnable {
	private int iterations;

	public GcTriggeringThread() {
		iterations = 4000000;
	}

	public GcTriggeringThread(int iterations) {
		this.iterations = iterations;
	}


	@Override
	public void run() {
		double temp = 0.0;
		for (int i = 0; i<iterations; i++) {
			temp += i;
			for (int j1 = 0; j1 < (2000000+new Random().nextInt()); j1++) {
				temp += new Random().nextDouble();
			}
		}
		TimeStampPrinter.printAbsoluteFinishingTime(temp);
	}

	public static void main(String[] args) {
		int threads = Integer.parseInt(args[0]);
		Thread[] threadsList = new Thread[threads];
		for (int i = 0; i <threads; i++) {
			threadsList[i] = new Thread(new GcTriggeringThread(), "thread" + i);
			threadsList[i].start();
		}
		for (int i = 0; i <threads; i++) {
			try {
			threadsList[i].join();
			} catch (Exception ie) {
			}
		}
		
	}
}
