package vtf_tests.serverClientTest.thinking;

import vtf_tests.serverClientTest.ThinkingBehaviour;

public class ModelThinkingBehaviour implements ThinkingBehaviour {

	public ModelThinkingBehaviour() {
		
	}
	
	@virtualtime.ModelPerformance(
			jmtModelFilename="/homes/nb605/VTF/src/models/waiting400_remote.jsimg",
			replaceMethodBody=true)
	public void think() {	
		System.out.println("This should NOT be printed - Method body is replaced");
	}

	
}
