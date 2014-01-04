package queueing;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;


public class ServiceTimeStats {

	
	public static void main(String[] args) throws IOException {
		
		Server server = new Server(400000);
		File file = new File("servicetimes.csv");
		FileWriter writer = new FileWriter(file);
		
		for (int i=0; i<10000; i++) {
			for (int j=0; j<10; j++) {
				int serviceLength = server.sampleJobSize();
				writer.write(serviceLength + ",");
			}
			writer.write("\n");
		}
		writer.flush();
		writer.close();
		System.out.println("done");
		
	}
	
}
