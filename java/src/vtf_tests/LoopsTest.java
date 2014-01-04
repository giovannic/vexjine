/*
 * This test wants to show how a code can be executed in real time to fail the test
 * and in virtual time to pass it
 */
package vtf_tests;

import junit.framework.TestCase;

public class LoopsTest extends TestCase {
	LoopsTest(String name) {
		super(name);
	}
	
	public void testNormal() {
		int threads = 4;
		Thread[] thd = new Thread[threads];		
		long myLimit = 10000000;
		parallelThread pt = new parallelThread(myLimit);
	    for(int i =0; i<threads;i++) {     
	        thd[i] = new Thread(pt, "loopsTestThread" + i);
	        thd[i].start();
	    }
	    for(int i =0; i<threads;i++) {
	    	try {
				thd[i].join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
	    }
	}
}

class parallelThread implements Runnable {
	private long myLimit;
	
	parallelThread(long _myLimit) {
		myLimit = _myLimit;		 
	}

	private double myLoops() {
		double temp = 1;
		for (int i =0 ; i < 3; i++) {
			temp += methodInner();	
		}
		return temp;
	}

	private double methodInner() {
		double temp = 1;
		for (int i =0 ; i < myLimit; i++) {
			temp += (temp / (i+1));	
		}
		return temp;
	}

	@virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/io_model_10ms.jsimg",
    		replaceMethodBody=true)
	public void run() {
		double temp = myLoops();
		temp += 1;
	}
}
