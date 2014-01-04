package vtf_tests.forwardleap.threadtypes;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import vtf_tests.forwardleap.utils.TimeStampPrinter;

public class IoThread  implements Runnable {
	private String filename;
	private int iterations;
	
	public IoThread() {
		iterations = 1;
		filename   = getClass().getClassLoader().getResource(".").getPath() + "../data/ioInput.txt";                 // server port number			
	}
	
	public IoThread(String filename) {
		iterations = 1;
		this.filename = filename;
	}
	
	public IoThread(String filename, int iterations) {
		this.iterations = iterations;
		this.filename = filename;
	}
	
	@Override
	public void run() {	
		
		if (filename == null) {
			return;
		}
		FileInputStream fis = null;
		byte[] cs_buffer = new byte[128];

		for (int i = 0; i<iterations; i++) {
			try {
				fis = new FileInputStream(new File(filename));

				int bytesRead = 0;
				long checkSum = 0;

				while ((bytesRead = fis.read(cs_buffer)) != -1) {
					for (int k = 0; k<bytesRead; k++) {
						checkSum += cs_buffer[k];
					}

				}

			} catch (IOException ie){
				System.out.println(ie.getMessage());
				System.exit(-1);
			} finally {
				if (fis != null) {
					try {
						fis.close();
						TimeStampPrinter.printAbsoluteFinishingTime("IO");
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
			}
		}
	}
	
}