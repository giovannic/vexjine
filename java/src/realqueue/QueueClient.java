package realqueue;

import java.net.*;
import java.io.*;

public class QueueClient extends Thread {
	
	int serverPort;
	Exp jobSizeDist;
	
	QueueClient(int _port, int rps) {
		serverPort = _port;
		jobSizeDist = new Exp(((double) 1 / rps));
	}
	
	public void run() {		
		try {

			long start = System.nanoTime();
			Socket socket = new Socket("127.0.0.1", serverPort);								

			//	System.out.println(Thread.currentThread().getName() + " connected... Client address and port " + socket.getInetAddress().getHostAddress() + ":" + socket.getPort() + ":" + socket.getLocalPort());			
			InputStream in = socket.getInputStream();
			OutputStream out = socket.getOutputStream();
			DataOutputStream dout = new DataOutputStream(out);
			
			
			dout.writeInt(sampleJobSize());
			//System.out.println(Thread.currentThread().getName() + " waiting");
			in.read();
			//System.out.println(Thread.currentThread().getName() + " served");
			long end = System.nanoTime();
			System.out.println("* " + (end-start));
		
		} catch (Exception e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}					
	}
	
	
	/**
	 * Get a sample of the jobs size.
	 * 
	 * @return
	 */
	int sampleJobSize() {
		double serviceTime = jobSizeDist.next();
		while (serviceTime <= 0) {
			serviceTime = jobSizeDist.next();
		}
		return (int) (serviceTime);
	}

	
}

