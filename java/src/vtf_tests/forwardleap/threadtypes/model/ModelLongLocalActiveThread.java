package vtf_tests.forwardleap.threadtypes.model;

public class ModelLongLocalActiveThread extends ModelThread {
	
	public ModelLongLocalActiveThread(int iterations) {
		super(iterations);
	}
	
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/vtest/long_local_service.jsimg",
    		replaceMethodBody=false)  	
	protected void execute() {
    	updateTemp(1);
	}	
    
    public boolean requiresResource() {
    	return true;
    }
    
    protected int resourceUsageInMsPerIteration() {
    	return 220;
    }
}
