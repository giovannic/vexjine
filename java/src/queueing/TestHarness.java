package queueing;

import java.util.Queue;

//import virtualtime.EventNotifier;

/**
 * Test harness which submits jobs to the queue.
 * For each job, we create a new thread which submits the job and 
 * wbaits until the job is serviced before returning.
 * @author Richard
 * 2008
 */
public class TestHarness {

	/**The queue of waiting jobs.*/
    Queue<Job> queue;

    /** Inter-arrival time distribution.*/
    Exp interArrivalDist;

    /**
     * Create a test harness with the given mean inter-arrival time.
     * @param mean_interArrivalTime
     */
    public TestHarness(double mean_interArrivalTime) {
        interArrivalDist = new Exp((double)1 / mean_interArrivalTime);
    }

//    virtualtime.ModelPerformance(
  //  /		jmtModelFilename="/homes/nb605/VTF/src/models/waiting400.jsimg",
   // 		replaceMethodBody=true)  		
    private void think() {
        try {
			double nextThinkTime = sampleInterArrivalTime();
    		long thinkingTimeMilliSeconds = (long)nextThinkTime;
    		int thinkingTimeNanoSeconds = (int)((nextThinkTime - thinkingTimeMilliSeconds) * 1000000);
    	
            wait(thinkingTimeMilliSeconds, thinkingTimeNanoSeconds);
            
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        
    }
    
    /**
     * Submits 400 jobs to the queue, at the rate determined by sampling the
     * inter-arrival time distribution.
     * 
     * @param q The queue of jobs.
     */
    public synchronized void start(Queue<Job> q) {
    	
        queue = q;

        int request = 0;
        while (request < 400) {
//        	System.out.println("arrival" + request);
            Thread thd = new Thread("arrival " + request) {
                public void run() {
                    makeRequest();
                }
            };
            thd.setDaemon(true);
            thd.start();

            think();
            
            request++;
        }

    }


    /**
     * Submits a request, waits until the job is serviced before returning.
     * The execution time of this method is the response time for the queue.
     */
    private void makeRequest() {
    
//    	System.out.println("new request!");
        Job job = new Job();
        synchronized (queue) {
            queue.offer(job);
            queue.notify();
        }
        synchronized (job) {
            while (!job.serviced) {
                try {
                    job.wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }

    }

    
    
    
    
    
    /**
     * Submits 400 jobs to the queue, at the rate determined by sampling the
     * inter-arrival time distribution.
     * 
     * @param q The queue of jobs.
     */
    public synchronized void warmup(Queue<Job> q) {
    	
        queue = q;

        int request = 0;
        while (request < 10) {
        	//System.out.println("arrival" + request);
            Thread thd = new Thread("arrival" + request) {
                public void run() {
                    makeWarmupRequest();
                }
            };
            thd.setDaemon(true);
            thd.start();

            try {
            	double nextThinkTime = sampleInterArrivalTime();
        		long thinkingTimeMilliSeconds = (long)nextThinkTime;
        		int thinkingTimeNanoSeconds = (int)((nextThinkTime - thinkingTimeMilliSeconds) * 1000000);
        		
        		wait(thinkingTimeMilliSeconds, thinkingTimeNanoSeconds);
                
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            request++;
        }

    }


    /**
     * Submits a request, waits until the job is serviced before returning.
     * The execution time of this method is the response time for the queue.
     */
    private void makeWarmupRequest() {

        Job job = new Job();
        synchronized (queue) {
            //System.out.print(".");
            queue.offer(job);
            queue.notify();
        }
        synchronized (job) {
            while (!job.serviced) {
                try {
                    job.wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }

    }
    
    
    
    /**
     * Returns the next inter-arrival time.
     * @return
     */
	private double sampleInterArrivalTime() {
        return interArrivalDist.next();
    }

}
