package vtf_tests.demo;

import vtf_tests.serverClientTest.ThinkingBehaviour;

public class DbInsertingModelClient implements ThinkingBehaviour {

	public DbInsertingModelClient() {
		
	}

	@virtualtime.ModelPerformance(
			jmtModelFilename="/homes/nb605/VTF/src/models/waiting10_remote.jsimg",
			replaceMethodBody=true)
	public void think() {	
		System.out.println("This should NOT be printed - Method body is replaced");
	}

	
}
