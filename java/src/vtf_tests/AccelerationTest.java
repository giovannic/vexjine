package vtf_tests;
import virtualtime.Accelerate;
import junit.framework.*;

public class AccelerationTest extends TestCase {
	
	public void testOne() {
		testThreads(1);
	}

	public void testFour() {
		testThreads(4);
	}
	
	private void testThreads(int theadsPerCategory) {
		Counters counters = new Counters();
		Thread[] fast   = new Thread[theadsPerCategory];
		Thread[] slow   = new Thread[theadsPerCategory];
		Thread[] normal = new Thread[theadsPerCategory];
		
		for (int i =0 ; i<theadsPerCategory; i++) {
			fast[i] 	= new Thread(new FastThread(counters), "FastThread" + i );
			slow[i] 	= new Thread(new SlowThread(counters), "SlowThread" + i);
			normal[i] 	= new Thread(new NormalThread(counters), "NormalThread" + i);
		}
		
		long start, end;
		start = System.nanoTime();
		try {
			for (int i =0 ; i<theadsPerCategory; i++) {
				slow[i].start();
				normal[i].start();
				fast[i].start();
			}
			
			for (int i =0 ; i<theadsPerCategory; i++) {
				fast[i].join();
				normal[i].join();
				slow[i].join();
			}
		} catch (InterruptedException ie) {
			
		}
		end = System.nanoTime();
		System.out.println((end - start)/1000000000.0 + " with incorrect order " + counters.getErrors());
		assertEquals(0, counters.getErrors());
		assertTrue(counters.haveAllThreadsFinished(theadsPerCategory));
	} 
	
	public static void main(String[] args) {
		AccelerationTest test = new AccelerationTest();
		test.testOne();
	//	test.testFour();
	}
	
}

class Counters {
	private int acceleratedFinished;
	private int normalFinished;
	private int slowFinished;
	private int errors;
	
	public Counters() {
		acceleratedFinished = 0;
		normalFinished = 0;
		slowFinished = 0;
		errors = 0;
	}
	
	public synchronized int getErrors() {
		return errors;
	}
	public boolean haveAllThreadsFinished(int allThreads) {
		return (acceleratedFinished == allThreads && normalFinished  == allThreads && slowFinished == allThreads);
	}
	
	boolean checkNormalBeforeFast() {
		if (normalFinished > 0) {
			System.out.println("Normal-threads finished before fast!");
			return true;
		}
		return false;
	}
	boolean checkAnySlowFinishedBefore(String thisThreadSpeed) {
		if (slowFinished > 0) {
			System.out.println("Slow-threads finished before " + thisThreadSpeed + "!!!");
			return true;
		}
		return false;
	}

	public synchronized void addAccelerated() {
		if (checkNormalBeforeFast() || checkAnySlowFinishedBefore("fast")) {
			++errors;
		}
		++acceleratedFinished;
	}
	public synchronized void addNormal() {
		if (checkAnySlowFinishedBefore("normal")) {
			++errors;
		}
		++normalFinished;
	}
	public synchronized void addSlow() {
		++slowFinished;
	}

}


abstract class AccelerationThread implements Runnable {
	
	protected Counters counters;
	public AccelerationThread(Counters counters) {
		this.counters = counters;
	}

	double innerLoops(double startingValue) {
		double temp = startingValue;
		for (int i =0 ; i < 1000000; i++) {
			temp += (temp / (i+1));	
		}
		return temp;
	}

	double myLoops(double startingValue) {
		double temp = startingValue;
		for (int i =0 ; i < 100; i++) {
			temp += innerLoops(temp);	
		}
		return temp;
	}
	
	abstract protected void logFinished();

	void runProcess() {
		myLoops((double)Thread.currentThread().getId());
		logFinished();
	}
	
	abstract public void run();
}


class FastThread extends AccelerationThread {
	public FastThread(Counters counters) {
		super(counters);
	}
	protected void logFinished() {
		counters.addAccelerated();
	}
	
	@virtualtime.Accelerate (speedup = 0.5)
	public void run() {
		runProcess();
	}
}

class SlowThread extends AccelerationThread  {

	public SlowThread(Counters counters) {
		super(counters);
	}
	protected void logFinished() {
		counters.addSlow();
	}
	
	@virtualtime.Accelerate (speedup = 5.0)
	public void run() {
		runProcess();
	}
}


class NormalThread extends AccelerationThread {

	public NormalThread(Counters counters) {
		super(counters);
	}
	protected void logFinished() {
		counters.addNormal();
	}
	public void run() {
		runProcess();
	}
}


