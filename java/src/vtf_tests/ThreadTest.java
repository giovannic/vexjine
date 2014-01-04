package vtf_tests;

public class ThreadTest {

	
	public static void main(String[] args) {
		long start = System.nanoTime();
		
		int threadsCount = Integer.parseInt(args[0]);
		parallel2[] threads = new parallel2[threadsCount];
		
		for (int i =0 ; i<threadsCount; i++) {
			threads[i] = new parallel2(1000);
			threads[i].start();
		}
		
		try {
			for (int i =0 ; i<threadsCount; i++) {
				threads[i].join();
			}
		} catch(InterruptedException ie) {
			
		}
		
		
		long end = System.nanoTime();
		System.out.println((end-start)/1000000000.0);
	}
}





class parallelThreadExtending extends Thread {
	
	private long myLimit;
	
	parallelThreadExtending(long _myLimit) {
		myLimit = _myLimit;
	}
	
	
	
	public void run() {
		double temp =0 ;
		for (long i = 0; i<myLimit; i++) {
			temp += Math.sin(i);
		}
		temp += 1;
	}
}

class parallel2 extends parallelThreadExtending {
	
	private long myLimit;
	
	parallel2(long _myLimit) {
		super(_myLimit);
		myLimit = _myLimit;
	}
	
	double apply() {
		double temp =0 ;
		for (long i = 0; i<10000; i++) {
			temp += Math.cos(i);
		}
		return temp;
	}
	
	public void run() {
		double temp =0 ;
		for (long i = 0; i<myLimit; i++) {
			temp += apply();
		}
		temp += 1;
		System.out.println(Thread.currentThread().getName() + " " + temp);
	}
}

