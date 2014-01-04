package queueing;

import java.util.LinkedList;

/**
 * M/M/1 Queue simulation.
 * 
 * @author Richard 2008
 *
 * @re-authored by Nick 2009 
 */
public class QueueingSim {
    public static void main(String args[]) {
        LinkedList<Job> q = new LinkedList<Job>();
        Server srv = new Server(Integer.parseInt(args[0]));
             
        srv.start("Server", q);        
        TestHarness harness = new TestHarness(400);
 
	    try {
			if (System.getProperty("warmup").equals("true")) {
				harness.warmup(q);
				System.out.println("Warmup period finished");
			}
	   } catch (Exception e) {

	   }
        
        harness.start(q);
    }

}
