package vtf_tests.demo.servers;

import vtf_tests.demo.CacheImpl;
import vtf_tests.serverClientTest.Request;

public class ModelDbAllSelectRealCache extends ModelDbSelectRealCache {

	public ModelDbAllSelectRealCache(CacheImpl cache) {
		super(cache);
	}
   
    @virtualtime.ModelPerformance(
       		jmtModelFilename="/homes/nb605/VTF/src/models/demo_select_insert_only_db.jsimg",
       		replaceMethodBody=false,
       		customerClass=0,
       		sourceNodeLabel="Source_SELECT")
	protected Object makeRequestToDBserver(Request request) {
		return 53;
	}
	
}
