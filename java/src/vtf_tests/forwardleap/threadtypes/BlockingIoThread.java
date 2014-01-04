package vtf_tests.forwardleap.threadtypes;

public class BlockingIoThread implements Runnable {
	private int serverPort;
	
	public BlockingIoThread() {
		serverPort   = 3456;                 // server port number			
	}
	
	public BlockingIoThread(int serverPort) {
		this.serverPort = serverPort;
	}
	
	@Override
	public void run() {	
		try {
			java.net.ServerSocket sock = new java.net.ServerSocket(serverPort);   // create socket and bind to port
			java.net.Socket clientSocket = sock.accept();                   // wait for client to connect
				
			//System.out.println("client has connected");	         
			//java.io.PrintWriter pw   	= new java.io.PrintWriter(clientSocket.getOutputStream(),true);
			java.io.BufferedReader br   = new java.io.BufferedReader(new java.io.InputStreamReader(clientSocket.getInputStream()));
			String msg = br.readLine();                  	// read msg from client
			br.readLine();                  	// read msg from client
			System.out.println("Message from the client >" + msg);
			//pw.println(msg);            					// send msg to client
									      
			br.close();
			//pw.close();                                     // close everything
			
			clientSocket.close();
		
			sock.close();
		} catch (Throwable e) {
		
			System.out.println("Server error " + e.getMessage());
		    e.printStackTrace();
		}
		
	}
	
}
