package queueing;

//import virtualtime.Accelerate;
import java.util.Queue;

import virtualtime.EventNotifier;

/**
 * Server class which operates on a queue. Each job is removed from the queue
 * and 'serviced'. The 'Service' task consists of building a long String by
 * appending characters. The length of the String to be built is determined by
 * sampling a Normal distribution.
 * 
 * @author Richard 2008
 */
public class Server {

	/** The queue of waiting jobs. */
	Queue<Job> queue;

	/** The job size distribution. */
	Exp jobSizeDist;
	
	/** Initialize with the requests per second - Was 400000 */
	public Server(int rps) {
		jobSizeDist = new Exp(((double) 1.0 / (double)rps));

		// Used to preload the packages classes
		System.nanoTime();
		Math.sin(Math.PI/2);
		Math.cos(Math.PI/2);

	}

	/**
	 * Starts servicing removing jobs from the queue and servicing them.
	 * 
	 * @param q
	 */
	public void start(String threadName, Queue<Job> q) {
		queue = q;
		Thread thread = new Thread(threadName) {

			@Override
			public void run() {
				while (true) {
					Job job;
					synchronized (queue) {
						while (queue.peek() == null) {
							try {
//								System.out.println("SERVER WAITING ON EMPTY QUEUEUEUEUEUEUEUEUEUEUEUEUEUEUEUEUEUUEUEUEUEUEUEUEUEUUEUEUEUEUEUEUEUEUUEUEUEUEUEUEUEUEU " + queue.hashCode());
								queue.wait();
							} catch (InterruptedException e) {
								e.printStackTrace();
							}
						}
						job = queue.poll();
						//System.out.print(queue.size());
					}
					serviceRequest(job);
					synchronized (job) {
						job.serviced = true;
						job.notifyAll();
					}
				}
			}

		};
		thread.setDaemon(true);
		thread.start();
	}

	/**
	 * Submits a request to the queue, then waits until the job is serviced.
	 */
	private void serviceRequest(Job job) {
		if (job != null) {
			int length = sampleJobSize();
//long start = System.nanoTime();
//long start_cputime = EventNotifier.getThreadVirtualTime();
			doWork(Math.PI / 2, length);
//long end_cputime = EventNotifier.getThreadVirtualTime();
//long diff_cputime = end_cputime - start_cputime;
//long diff = System.nanoTime() - start;
//System.out.println("Service time " + diff + " for " + length + " = " + (diff/length) +  " in cpu time: " + diff_cputime + " " + (diff_cputime/length));
		}
	}

	/**
	 * Perform some real work.
	 * 
	 * @param y
	 *            This must be passed in pi/2. Passed as a parameter to avoid
	 *            possible compiler optimisations.
	 * @param iterations
	 *            The number of loop iterations to perform.
	 */

	
	// Ac//cele//rate(speedup = 1.25)
	public void doWork(double y, int iterations) {
		double x = y, a = 0;
		while (a < iterations) {
			a += Math.sin(x);
			x += Math.cos(x);
		}
	}

	/**
	 * Get a sample of the jobs size.
	 * 
	 */
	int sampleJobSize() {
		double serviceTime = jobSizeDist.next();
		while (serviceTime < 1) {
			serviceTime = jobSizeDist.next();
		}
		return (int) (serviceTime);
	}

	public static void main(String[] args) {
		// Loading used classes
		System.nanoTime();
		Math.sin(Math.PI/2);
		Math.cos(Math.PI/2);

		Server s = new Server(400000);
		s.doWork(Math.PI/2, Integer.parseInt(args[0]));
	}
}
