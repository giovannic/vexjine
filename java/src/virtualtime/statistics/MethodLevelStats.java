package virtualtime.statistics;

import java.io.File;
import java.io.IOException;
import java.io.PrintStream;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.StringTokenizer;

public class MethodLevelStats implements InstrumentationStatsGatheringInterface {
	Map<Integer, MethodLevelInfo> methodStatsRecords;
	public MethodLevelStats() {
		methodStatsRecords = new HashMap<Integer, MethodLevelInfo>();
	}
	
	@Override
	public void onInstrumentation(String className, String methodName,
			String desc, InstrumentationStatsRecord record) {

		String fqn = className + " " + methodName + " " + desc;
		int methodHashCode = fqn.hashCode();
		MethodLevelInfo methodLevelInformation = null; 
		
		if (!methodStatsRecords.containsKey(methodHashCode)) {
			methodLevelInformation = new MethodLevelInfo(fqn);
			methodStatsRecords.put(methodHashCode, methodLevelInformation);
		} else {
			methodLevelInformation = methodStatsRecords.get(methodHashCode);
		}
		
		methodLevelInformation.add(record);
		
	}

	@Override
	public void print(String string) {
		try {
			PrintStream out = new PrintStream(new File(string));
			out.println("JINE instrumentation statistics: By method");
			out.println("Class,Method,FQN,Profiled,Invalidated,Instructions,IO points,Sync primitives");
			
			Iterator<Map.Entry<Integer, MethodLevelInfo>> it = methodStatsRecords.entrySet().iterator();
		    while (it.hasNext()) {
		        Map.Entry<Integer, MethodLevelInfo> pairs = (Map.Entry<Integer, MethodLevelInfo>)it.next();
		        MethodLevelInfo methodLevelInformation = (MethodLevelInfo)pairs.getValue();
		        methodLevelInformation.print(out);	     
		    }

			out.close();
		} catch (IOException e) {
			e.printStackTrace(); 
		}
	}
}

class MethodLevelInfo {
	MethodLevelInfo(String fqn) {
		this.fqn = fqn;
		record = new MethodStatsRecord();
		record.setProfiled();
	}
	
	public void print(PrintStream out) {
		StringTokenizer tokens = new StringTokenizer(fqn, " ");
		out.println(tokens.nextToken() + "," + tokens.nextToken() + "," + tokens.nextToken() + "," + (record.isProfiled()?"1":"0")  + "," + (record.isRetransformed()?"1":"0") + "," + record.getInstructions()  + "," + record.getIoPoints()  + "," + record.getSyncPrimitives());
		
	}

	public static void getStackTrace(PrintStream output) {
		Thread curThread = Thread.currentThread();
		StackTraceElement[] st_array = curThread.getStackTrace();
		int stackTraceLength = st_array.length;
		StackTraceElement st;
		for (int i = 2 ;i<stackTraceLength; i++) {	// the last two methods are getStackTrace() and Thread.getStackTrace();
			st = st_array[i];
			output.println(st.getClassName()+"."+st.getMethodName() + "():" +st.getLineNumber() + "<-");
		}
		System.out.println();
	}
	public void add(InstrumentationStatsRecord record2) {
		if (!record2.isRetransformed()) {
			record.add(record2);
		} else {
			if (!record2.isProfiled()) {
				getStackTrace(System.out);
				record.setRetransformed();
			}
		}
	}

	MethodLevelInfo(String className, String methodName, String desc) {
		fqn = className + " " + methodName + " " + desc;
	}

	String fqn;
	InstrumentationStatsRecord record;
}