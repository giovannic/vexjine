package vtf_tests.demo.servers;

import org.junit.Assert;

import vtf_tests.serverClientTest.Request;

public class ModelDbSelectModelCache implements ServerBehaviour {

	public ModelDbSelectModelCache() {
		
	}
	
	@virtualtime.ModelPerformance(
			jmtModelFilename="/homes/nb605/VTF/src/models/demo_select.jsimg",
			replaceMethodBody=true)
	public void service(Request request) {
		Assert.fail("This method body should be replaced during runtime");
	}
	
	public void cleanup() {

	}

}
