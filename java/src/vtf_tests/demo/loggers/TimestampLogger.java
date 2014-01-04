package vtf_tests.demo.loggers;

import java.io.File;
import java.io.IOException;
import java.io.PrintStream;
import java.text.DecimalFormat;
import java.util.Collections;
import java.util.Vector;

import vtf_tests.demo.DemoConfiguration;

public class TimestampLogger implements LoggerInterface {

	public TimestampLogger(String title) {
		timestamps = new Vector<Long>();
		this.title = title;
	}
	
	@Override
	public void onEventEnd(long startingTime) {
		timestamps.add(startingTime);
		
	}

	@Override
	public long onEventStart() {
		return 0;
	}
	
	public void print(PrintStream out) throws IOException {
		if (timestamps.size() > 0) {
			Collections.sort(timestamps);
			long min = timestamps.elementAt(0);
	        DecimalFormat myFormatter = new DecimalFormat(DemoConfiguration.logOutputPattern);
			for (Long l : timestamps) {
				out.println(title + " " + myFormatter.format((l-min)/1e9));	
			}
		}
	}
	

	@Override
	public void print(String filename) {
		try {
			PrintStream out = new PrintStream(new File(filename));
			print(out);
		} catch (IOException ie) {
			ie.printStackTrace();
		}
	}
	protected Vector<Long> timestamps;
	protected String title;
 
}
