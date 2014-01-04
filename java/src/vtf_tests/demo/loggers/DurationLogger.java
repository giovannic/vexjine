package vtf_tests.demo.loggers;

import java.io.File;
import java.io.IOException;
import java.io.PrintStream;
import java.text.DecimalFormat;
import java.util.Collections;
import java.util.Vector;

import vtf_tests.demo.DemoConfiguration;

public class DurationLogger implements LoggerInterface {

	public DurationLogger(String title) {
		durations = new Vector<Long>();
		this.title = title;
	}
	
	@Override
	public void onEventEnd(long startingTime) {
		durations.add(System.nanoTime() - startingTime);
	}

	@Override
	public long onEventStart() {
		return System.nanoTime();
	}

	public void print(PrintStream out)  throws IOException {
//		Collections.sort(durations);
        DecimalFormat myFormatter = new DecimalFormat(DemoConfiguration.logOutputPattern);
		for (Long l : durations) {
//			for (int i = 0; i<durations.size(); i++) {
//				Long l = durations.elementAt(i);
			out.println(title + " " + myFormatter.format(l/1e9));	
		}
	}
	

	@Override
	public void print(String filename)  {
		try {
			PrintStream out = new PrintStream(new File(filename));
			print(out);
		} catch (IOException ie) {
			ie.printStackTrace();
		}
	}
	
	protected Vector<Long> durations;
	protected String title;
}
