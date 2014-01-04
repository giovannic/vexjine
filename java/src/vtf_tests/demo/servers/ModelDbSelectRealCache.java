package vtf_tests.demo.servers;

import vtf_tests.demo.CacheImpl;
import vtf_tests.demo.loggers.Loggers;
import vtf_tests.serverClientTest.Request;

public class ModelDbSelectRealCache implements ServerBehaviour {

	public ModelDbSelectRealCache(CacheImpl cache) {
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
    		long startTime = Loggers.selectResponseTimeLogger.onEventStart();
    		Object value = makeRequestToDBserver(request);
    		Loggers.selectRequestStartTimestamp.onEventEnd(startTime);
    		Loggers.selectResponseTimeLogger.onEventEnd(startTime);
    		
    		cache.add(key, value);
    		result = Integer.valueOf(value.toString());
    	} else {
    		result = Integer.valueOf(cachedValue.toString());
    	} 	
    	request.setResult(result);
    	
    }
    
    protected CacheImpl cache;
	
	public void cleanup() {

	}
}
