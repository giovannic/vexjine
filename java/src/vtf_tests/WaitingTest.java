package vtf_tests;

public class WaitingTest implements Runnable {
	private static boolean waiting; 
	public void run() {
		try {
			synchronized(WaitingTest.class) {	
				System.out.println("in run() - waiting for 20 sec");
				WaitingTest.waiting = true;
				WaitingTest.class.wait(20000);
			}
		} catch (InterruptedException x) {
			System.out.println("in run() - interrupted while sleeping");
		}		
		
			
		System.out.println("in run() - leaving normally");
	}

	public static void main(String[] args) {
		long start, end;
		WaitingTest si = new WaitingTest();
		Thread t = new Thread(si, "waitingThread");
		waiting = false;
		start = System.nanoTime();
		t.start();

		try {
			synchronized(WaitingTest.class) {
					System.out.println("in main() - before other thread");
				do {
					// needed to release the monitor
					WaitingTest.class.wait(1500);	
					System.out.println("waited for 1,5 sec");
				} while(!WaitingTest.waiting);
				System.out.println("in main() - interrupting other thread");
				WaitingTest.class.notify();	
			}
		} catch (InterruptedException x) {
			System.out.println("in main - should never be printed");
		}		
		
		System.out.println("in main() - leaving");
		end = System.nanoTime();
		System.out.println("Duration: " + (end-start));
	}
}
