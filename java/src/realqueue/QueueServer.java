package realqueue;

import java.net.*;
import java.io.*;

public class QueueServer extends Thread { 
	static public final int BUFSIZE = 1024;
	static public int SOBUFSIZE;
	static public int bytesTransferred = 0;  
	static public int initialLingering = 0;
	static public int consumptionDelay = 0;  
	
	public synchronized static int addBytes(int bytes) {
		bytesTransferred += bytes;
		return bytesTransferred;
	}
	public synchronized static int removeBytes(int bytes) {
		bytesTransferred -= bytes;
		return bytesTransferred;
	}

	public QueueServer() {
		
	}
	
	public void run() {

		//entry.add(Thread.currentThread().getName() + " Client address and port " + clntSocket.getInetAddress().getHostAddress() + ":" + clntSocket.getPort() + ":" + clntSocket.getLocalPort());
		//entry.add("Thread = " + Thread.currentThread().getName());
	
		try {
			ServerSocket servSock = new ServerSocket(3567, 256);
			if (QueueServer.SOBUFSIZE > 0 ) {
				servSock.setReceiveBufferSize(QueueServer.SOBUFSIZE);
			}
			
			Socket clntSocket;
//			System.out.println("starting server");
			while (true) {
				clntSocket = servSock.accept();
				//System.out.println("accept o " + Thread.currentThread().getName());			
				InputStream in = clntSocket.getInputStream();
				OutputStream out = clntSocket.getOutputStream();
				DataInputStream din = new DataInputStream(in);

				byte buffer = 'a';
				
				int jobsize = din.readInt();
				//System.out.println("O server daibase jobsize len: " + jobsize);
				
				doWork(Math.PI/2, jobsize);
				out.write(buffer);
				
				clntSocket.close();	
			}			  
		} catch (IOException e) { 
			System.out.println("***********Exception = " + e.getMessage());
			e.printStackTrace();
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
	public void doWork(double y, int iterations) {
		double x = y, a = 0;
		// #ifdef TIMETEST
//		long start =0 , end =0;
//		start = System.nanoTime();
		// #endif
		while (a < iterations) {
			a += Math.sin(x);
			x += Math.cos(x);
		}
		// #ifdef TIMETEST
//		end = System.nanoTime();
//		System.out.println(((end-start)/iterations));
		// #endif
	}
	
}
