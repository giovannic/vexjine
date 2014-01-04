package vtf_tests.demo.loggers;

public class Loggers {
	
	public static LoggerInterface requestResponseTimeLogger = new EmptyLogger();
	public static LoggerInterface selectResponseTimeLogger = new EmptyLogger();
	
	public static LoggerInterface selectThinkingTimeLogger  = new EmptyLogger();
	public static LoggerInterface insertResponseTimeLogger = new EmptyLogger();
	
	public static LoggerInterface insertWaitingTimeLogger  = new EmptyLogger();
	public static LoggerInterface selectRequestStartTimestamp = new EmptyLogger();
	public static LoggerInterface insertRequestStartTimestamp = new EmptyLogger();
	
}
