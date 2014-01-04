/*
 * Created on Jun 9, 2008
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package virtualtime;

public class EventNotifiervtonly {
	public static native void init();
	
    private static native long _getThreadVirtualTime();
    public static long getThreadVirtualTime() {
    	return _getThreadVirtualTime();
    }
    
    private static native long _getPapiRealTime();
	public static long getPapiRealTime() {
		return _getPapiRealTime();
	}
		
    private static native long _getJvmtiRealTime();
    public static long getJvmtiRealTime() {
    	return _getJvmtiRealTime();
    }
    private static native long _getJvmtiVirtualTime(Thread thread);
    public static long getJvmtiVirtualTime() {
    	return _getJvmtiVirtualTime(null);//Thread.currentThread());
    }
    private static native long _getElapsedTime();
    public static long getElapsedTime() {
    	return _getElapsedTime();
    }    
}
