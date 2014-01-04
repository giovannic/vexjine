package vtf_tests.forwardleap.threadtypes.model;

public class ModelShortLocalActiveThread extends ModelThread {
	
	public ModelShortLocalActiveThread(int iterations) {
		super(iterations);
	}
	
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/vtest/short_local_service.jsimg",
    		replaceMethodBody=false)  	
	protected void execute() {
    	updateTemp(1);
	}
    
    public boolean requiresResource() {
    	return true;
    }
    
    protected int resourceUsageInMsPerIteration() {
    	return 5;
    }
}

