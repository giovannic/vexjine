package vtf_tests.modelLoops;
import java.util.Vector;
import vtf_tests.TestUtils;
import junit.framework.*;

public class BasicFuncCombinations {

	
	public void profile() {
		long totalTime = 0;
		for (int i = 0 ; i<30; i++) {
			totalTime += testThreads(new Vector<Thread>(), 1, 0, 0, 0);
		}
		totalTime /= 30;
		System.out.println((double)totalTime/1e9);
	}
	
	public long testAllReal(int real) {
		return testThreads(new Vector<Thread>(), real, 0, 0, 0);
	}

	public long testAllAccelerated(int acc) {
		return testThreads(new Vector<Thread>(), 0, 0, acc, 0);
	}
	
	public long testAllModel(int modelled) {
		return testThreads(new Vector<Thread>(), 0, modelled, 0, 0);
	}
	
	public long testHalfRealHalfWaiting(int half) {
		return testThreads(new Vector<Thread>(), half, 0, 0, half);
	}
	
	public long testHalfModelHalfReal(int half) {
		return testThreads(new Vector<Thread>(), half, half, 0, 0);
	}
	
	public long testHalfRealHalfAcc(int half) {
		return testThreads(new Vector<Thread>(), half, 0, half, 0);
	}

	public long testThirdModelThirdAccThirdReal(int third) {
		return testThreads(new Vector<Thread>(), third, third, third, 0);
	}


	public long testThirdAccThirdWaitingThirdReal(int third) {
		return testThreads(new Vector<Thread>(), third, 0, third, third);
	}
	
	
	public long testThirdModelThirdWaitingThirdReal(int third) {
		return testThreads(new Vector<Thread>(), third, third, 0, third);
	}
	
	public long testThreads(Vector<Thread> threads, int realThreads, int modelThreads, int acceleratedThreads, int waitingThreads) {
		int totalThreads = realThreads + modelThreads + acceleratedThreads + waitingThreads;
		
		for (int i =0 ; i<totalThreads; i++) {
			if (realThreads-- > 0) {
				threads.add(new Thread(new LoopThread(new RealLoops()), "realThread" + i));
			}
			if (modelThreads-- > 0) {
				threads.add(new Thread(new ModelLoopsSingleLoopThread(new RealLoops()), "modelThread" + i));
			}
			if (acceleratedThreads-- > 0) {
				threads.add(new Thread(new LoopThread(new AcceleratedLoops()), "acceleratedThread" + i));
			}
			if (waitingThreads-- > 0) {
				threads.add(new Thread(new LoopThread(new WaitingLoops()), "waitingThread" + i));
			}			
		}
		
		long start, end;
		start = System.nanoTime();
		try {
			for (int i =0 ; i<totalThreads; i++) {
				threads.elementAt(i).start();
			}
			for (int i =0 ; i<totalThreads; i++) {
				threads.elementAt(i).join();
			}
		} catch (InterruptedException ie) {
			
		}
		end = System.nanoTime();
//		System.out.println((end - start)/1000000000.0);
		
		return (end-start);
	} 
	
//	public static void testRealToModelEquivalence() {
//		TestLoops test = new TestLoops();
//		for (int t = 8; t<=64; t*=2) {
//			double realTime = test.testAllReal(t);
//			double modelTime = test.testHalfModelHalfReal(t/2);
//			System.out.println(t + " threads " + realTime/1e6 + " and " + modelTime/1e6);
//			Assert.assertTrue(TestUtils.checkRatio(realTime, modelTime, 0.20));
//		}
//	}

/*	
	public static void testRealModelAccelEquivalence() {
		TestLoops test = new TestLoops();
		for (int t = 12; t<=12; t*=2) {
			double realTime = test.testAllReal(t);
			double modelTime = test.testThirdModelThirdAccThirdReal(t/3);
			System.out.println(t + " threads " + realTime/1e6 + " and " + modelTime/1e6);
			Assert.assertTrue(TestUtils.checkRatio(realTime, modelTime, 0.20));
		}
	}
*/	

	/*
	public static void testMulticoreBasic() {
//		System.out.println((double)new TestLoops().testAllReal(8)/1e9);
		//System.out.println((double)new TestLoops().testHalfRealHalfWaiting(4)/1e9);
//		System.out.println((double)new TestLoops().testAllAccelerated(4)/1e9);
		//System.out.println((double)new TestLoops().testAllModel(4)/1e9);
		
		//System.out.println((double)new TestLoops().testHalfModelHalfReal(4)/1e9);
	//	System.out.println((double)new TestLoops().testThirdAccThirdWaitingThirdReal(4)/1e9);
//		System.out.println((double)new TestLoops().testThirdModelThirdWaitingThirdReal(4)/1e9);
//		System.out.println((double)new TestLoops().testHalfRealHalfAcc(4)/1e9);
		
		
	}*/

	public static void main(String[] args) {
//		System.out.println((double)new TestLoops().testAllReal(Integer.parseInt(args[0]))/1e9);
		//System.out.println((double)new TestLoops().testThirdAccThirdWaitingThirdReal(4)/1e9);
//		System.out.println((double)new TestLoops().testThirdAccThirdWaitingThirdReal(4)/1e9);

		System.out.println((double)new BasicFuncCombinations().testAllReal(1)/1e9);
	}
}



class LoopThread implements Runnable {
	protected LoopingBehaviour loop;
	public LoopThread(LoopingBehaviour loop) {
		this.loop = loop;
	}

	protected double innerLoops(double startingValue) {
		return loop.doLoop();
	}

	protected double myLoops(double startingValue) {
		double temp = startingValue;
		for (int i =0 ; i < 100; i++) {
			temp += innerLoops(temp);	
		}
		return temp;
	}
	
	public void run() {
		myLoops((double)Thread.currentThread().getId());	
	}
}

interface LoopingBehaviour {
	public double doLoop();
}
class RealLoops implements LoopingBehaviour {
	public RealLoops() {
		
	}
	
	public double doLoop() {
		double temp = 124;
		for (int i =0 ; i < 1000000; i++) {
			temp += (temp / (i+1));	
		}
		return temp;
	}
}

class ModelLoops implements LoopingBehaviour {
	public ModelLoops() {
		
	}
	
	@virtualtime.ModelPerformance(
			jmtModelFilename="/homes/nb605/VTF/src/models/loops_as_local_inf_server.jsimg",
			replaceMethodBody=false)
	public double doLoop() {
		return 6432523.54325;
	}
}

class WaitingLoops implements LoopingBehaviour {
	public WaitingLoops() {
		
	}
				
	public double doLoop() {
		try {
			Thread.sleep(11);
		} catch (InterruptedException ie) {
			
		}
		return 6432523.54325;
	}
}

class ModelLoopsSingleLoopThread extends LoopThread {
	
	public ModelLoopsSingleLoopThread(LoopingBehaviour loop) {
		super(loop);
	}
	
	@virtualtime.ModelPerformance(
			jmtModelFilename="/homes/nb605/VTF/src/models/loops_as_local_inf_server_x100.jsimg",
			replaceMethodBody=false)
	protected double myLoops(double startingValue) {
		return 6432523.54325;
	}
}


class AcceleratedLoops implements LoopingBehaviour {
	public AcceleratedLoops() {
		
	}
	
	@virtualtime.Accelerate(speedup=0.5)
	public double doLoop() {
		double temp = 124;
		for (int i =0 ; i < 2000000; i++) {
			temp += (temp / (i+1));	
		}
		return temp;
	}
}

