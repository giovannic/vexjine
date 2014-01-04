package vtf_tests.demo.servers;

import vtf_tests.demo.CacheImpl;
import vtf_tests.demo.loggers.Loggers;
import vtf_tests.serverClientTest.Request;

public class ModelDbAllUpdateRealCache implements ServerBehaviour {

	public ModelDbAllUpdateRealCache(CacheImpl cache) {
		this.cache = cache;
	}
   
    @virtualtime.ModelPerformance(
       		jmtModelFilename="/homes/nb605/VTF/src/models/demo_select_insert_only_db.jsimg",
       		replaceMethodBody=false,
       		customerClass=1)
	protected int makeInsertingRequestToDBserver(Request request) {
    	return 52;
	}
	
    public void service(Request request) {
		long startingTime = Loggers.insertResponseTimeLogger.onEventStart();
    	makeInsertingRequestToDBserver(request);
    	Loggers.insertResponseTimeLogger.onEventEnd(startingTime);
    	
    	int key = (int)request.getChar();
    	cache.invalidate(key);
    }
    
    protected CacheImpl cache;
	
	public void cleanup() {

	}
}