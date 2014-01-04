package virtualtime.statistics;

public interface InstrumentationStatsGatheringInterface {	
	public void onInstrumentation(String className, String methodName, String desc, InstrumentationStatsRecord record);

	public void print(String string);
}
