package virtualtime.debugging;

public class MessagePrinter {

	
	final static long zeroTime = System.currentTimeMillis();
	public static void print(String className, String methodName) {
		StringBuilder str = new StringBuilder();
		str.append("Method: " + className + " " + methodName);
		for(int i = (9+className.length()+methodName.length()); i< 70; i++) {
			str.append(" ");
		}
		Thread curThread = Thread.currentThread();
		str.append(curThread.getName() + " at " + ((System.currentTimeMillis() - zeroTime)/1e3) + "\n");
		StackTraceElement[] st_array = curThread.getStackTrace();
		int stackTraceLength = st_array.length;
			
		for (int i =2 ;i<stackTraceLength; i++) {	// the last two methods are getStackTrace() and Thread.getStackTrace();
			str.append("\t\t\t\t" + st_array[i].getClassName() + " " + st_array[i].getMethodName() + "\n");
		}
		
		System.out.println(str.toString());
		//System.out.println(str.toString() + Thread.currentThread() + " at " + ((System.currentTimeMillis() - zeroTime)/1e3));
		
	}
}
