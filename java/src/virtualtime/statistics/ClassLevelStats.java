package virtualtime.statistics;

import java.io.File;
import java.io.IOException;
import java.io.PrintStream;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

public class ClassLevelStats implements InstrumentationStatsGatheringInterface {
	Map<Integer, ClassLevelInformation> classStatsRecords;
	
	public ClassLevelStats() {
		classStatsRecords = new HashMap<Integer, ClassLevelInformation>();
	}
	
	@Override
	public void onInstrumentation(String className, String methodName,
			String desc, InstrumentationStatsRecord record) {
		

		int classHashCode = className.hashCode();
		ClassLevelInformation classLevelInformation = null; 
		
		if (!classStatsRecords.containsKey(classHashCode)) {
			classLevelInformation = new ClassLevelInformation(className);
			classStatsRecords.put(classHashCode, classLevelInformation);
		} else {
			classLevelInformation = classStatsRecords.get(classHashCode);
		}
		
		classLevelInformation.add(record);
	}

	@Override
	public void print(String string) {
		try {
			PrintStream out = new PrintStream(new File(string));
			out.println("JINE instrumentation statistics: By class");
			out.println("Class,Methods,Profiled,Profiled(%),Invalidated (%),Total Instructions,Total IO points,Total sync primitives");
			
			Iterator<Map.Entry<Integer, ClassLevelInformation>> it = classStatsRecords.entrySet().iterator();
		    while (it.hasNext()) {
		        Map.Entry<Integer, ClassLevelInformation> pairs = (Map.Entry<Integer, ClassLevelInformation>)it.next();
		        ClassLevelInformation classLevelInformation = (ClassLevelInformation)pairs.getValue();
		        classLevelInformation.print(out);	     
		    }

			out.close();
		} catch (IOException e) {
			e.printStackTrace(); 
		}
	}


}

class ClassLevelInformation {
	ClassLevelInformation(String className) {
		this.className = className;
		summedMethodRecords = new MethodStatsRecord();
		totalMethods = 0;
		profiledMethods = 0;
		invalidatedMethods = 0;
	}
	
	public void print(PrintStream out) {
		out.println(className + "," + totalMethods + "," + profiledMethods + "," + 100.0*(double)profiledMethods/(double)totalMethods + "," + 100.0*(double)invalidatedMethods/(double)totalMethods + "," + summedMethodRecords.getInstructions() + "," + summedMethodRecords.getIoPoints() + "," + summedMethodRecords.getSyncPrimitives());	
	}

	public void add(InstrumentationStatsRecord record) {
		if (!record.isRetransformed()) {
			summedMethodRecords.add(record);
			++totalMethods;
			if (record.isProfiled()) {
				++profiledMethods;		
			}
		} else {
			++invalidatedMethods;
		}
	}

	String className;
	InstrumentationStatsRecord summedMethodRecords;
	int totalMethods;
	int profiledMethods;
	int invalidatedMethods;
}