package vtf_tests.forwardleap.threadtypes.model;

public class ModelShortRemoteActiveThread extends ModelThread {
	
	public ModelShortRemoteActiveThread(int iterations) {
		super(iterations);
	}
	
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/vtest/short_remote_service.jsimg",
    		replaceMethodBody=false)  	
	protected void execute() {
    	updateTemp(1);
	}	
    
    public boolean requiresResource() {
    	return false;
    }
    
    protected int resourceUsageInMsPerIteration() {
    	return 5;
    }
}
