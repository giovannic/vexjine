package vtf_tests;

import java.text.NumberFormat;
import java.text.ParseException;
import java.util.Locale;

import queueing.Exp;
public class ModelTest {

	public double temp;
	
	ModelTest() {
		temp = 0.0;
	}
	
	protected void updateTemp(int millionIterations) {
    	double doSthIterations = millionIterations * 1000000;
	    for (double i = 1; i<doSthIterations; i++) {
	    	temp += Math.pow(1.0 + 1.0/i, i); 
	    }
	    temp /= doSthIterations;
    }
	
	
	// REMOTE MODELS
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/waiting400.jsimg",
    		replaceMethodBody=true)  		
    private void think() {

    }
    
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/waiting400_x10.jsimg",
    		replaceMethodBody=true)  		
    private void thinkSerialModel() {

    }
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/waiting400_x10.jsimg",
    		replaceMethodBody=false)  		
    private void thinkActuallySerialModel() {
    	try {
    		long delay = (long)Exp.exp(1.0 / 400.0);    	
    		Thread.sleep(delay);
    	} catch (InterruptedException e) {
        	
        }    
    }
    

    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/waiting400.jsimg",
    		replaceMethodBody=false)
    private void thinkActually() {
    	try {
    		long delay = (long)Exp.exp(1.0 / 400.0);    	
    		Thread.sleep(delay);
    	} catch (InterruptedException e) {
        	
        }    	
    }
    
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/mm1.jsimg",
    		replaceMethodBody=true)  		
    private void doSth() {
    	updateTemp(1);
    }
    
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/mm1.jsimg",
    		replaceMethodBody=false)  		
    private void doSthActually() {
    	updateTemp(1);
    }

    
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/mm1.jsimg",
    		replaceMethodBody=true)  		
    private void doMore() {
    	updateTemp(10);
    }
    
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/mm1.jsimg",
    		replaceMethodBody=false)  		
    private void doActuallyMore() {
    	updateTemp(10);
    }
    
    
    
    //LOCAL MODELS
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/waiting400_local.jsimg",
    		replaceMethodBody=true)  		
    private void thinkLocal() {

    }

    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/waiting400_local.jsimg",
    		replaceMethodBody=false)
    private void thinkActuallyLocal() {
    	try {
    		long delay = (long)Exp.exp(1.0 / 400.0);    	
    		Thread.sleep(delay);
    	} catch (InterruptedException e) {
        	
        }    	
    }
    
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/mm1_local.jsimg",
    		replaceMethodBody=true)  		
    private void doSthLocal() {
    	updateTemp(1);
    }
    
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/mm1_local.jsimg",
    		replaceMethodBody=false)  		
    private void doSthActuallyLocal() {
    	updateTemp(1);
    }

    
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/mm1_local.jsimg",
    		replaceMethodBody=true)  		
    private void doMoreLocal() {
    	updateTemp(10);
    }
    
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/mm1_local.jsimg",
    		replaceMethodBody=false)  		
    private void doActuallyMoreLocal() {
    	updateTemp(10);
    }
    
    
    
    
    /*
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/waiting400_local_normal.jsimg",
    		replaceMethodBody=true)  		*/
    private void thinkNormally() {	// think using normal distribution

    }
    
    public String getLegend(int experimentId) {
    	switch(experimentId) {
	    	case 0: return "Remote think,Replace";
	    	case 1: return "Remote think,Execute";
	    	case 2: return "Remote doSth,Replace";
	    	case 3: return "Remote doSth,Execute";
	    	case 4: return "Remote doSthMore,Replace";
	    	case 5: return "Remote doSthMore,Execute";
	    	case 6: return "Local think,Replace";
	    	case 7: return "Local think,Execute";
	    	case 8: return "Local doSth,Replace";
	    	case 9: return "Local doSth,Execute";
	    	case 10: return "Local doSthMore,Replace";
	    	case 11: return "Local doSthMore,Execute";
	    	case 12: return "Remote serial think,Replace";
	    	case 13: return "Remote serial think,Execute";
	    	case 14: return "Local think normal,Replace";
	    	default: return "Unknown experiment";
    	}
    	
    }

    /*
     * Syntax: ModelTest experimentId useBusyWaitBackground
     * 
     */
	public static void main(String[] args) {
		int iterations = 10;
		int experimentId = Integer.parseInt(args[0]);
		ModelTest model = new ModelTest();
		System.out.println("****************************");
		System.out.println(model.getLegend(experimentId));
		System.out.println("****************************");
		
		Thread[] busyBackGroundThread = null;
		int threads = 1;
		long beforeThreadStart = 0;
		boolean usingBackgroundThread = (args.length > 1);
		if (usingBackgroundThread) {
			beforeThreadStart = System.nanoTime();
			try {
				threads = Integer.parseInt(args[1]);
			} catch (Exception e) {
				threads = 1;
			}
			if (threads > 0) {
				busyBackGroundThread = new Thread[threads];
				for (int i=0 ; i<threads; i++) {
					busyBackGroundThread[i] = new Thread(new myBusyThread(iterations), "BusyWaiting Thread " + i);
					busyBackGroundThread[i].start();
				}
			}
		}
		
		long totalStart = System.nanoTime();
		
		switch(experimentId) {
			// RESOURCE	
			case 0: for (int i =0; i<iterations; i++) {	long start = System.nanoTime();	model.think(); long end = System.nanoTime(); System.out.println((end-start) + " from " + start + " to " + end);	}break;
			case 1: for (int i =0; i<iterations; i++) {	long start = System.nanoTime();	model.thinkActually(); long end = System.nanoTime(); System.out.println((end-start) + " from " + start + " to " + end);	}break;
			case 2: for (int i =0; i<iterations; i++) {	long start = System.nanoTime();	model.doSth(); long end = System.nanoTime(); System.out.println((end-start) + " from " + start + " to " + end);	}break;
			case 3: for (int i =0; i<iterations; i++) {	long start = System.nanoTime();	model.doSthActually(); long end = System.nanoTime(); System.out.println((end-start) + " from " + start + " to " + end);	}break;
			case 4: for (int i =0; i<iterations; i++) {	long start = System.nanoTime();	model.doMore(); long end = System.nanoTime(); System.out.println((end-start) + " from " + start + " to " + end);	}break;
			case 5: for (int i =0; i<iterations; i++) {	long start = System.nanoTime();	model.doActuallyMore(); long end = System.nanoTime(); System.out.println((end-start) + " from " + start + " to " + end);	}break;
			// LOCAL
			case 6: for (int i =0; i<iterations; i++) {	long start = System.nanoTime();	model.thinkLocal(); long end = System.nanoTime(); System.out.println((end-start) + " from " + start + " to " + end);	}break;
			case 7: for (int i =0; i<iterations; i++) {	long start = System.nanoTime();	model.thinkActuallyLocal(); long end = System.nanoTime(); System.out.println((end-start) + " from " + start + " to " + end);	}break;
			case 8: for (int i =0; i<iterations; i++) {	long start = System.nanoTime();	model.doSthLocal(); long end = System.nanoTime(); System.out.println((end-start) + " from " + start + " to " + end);	}break;
			case 9: for (int i =0; i<iterations; i++) {	long start = System.nanoTime();	model.doSthActuallyLocal(); long end = System.nanoTime(); System.out.println((end-start) + " from " + start + " to " + end);	}break;
			case 10: for (int i =0; i<iterations; i++) {	long start = System.nanoTime();	model.doMoreLocal(); long end = System.nanoTime(); System.out.println((end-start) + " from " + start + " to " + end);	}break;
			case 11: for (int i =0; i<iterations; i++) {	long start = System.nanoTime();	model.doActuallyMoreLocal(); long end = System.nanoTime(); System.out.println((end-start) + " from " + start + " to " + end);	}break;
			case 12: {long start = System.nanoTime();	model.thinkSerialModel(); long end = System.nanoTime(); System.out.println((end-start) + " from " + start + " to " + end); }break;
			case 13: {long start13 = System.nanoTime();	model.thinkActuallySerialModel(); long end13 = System.nanoTime(); System.out.println((end13-start13) + " from " + start13 + " to " + end13); }break;
			
			case 14: for (int i =0; i<iterations; i++) {long start13 = System.nanoTime();	model.thinkNormally(); long end13 = System.nanoTime(); System.out.println((end13-start13) + " from " + start13 + " to " + end13); }break;
		}

		long totalEnd = System.nanoTime();
		System.out.println("--------"+model.temp+"---------");

		NumberFormat nf = NumberFormat.getInstance(Locale.GERMAN);
		System.out.println("Total virtual ns: " + nf.format(totalEnd-totalStart) + " from " +totalStart + " to " + totalEnd + " :" + (totalEnd-totalStart));
				
		if (usingBackgroundThread) {
			try {
				for (int i=0 ; i<threads; i++) {
					busyBackGroundThread[i].join();
				}
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			long now = System.nanoTime();
			System.out.println("Joined busy thread at : " + nf.format(now-beforeThreadStart));
		}
	}
}


class myBusyThread implements Runnable {
	long temp;
	long totalIterations;
	myBusyThread(int iterations) {
		temp = 1;
		totalIterations = iterations * 100000000;
	}
	
	private long fixIt(long _temp){
		return _temp / 5;
	}
	public void run() {
		for (long i=1; i<totalIterations; i++) {
			if (i % 1000000 == 0) {
				temp = fixIt(temp);	// to get back after NATIVE WAITING :)
			} else {
				if (i % 5 == 0) {
					temp /= 5; 
				} else {
					temp += (i-1);
				}
			}
		}
//		System.out.println(temp);
	}
	
	public double getTemp() {
		return temp;
	}
}