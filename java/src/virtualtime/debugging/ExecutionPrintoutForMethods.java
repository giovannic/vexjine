package virtualtime.debugging;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.HashMap;
import java.util.Vector;

public class ExecutionPrintoutForMethods {

	protected HashMap<Integer, Vector<String>> methodsToInstrument = null;
	public ExecutionPrintoutForMethods(String filename) {
		methodsToInstrument = new HashMap<Integer, Vector<String>>();
		parseFileForMethods(filename);
	}
	 
	protected void parseFileForMethods(String filename) {
		File file = new File(filename);

		BufferedReader bufRdr = null;
		try {
			bufRdr = new BufferedReader(new FileReader(file));

			String line = null;
			String className = null;
			String methodName = null;
			int lastSlash;
			while((line = bufRdr.readLine()) != null) {
				if (line.equals("") || line.charAt(0) == '#') {
					continue;
				}
				//if ((lastSlash = line.lastIndexOf('/')) != -1) {
				lastSlash = line.indexOf(' ');
				if (lastSlash == -1) {
					add(line.hashCode(), null);
				} else {
					
					className = line.substring(0, lastSlash);
					methodName = line.substring(lastSlash+1);
					lastSlash = methodName.indexOf(' ');
					if (lastSlash != -1) {
						methodName = methodName.substring(0, lastSlash);
					}
					add(className.hashCode(), methodName);
				}
			}
		} catch (IOException e1) {
			e1.printStackTrace();
		}
	}
	
	protected void add(Integer classId, String methodName) {
		Vector<String> s = null;
		if (methodName != null) {	// methodName null means that the entire class should be monitored
			if (methodsToInstrument.containsKey(classId)) {
				s = methodsToInstrument.get(classId);
				
			} else {
				s = new Vector<String>();
			}
			s.add(methodName);
		}
		methodsToInstrument.put(classId, s);
	}
	
	public boolean instrumentsClass(String className) {
		return methodsToInstrument.containsKey(className.hashCode());
	}
	
	public Vector<String> getMethodsOfClass(String className) {
		return methodsToInstrument.get(className.hashCode());
	}
	
}
