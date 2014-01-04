package vtf_tests.forwardleap.threadtypes;

import java.io.IOException;


public class UserInputIoThread  implements Runnable {
	private int iterations;

	public UserInputIoThread() {
		iterations = 1;
	}

	public UserInputIoThread(String filename) {
		iterations = 1;
	}

	public UserInputIoThread(String filename, int iterations) {
		this.iterations = iterations;
	}

	@Override
	public void run() {	
		byte[] cs_buffer = new byte[1];
		for (int i = 0; i<iterations; i++) {

			int bytesRead = 0;
			long checkSum = 0;

			try {
				System.out.print(">");
				while ((bytesRead = System.in.read(cs_buffer)) != -1) {
					System.out.println("You printed: " + new String(cs_buffer)); 
					for (int k = 0; k<bytesRead; k++) {
						checkSum += cs_buffer[k];
					}
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

}