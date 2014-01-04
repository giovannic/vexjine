package realqueue;

import java.io.*;

public class RealQueue {
	public static Exp interArrivalDist;
	public static void main(String[] args) throws IOException {
		int threads =0;
		int jobsize=0;
		
		if (args.length != 2) {
 			throw new IllegalArgumentException("\nServer Parameter: <Total threads to serve> <Jobsize>\n");			
		}
		
		try {
			threads = Integer.parseInt(args[0]);
			jobsize = Integer.parseInt(args[1]);
		} catch (NumberFormatException ne) {
			throw new IllegalArgumentException("\nServer Parameter: Expected numeric values for threads and jobsize");
		}
		if (threads == 0) {
 			throw new IllegalArgumentException("\nServer Parameter: At least one thread needed for the experiment");	
		}
		
		interArrivalDist = new Exp(0.0025);
		
		// SERVER
		QueueServer server = new QueueServer();
		server.setDaemon(true);
		server.start();
		
		
		// CLIENT
		QueueClient[] clients = new QueueClient[threads];
		for (int i =0 ; i<threads; i++) {
			clients[i] = new QueueClient(3567, jobsize);
		}
		
		try {
			//	Spawning threads, then waiting according to an exponential distribution, and again
			for (int i =0 ; i<threads; i++) {
				clients[i].start();
	            long interArrival = sampleInterArrivalTime();
  	            Thread.sleep(interArrival);
			}
			
			for (int i =0 ; i<threads; i++) {
				clients[i].join();
			}
			
	     } catch (InterruptedException e) {
	         e.printStackTrace();
	     }	
	     
	     
	}

	
	/**
	* Returns the next inter-arrival time.
	* @return
	*/
	private static long sampleInterArrivalTime() {
		long interArrival = (long) interArrivalDist.next();
		while (interArrival < 1) {
		    interArrival = (long) interArrivalDist.next();
		}
		return interArrival;
	}

}

