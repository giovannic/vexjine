package vtf_tests;

public class SleepTester implements Runnable {
	public void run() {
		try {
			System.out.println("in run() - sleep for 20 seconds");
			Thread.sleep(20000);
			System.out.println("in run() - woke up");
		} catch (InterruptedException x) {
			System.out.println("in run() - interrupted while sleeping");
			return;
		}
		System.out.println("in run() - leaving normally");
	}

	public static void main(String[] args) {
		long start, end;
		SleepTester si = new SleepTester();
		Thread t = new Thread(si);
		
		start = System.nanoTime();
		//t.start();

		// Be sure that the new thread gets a chance to
		// run for a while.
		try {
			Thread.sleep(2000);
			System.out.println("in main() - interrupting after 2sec");
		} catch (InterruptedException x) {
		}

		//System.out.println("in main() - interrupting other thread");
//		t.interrupt();
		System.out.println("in main() - leaving");
		end = System.nanoTime();
		System.out.println("Duration: " + (end-start));
	}

}
