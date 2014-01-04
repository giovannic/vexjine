package vtf_tests;

public class JoinTest {
	public double temp = 1.0;
	public boolean testTimeOut = false;
	public class RunningThread implements Runnable {

		@Override
		public void run() {
			for (int i = 1; i<100000; i++) {
				temp += Math.pow(1.0 + 1.0/i, i);				
			}
			
		}
		
	}

	public class TestTimerThread implements Runnable {

		@Override
		public void run() {
			try {
				Thread.sleep(7000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			testTimeOut = true;
		}
		
	}
	public static void main(String[] args) {
		int threads = Integer.parseInt(args[0]);
		Thread[] thread = new Thread[threads];
		JoinTest j = new JoinTest();
		Thread timer = new Thread(j.new TestTimerThread(), "TestTimer");
		int round = 0;
		long start = System.nanoTime();
		timer.start();

		while (!j.testTimeOut) {
			for (int i = 0; i<threads; i++) {
				thread[i] = new Thread(j.new RunningThread(), "RunningThread" + i);
				thread[i].start();
			}
			for (int i = 0; i<threads; i++) {
				try {
					thread[i].join();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}		
			System.out.println("Round " + (++round) + " finished after " + (System.nanoTime()-start)/1e9);
		}
	}
}
