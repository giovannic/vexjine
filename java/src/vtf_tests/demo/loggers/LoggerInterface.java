package vtf_tests.demo.loggers;

import java.io.IOException;
import java.io.PrintStream;

public interface LoggerInterface {
	public long onEventStart();
	public void onEventEnd(long startingTime);
	public void print(PrintStream out) throws IOException;
	public void print(String filename);
}
