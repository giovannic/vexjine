package vtf_tests.forwardleap.threadtypes.model;

public class ModelShortLocalInactiveThread extends ModelThread {
	
	public ModelShortLocalInactiveThread(int iterations) {
		super(iterations);
	}
	
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/vtest/short_local_service.jsimg",
    		replaceMethodBody=true)  	
	protected void execute() {
		System.exit(-1);		// should never be executed
	}	
    
    public boolean requiresResource() {
    	return true;
    }
    
    protected int resourceUsageInMsPerIteration() {
    	return 5;
    }
}
