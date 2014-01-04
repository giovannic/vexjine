package vtf_tests.forwardleap.threadtypes.model;

public class ModelLongRemoteInactiveThread extends ModelThread {
	
	public ModelLongRemoteInactiveThread(int iterations) {
		super(iterations);
	}
	
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/vtest/long_remote_service.jsimg",
    		replaceMethodBody=true)  	
	protected void execute() {
		System.exit(-1);		// should never be executed
	}	
    
    
    public boolean requiresResource() {
    	return false;
    }
    
    protected int resourceUsageInMsPerIteration() {
    	return 220;
    }
}
