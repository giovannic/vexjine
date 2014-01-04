package vtf_tests.demo;

import vtf_tests.serverClientTest.ThinkingBehaviour;
import vtf_tests.demo.loggers.*;

public class CountingLettersModelClient implements ThinkingBehaviour {

	public CountingLettersModelClient() {
		
	}
/*
	public void think() {	
		long startTime = Loggers.selectThinkingTimeLogger.onEventStart();
		modelThink();
		Loggers.selectThinkingTimeLogger.onEventEnd(startTime);
	}
*/
	@virtualtime.ModelPerformance(
			jmtModelFilename="/homes/nb605/VTF/src/models/waiting5_remote.jsimg",
			replaceMethodBody=true)
	public void think() {	
		System.out.println("This should NOT be printed - Method body is replaced");
	}

	
}
