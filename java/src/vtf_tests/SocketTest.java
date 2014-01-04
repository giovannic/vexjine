package vtf_tests;

import java.net.*;
import java.io.*;


public class SocketTest {
	public static Socket clientSocket;
	public static InputStreamReader clientInput;
	public static java.io.BufferedReader br;
	
	public static void main(String[] args) {
		long start, end;
		int serverPort = 7865;
		start = System.nanoTime();
		
		ServerSocket socket;
		
		try {
			socket = new ServerSocket(serverPort);
		} catch ( IOException e ) {
			System.out.println( "Could not create the socket..." );
			return;
		}
		
		Thread server = new Thread(new Server(socket, serverPort));
		server.start();
		
		try {
			Thread.sleep(500);
		} catch (InterruptedException ie) {
				
		}
		
		/*
		//Works
		try {
			socket.close();
			System.out.println( "Interrupting socket accept..." );
		} catch ( IOException e ) {	
			return;
		}*/
		
		
		Thread client = new Thread(new Client(serverPort));
		client.start();
		
		
		
		try {
			Thread.sleep(1000);
		} catch (InterruptedException ie) {
				
		}
		
		System.out.println( "Interrupting socket read?..." );
		try {
			//server.interrupt();
			SocketTest.clientInput.close();
			SocketTest.br.close();
			SocketTest.clientSocket.close();
		} catch (IOException ie) {
			ie.printStackTrace();
		}
		
		
		System.out.println("in main() - leaving");
		end = System.nanoTime();
		System.out.println("Duration: " + (end-start));
	}

}


class Server implements Runnable {
	int serverPort;
	ServerSocket socket;

	Server(ServerSocket socket, int serverPort) {
		this.socket 	= socket;
		this.serverPort = serverPort;
	}
	
	public void run() {

		System.out.println( "Waiting for connection..." );
		try {
			
			SocketTest.clientSocket = socket.accept();                   // wait for client to connect
//			SocketTest.clientSocket.setSoTimeout(1000);
		} catch (IOException ioe) {
			System.out.println("Server interrupted while accepting");
			return;
		}
		boolean readMessage = false;
		while (!readMessage) {
			try {
					
				SocketTest.clientInput = new java.io.InputStreamReader(SocketTest.clientSocket.getInputStream());
				SocketTest.br  = new java.io.BufferedReader(SocketTest.clientInput);
				System.out.println("Waiting new message from client...");

				String msg = SocketTest.br.readLine();                  	// read msg from client 
				System.out.println("Message from the client >" + msg);
				readMessage = true;
				SocketTest.br.close();
				SocketTest.clientSocket.close();

				socket.close();
			} catch (SocketTimeoutException socke) {
				System.out.println("Server timed out while reading");
				
			} catch (IOException ioe) {
				System.out.println("Server interrupted while reading");
				return;
			}
		}
		System.out.println("in run() - leaving normally");

	}
}


class Client implements Runnable {
	private int serverPort;
	Client(int serverPort) {
		this.serverPort = serverPort;
	}
	
	public void run() {
		String    serverIPname = "localhost";     // server IP name
	        
	   
	    String message = new String("A");
	    int message_size = 16;
	    for (int i = 1; i<message_size; i++) {
	    	message += "A";
	    }
		
    	try {
    		Socket sock = new java.net.Socket(serverIPname,serverPort);       			// create socket and connect
	        System.out.println("Connected to Server");
    		
    		java.io.PrintWriter pw   = new java.io.PrintWriter(sock.getOutputStream(), true);  // create reader and writer
    		
    		double temp = 1.0;
    		double sum = 0.0;
    		// ~10sec waiting
    		for (int k = 1; k < 35; k++) {
    			for (int j = 1; j < 100000000; j++) {
	    			sum += temp;
	    			temp /= 2.0;
    			}
    		}
    		for (int i = 1; i<8*sum; i++) {
    			message += "A";
    	    }
//    		try {
//    			Thread.sleep(6000);
//    		} catch (InterruptedException ie) {
//    				
//    		}
	        pw.println(message);        // send msg to the server		
            pw.close();                 // close everything

	        sock.close();
		
		} catch (Throwable e) {
	        System.out.println("Client error " + e.getMessage());
			e.printStackTrace();
    	}	

	}
}
