package vtf_tests.serverClientTest.mm1servers;

import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Queue;

import vtf_tests.serverClientTest.Request;
import vtf_tests.serverClientTest.ServerInterface;

/***
 * Single queue class
 * @author root
 */
public abstract class MM1Server implements ServerInterface {
	MM1Server(int totalRequests) {
		queue = new LinkedList<Request>();
		serverThread = null;
		this.totalRequests = totalRequests;
	}
	
	private int totalRequests;
	static final double maximumTheoreticalSecondsPerRequest = 3.0;
	
	public long getTheoreticalValue(double mi, double lambda) {
		return (long)(1e9 / (mi - lambda));
	}
	
	public int[] generateInputValues(double lambda, int totalValues) {
		long averageServiceTime = getAverageServiceTime();
		return generateInputValues(averageServiceTime, lambda, totalValues);
	}
	
	public int[] generateInputValues(long averageServiceTime, double lambda, int totalValues) {
		return generateInputValues(averageServiceTime, lambda, totalValues, 0.0, maximumTheoreticalSecondsPerRequest);
	}
	
	public int[] generateInputValues(long averageServiceTime, double lambda, int totalValues, double from, double to) {
		int[] inputValues = new int[totalValues];

		double maxValue = (1000000000.0 / ((lambda + 1.0/to) * averageServiceTime));

		if (from == 0.0) {
			double step = (maxValue - from)/ totalValues;
			for (int i=0 ; i<totalValues; i++) {
				inputValues[i] = (int)(from + step * (i+1)); 
			}
		} else {
			double lowValue = (1000000000.0 / ((lambda + 1.0/from) * averageServiceTime));

			double step = (maxValue - lowValue)/ totalValues;
			for (int i=0 ; i<totalValues; i++) {
				inputValues[i] = (int)(lowValue + step * (i+1)); 
			}
		}
		return inputValues;
	}
	
	public double[] generateArrivalRates(int totalValues) {
		return generateArrivalRates(totalValues, 0.0, maximumTheoreticalSecondsPerRequest);
	}
	
	/***
	 * Generates arrival rates (assuming exponential distribution) for the constant server 
	 * rate. 
	 * @param totalValues Total number of times to be acquired by the generated rates
	 * @param from 	Lowest time to be expected
	 * @param to	Highest time to be expected
	 * @return Server types that do not have a constant rate just return 1
	 */
	public double[] generateArrivalRates(int totalValues, double from, double to) {
		double[] rates = new double[totalValues];
		for (int i=0; i<totalValues; i++) {
			rates[i] = 1.0;
		}
		return rates;
	}
	
	public void makeRequest(Request request) {
        synchronized (queue) {
            if (queue.isEmpty()) {
            	queue.notify();
            }
            queue.offer(request);
        }
        synchronized (request) {
            while (!request.isServiced()) {
                try {
                	request.wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
	}	
	
	public void start() {
		serverThread = new Thread("MM1-Server") {
			public void run() {
				int requests = 0;
				while (requests++ < totalRequests) {
					Request request;
					synchronized (queue) {
						while (queue.peek() == null) {
							try {
								queue.wait();
							} catch (InterruptedException e) {
								e.printStackTrace();
							}
						}
						request = queue.poll();
					}
					//System.out.println("servicing request " + requests + " from " + totalRequests);
					service(request);
					synchronized (request) {
						request.setServiced(true);
						request.notifyAll();
					}
				}
			}

		};
//		serverThread.setDaemon(true);
		serverThread.start();
	}
	
	public void stop() {
		if (serverThread != null && serverThread.isAlive()) {
			try {
				serverThread.join();
			} catch (InterruptedException ie) {
				ie.printStackTrace();
			}
		}
	}
	
	/** The queue of waiting jobs. */
	protected Queue<Request> queue;
	protected Thread serverThread;
}
