package vtf_tests.demo.servers;

import org.junit.Assert;
import vtf_tests.serverClientTest.Request;

public class ModelDbAllUpdateModelCache implements ServerBehaviour {

	public ModelDbAllUpdateModelCache() {
		
	}
	
	@virtualtime.ModelPerformance(
			jmtModelFilename="/homes/nb605/VTF/src/models/demo_select_insert.jsimg",
			replaceMethodBody=true,
			customerClass=1,
			sourceNodeLabel="Source_INSERT")
	public void service(Request request) {
		Assert.fail("This method body should be replaced during runtime");
	}
	
	public void cleanup() {

	}

}