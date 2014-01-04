package vtf_tests.demo.loggers;

import java.io.File;
import java.io.IOException;
import java.io.PrintStream;

public class EmptyLogger implements LoggerInterface {

	@Override
	public long onEventStart() {
		return 0;
	}

	@Override
	public void onEventEnd(long startingTime) {

	}

	@Override
	public void print(PrintStream out) throws IOException {
		
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

}
