package vtf_tests.demo.servers;

import org.junit.Assert;

import vtf_tests.serverClientTest.Request;

public class ModelDbAllSelectModelCache implements ServerBehaviour {

	public ModelDbAllSelectModelCache() {
		
	}
	
	@virtualtime.ModelPerformance(
			jmtModelFilename="/homes/nb605/VTF/src/models/demo_select_insert.jsimg",
			replaceMethodBody=true,
			customerClass=0,
			sourceNodeLabel="Source_SELECT")
	public void service(Request request) {
		Assert.fail("This method body should be replaced during runtime");
	}
	
	public void cleanup() {

	}
	
}
