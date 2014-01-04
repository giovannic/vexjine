package vtf_tests.demo;

import vtf_tests.demo.loggers.Loggers;
import vtf_tests.serverClientTest.Request;

public class CountingLettersCacheImplServer extends CountingLettersModelDescribedDB {

	public CountingLettersCacheImplServer(int requests, CacheImpl cache) {
		super(requests);
		this.cache = cache;
	}
   
	
    @virtualtime.ModelPerformance(
       		jmtModelFilename="/homes/nb605/VTF/src/models/demo_select_only_db.jsimg",
       		replaceMethodBody=false)
	protected Object makeRequestToDBserver(Request request) {
		return 53;
	}
	
    public void service(Request request) {
    	int result = 0;
    	int key = (int)request.getChar();
    	Object cachedValue = cache.retrieve(key);
    	if (cachedValue == null) {
        	
    		long startingTime = Loggers.selectResponseTimeLogger.onEventStart();
    		Object value = makeRequestToDBserver(request);
    		Loggers.selectRequestStartTimestamp.onEventEnd(startingTime);
    		Loggers.selectResponseTimeLogger.onEventEnd(startingTime);
    		
    		cache.add(key, value);
    		result = Integer.valueOf(value.toString());
    	} else {
    		result = Integer.valueOf(cachedValue.toString());
    	} 	
    	request.setResult(result);
    	
    }
    
    protected CacheImpl cache;
}
