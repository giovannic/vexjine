package virtualtime.statistics;

import java.io.File;
import java.io.IOException;
import java.io.PrintStream;

public class GlobalStats implements InstrumentationStatsGatheringInterface {
	private InstrumentationStatsRecord globalRecord;
	private String lastClassName;
	GlobalStats(InstrumentationStatsRecord record) {
		globalRecord = record;
		lastClassName = null;
		totalClasses = 0;
		totalMethods = 0;
		profiledMethods = 0;
		totalInvalidations = 0;
	}
	
	@Override
	public void onInstrumentation(String className, String methodName,
			String desc, InstrumentationStatsRecord record) {
	
		if (!record.isRetransformed()) {
			if (lastClassName == null || !lastClassName.equals(className)) {
				++totalClasses;
				lastClassName = className;
			}
			++totalMethods;
			if (record.isProfiled()) {
				++profiledMethods;
				record.clearFlags();	
			}
		} else {
			++totalInvalidations;
			record.clearFlags();
		}
	}
	
	private static int totalClasses;
	private static int totalMethods;
	private static int profiledMethods;
	private static int totalInvalidations ;
	
	@Override
	public void print(String string) {
		try {
			PrintStream out = new PrintStream(new File(string));
			out.println("JINE instrumentation statistics: Global");
			out.println("Total classes: " + totalClasses);
			out.println("Total methods: " + totalMethods);
			out.println("Profiled methods: " + profiledMethods + "(" + (100.0*(double)profiledMethods/(double)totalMethods)+ ")");
			out.println("Total instructions: " + globalRecord.getInstructions() + " (pm: " + (double)globalRecord.getInstructions()/(double)totalMethods + ")");
			out.println("Total IO points: " + globalRecord.getIoPoints());
			out.println("Total sync primitives: " + globalRecord.getSyncPrimitives());
			out.close();
		} catch (IOException e) {
			e.printStackTrace(); 
		}
	}

}

