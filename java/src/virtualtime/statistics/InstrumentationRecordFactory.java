package virtualtime.statistics;

public class InstrumentationRecordFactory {
	private static final int NO_STATISTICS = 0;
	private static final int CLASS_LEVEL_STATISTICS = 1;
	private static final int METHOD_LEVEL_STATISTICS = 2;
	private static final int GLOBAL_STATISTICS = 3;

	private static int mode = NO_STATISTICS;
	private static InstrumentationStatsRecord cachedInstrumentationRecord = new EmptyInstrumentationStatsRecord();
	private static InstrumentationStatsGatheringInterface statisticsRegistry = new NoInstrumentationStats();
	
	public static void setStatistics(String statistics) {
		if (statistics.equals("class")) {
			mode = CLASS_LEVEL_STATISTICS;
			statisticsRegistry = new ClassLevelStats();
		} else if (statistics.equals("method")) {
			mode = METHOD_LEVEL_STATISTICS;
			statisticsRegistry = new MethodLevelStats();
		} else if (statistics.equals("global")) {
			mode = GLOBAL_STATISTICS;
			cachedInstrumentationRecord = new MethodStatsRecord();	// a single record for all methods
			statisticsRegistry = new GlobalStats(cachedInstrumentationRecord);
		} else {
			mode = NO_STATISTICS;	
		}
	}
	
	public static void printStatistics() {
		statisticsRegistry.print("/data/jine_instrumentation_stats.csv");
	}
	
	public static InstrumentationStatsRecord getRecord() {
		switch (mode) {
		case NO_STATISTICS:
			return cachedInstrumentationRecord;
		case GLOBAL_STATISTICS:
			return cachedInstrumentationRecord;
		default:
			return new MethodStatsRecord();		// else create a new one
		}
	}
	
	public static void addRecord(String className, String methodName, String desc, InstrumentationStatsRecord record) {
		statisticsRegistry.onInstrumentation(className, methodName, desc, record);		
	}
}


