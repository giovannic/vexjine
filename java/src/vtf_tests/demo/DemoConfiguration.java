package vtf_tests.demo;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintStream;

import vtf_tests.demo.loggers.DurationLogger;
import vtf_tests.demo.loggers.EmptyLogger;
import vtf_tests.demo.loggers.LoggerInterface;
import vtf_tests.demo.loggers.Loggers;
import vtf_tests.demo.loggers.TimestampLogger;

public class DemoConfiguration {
	public static int warmupRequests = 20;
	public static int normalRequests = 3000;
	public static String logOutputPattern = new String("0.0000");
	public static double lambda = 0.4;	

	public static int defaultCacheSize = 39;

	static {
		try {
			defaultCacheSize = Integer.parseInt(System.getProperty("cacheSize"));
		} catch (Exception e) {
			defaultCacheSize = 39;
		}
	}
	
	public static void setLoggers() {
		
		if (System.getProperty("timeServer") != null && System.getProperty("timeServer").equals("true")) {
			Loggers.requestResponseTimeLogger = new DurationLogger("REQUEST duration");
			
			Loggers.selectResponseTimeLogger = new DurationLogger("SELECT duration");
			Loggers.insertResponseTimeLogger = new DurationLogger("INSERT duration");
		}
		if (System.getProperty("timeThink") != null && System.getProperty("timeThink").equals("true")) {
			Loggers.selectThinkingTimeLogger = new DurationLogger("SELECT think-time");
			Loggers.insertWaitingTimeLogger = new DurationLogger("INSERT think-time");	
		}
		if (System.getProperty("requestStart") != null && System.getProperty("requestStart").equals("true")) {
			Loggers.selectRequestStartTimestamp = new TimestampLogger("SELECT request start");
			Loggers.insertRequestStartTimestamp = new TimestampLogger("INSERT request start");	
		}		
		
	}

	public static void printLoggers() {
		PrintStream out = null;
		try {
			if (System.getProperty("logToFile") != null && System.getProperty("logToFile").equals("true")) {
				out = new PrintStream(new File("/data/demo_logfile"));
			} else {
				out = System.out;
			}
			
			Loggers.requestResponseTimeLogger.print(out);
			
			Loggers.selectResponseTimeLogger.print(out);
			Loggers.insertResponseTimeLogger.print(out);
			
			Loggers.selectThinkingTimeLogger.print(out);
			
	//		Loggers.insertWaitingTimeLogger.print();	
			Loggers.selectRequestStartTimestamp.print(out);
			Loggers.insertRequestStartTimestamp.print(out);
		
			if (System.getProperty("logToFile") != null && System.getProperty("logToFile").equals("true")) {
				out.close();
			}
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException io) {
			io.printStackTrace();
		}
	}
	
}
